#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <sys/mman.h>

#include "dma.h"
#include "layout.h"

/* AXI DMA MM2S registers, simple (non-scatter-gather) mode. */
#define MM2S_DMACR      0x00
#define MM2S_DMASR      0x04
#define MM2S_SA         0x18
#define MM2S_SA_MSB     0x1c   /* reserved unless the DMA is built 64-bit */
#define MM2S_LENGTH     0x28   /* writing this starts the transfer */

#define DMACR_RS        (1u << 0)
#define DMACR_RESET     (1u << 2)

#define DMASR_HALTED    (1u << 0)
#define DMASR_IDLE      (1u << 1)
#define DMASR_DMAINTERR (1u << 4)
#define DMASR_DMASLVERR (1u << 5)
#define DMASR_DMADECERR (1u << 6)
#define DMASR_ERRORS    (DMASR_DMAINTERR | DMASR_DMASLVERR | DMASR_DMADECERR)

#define UDMABUF_DEV     "/dev/udmabuf0"
#define UDMABUF_PHYS    "/sys/class/u-dma-buf/udmabuf0/phys_addr"

#define ERR(...) do { snprintf(err, errlen, __VA_ARGS__); return -1; } while (0)

static void wr(struct pm_dma *d, uint32_t off, uint32_t v) { d->regs[off / 4] = v; }
static uint32_t rd(struct pm_dma *d, uint32_t off)         { return d->regs[off / 4]; }

static double elapsed(const struct timespec *t0)
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (now.tv_sec - t0->tv_sec) + (now.tv_nsec - t0->tv_nsec) / 1e9;
}

/* u-dma-buf gives a contiguous, uncached buffer and tells us its physical
 * address, which is exactly what the DMA needs. Its absence is not an error --
 * we fall back to a fixed window through /dev/mem. */
static int open_udmabuf(struct pm_dma *d)
{
    FILE *f;
    unsigned long phys;
    int fd;
    void *p;

    f = fopen(UDMABUF_PHYS, "r");
    if (!f)
        return -1;
    if (fscanf(f, "%lx", &phys) != 1) { fclose(f); return -1; }
    fclose(f);

    fd = open(UDMABUF_DEV, O_RDWR | O_SYNC);
    if (fd < 0)
        return -1;

    p = mmap(NULL, PM_BUF_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
    if (p == MAP_FAILED)
        return -1;

    d->buf          = (uint64_t *)p;
    d->buf_phys     = phys;
    d->buf_size     = PM_BUF_SIZE;
    d->from_udmabuf = 1;
    return 0;
}

int pm_dma_open(struct pm_dma *d, char *err, size_t errlen)
{
    int fd;
    void *p;
    struct timespec t0;

    memset(d, 0, sizeof(*d));

    fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd < 0)
        ERR("open(/dev/mem): %s", strerror(errno));

    p = mmap(NULL, PM_DMA_MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED,
             fd, (off_t)PM_DMA_BASE_ADDR);
    if (p == MAP_FAILED) {
        close(fd);
        ERR("mmap(DMA 0x%lx): %s", PM_DMA_BASE_ADDR, strerror(errno));
    }
    d->regs = (volatile uint32_t *)p;

    if (open_udmabuf(d) != 0) {
        /* O_SYNC keeps this mapping uncached, so what we write is what the DMA
         * reads -- there is no cache to flush from userspace. */
        p = mmap(NULL, PM_BUF_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED,
                 fd, (off_t)PM_BUF_PHYS_FALLBACK);
        if (p == MAP_FAILED) {
            close(fd);
            ERR("mmap(buffer 0x%lx): %s -- load u-dma-buf, or check the "
                "reserved-memory node in system-user.dtsi",
                PM_BUF_PHYS_FALLBACK, strerror(errno));
        }
        d->buf      = (uint64_t *)p;
        d->buf_phys = PM_BUF_PHYS_FALLBACK;
        d->buf_size = PM_BUF_SIZE;
    }
    close(fd);

    /* Reset the engine so a transfer left in flight by a previous run cannot
     * deliver stray beats into the next MODE_LOAD. */
    wr(d, MM2S_DMACR, DMACR_RESET);
    clock_gettime(CLOCK_MONOTONIC, &t0);
    while (rd(d, MM2S_DMACR) & DMACR_RESET) {
        if (elapsed(&t0) > 1.0)
            ERR("AXI DMA at 0x%lx did not come out of reset -- check the "
                "bitstream and the address map", PM_DMA_BASE_ADDR);
    }
    wr(d, MM2S_DMACR, DMACR_RS);
    clock_gettime(CLOCK_MONOTONIC, &t0);
    while (rd(d, MM2S_DMASR) & DMASR_HALTED) {
        if (elapsed(&t0) > 1.0)
            ERR("AXI DMA stayed halted after Run/Stop was set (DMASR 0x%08x)",
                rd(d, MM2S_DMASR));
    }

    printf("AXI DMA at 0x%lx, %zu KiB buffer at 0x%lx (%s).\n",
           PM_DMA_BASE_ADDR, d->buf_size / 1024, d->buf_phys,
           d->from_udmabuf ? "u-dma-buf" : "/dev/mem");
    return 0;
}

void pm_dma_close(struct pm_dma *d)
{
    if (d->regs) munmap((void *)d->regs, PM_DMA_MAP_SIZE);
    if (d->buf)  munmap((void *)d->buf, d->buf_size);
    memset(d, 0, sizeof(*d));
}

long pm_dma_fill_generated(struct pm_dma *d, uint8_t kernel, uint32_t n,
                           char *err, size_t errlen)
{
    if (!d->buf)
        ERR("no instruction buffer: the DMA path is not available");
    if (n == 0 || n > d->buf_size / 8)
        ERR("%u instructions is out of range (1..%zu)", n, d->buf_size / 8);

    layout_gen_words(d->buf, n, kernel);
    return (long)n;
}

long pm_dma_fill_file(struct pm_dma *d, const char *path, uint32_t n,
                      char *err, size_t errlen)
{
    FILE *f;
    size_t got;

    if (!d->buf)
        ERR("no instruction buffer: the DMA path is not available");
    if (n == 0 || n > d->buf_size / 8)
        ERR("%u instructions is out of range (1..%zu)", n, d->buf_size / 8);

    f = fopen(path, "rb");
    if (!f)
        ERR("%s: %s", path, strerror(errno));

    got = fread(d->buf, 8, n, f);
    fclose(f);
    if (got != n)
        ERR("%s holds %zu instructions, not the %u declared", path, got, n);
    return (long)n;
}

int pm_dma_send(struct pm_dma *d, uint32_t n, char *err, size_t errlen)
{
    struct timespec t0;
    uint32_t sr;

    if (!d->regs || !d->buf)
        ERR("the DMA path is not available");
    if (n == 0 || n > d->buf_size / 8)
        ERR("%u instructions is out of range (1..%zu)", n, d->buf_size / 8);

    sr = rd(d, MM2S_DMASR);
    if (sr & DMASR_ERRORS) {
        /* A previous transfer faulted; the engine stays down until reset. */
        wr(d, MM2S_DMACR, DMACR_RESET);
        while (rd(d, MM2S_DMACR) & DMACR_RESET) { }
        wr(d, MM2S_DMACR, DMACR_RS);
    }

    wr(d, MM2S_SA, (uint32_t)(d->buf_phys & 0xffffffffUL));
    /* The DMA is built with a 32-bit address width, so SA_MSB is a reserved
     * register there. Only touch it if the buffer really is above 4 GiB, which
     * on this board it never is -- DDR ends at 0xFFFFFFFF. */
    if ((uint64_t)d->buf_phys >> 32)
        wr(d, MM2S_SA_MSB, (uint32_t)((uint64_t)d->buf_phys >> 32));
    /* Length is in bytes and starts the transfer, so it goes last. */
    wr(d, MM2S_LENGTH, n * 8u);

    clock_gettime(CLOCK_MONOTONIC, &t0);
    for (;;) {
        sr = rd(d, MM2S_DMASR);
        if (sr & DMASR_ERRORS)
            ERR("DMA error, DMASR 0x%08x (decode/slave error usually means the "
                "buffer at 0x%lx is not in the DMA's address map)",
                sr, d->buf_phys);
        if (sr & DMASR_IDLE)
            return 0;
        if (elapsed(&t0) > 5.0)
            ERR("DMA did not finish in 5 s, DMASR 0x%08x -- if it never leaves "
                "Idle low, the stream is not being accepted (was MODE_LOAD "
                "issued first?)", sr);
    }
}
