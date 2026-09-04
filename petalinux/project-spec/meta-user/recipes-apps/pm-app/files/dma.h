/*
 * dma.h -- the instruction path into program_manager_top.
 *
 * Instructions are 64-bit VLIW words and reach the manager on its AXI4-Stream
 * slave port, not over AXI-Lite. An AXI DMA in MM2S simple mode moves them out of
 * a physically contiguous DDR buffer; this drives that DMA directly from
 * userspace over /dev/mem, the same way pm-app already drives the manager. Simple
 * mode is four registers and needs no kernel driver, and there is no interrupt to
 * wait on -- the DMA raises Idle when the transfer is done, and asserts TLAST on
 * its final beat, which is the framing the manager closes the transfer on.
 *
 * The source buffer comes from u-dma-buf (/dev/udmabuf0, whose physical address
 * is published in sysfs) when the module is loaded, and otherwise from a fixed
 * physical window mapped through /dev/mem. Either way it must be uncached and
 * contiguous: the DMA reads DDR directly and knows nothing about the MicroBlaze's
 * caches.
 */
#ifndef PM_DMA_H
#define PM_DMA_H

#include <stdint.h>
#include <stddef.h>

/* AXI DMA control registers, from the block design's address map. Keep in step
 * with the assign_bd_address line for axi_dma_0 in
 * vivado/VCU128_microblaze_bd.tcl. */
#define PM_DMA_BASE_ADDR   0x40400000UL
#define PM_DMA_MAP_SIZE    0x1000UL

/* Fallback source buffer when u-dma-buf is not available: a window at the top of
 * the 2 GB DDR aperture (0x80000000..0xFFFFFFFF) that the reserved-memory node in
 * system-user.dtsi keeps Linux out of. */
#define PM_BUF_PHYS_FALLBACK 0xFF000000UL
#define PM_BUF_SIZE          0x00100000UL   /* 1 MiB = 131072 VLIW words */

#define PM_MAX_INSTR       (PM_BUF_SIZE / 8)

struct pm_dma {
    volatile uint32_t *regs;    /* DMA control registers */
    uint64_t          *buf;     /* instruction staging buffer, virtual */
    unsigned long      buf_phys;
    size_t             buf_size;
    int                from_udmabuf;
};

/* Maps the DMA registers and the source buffer. Returns 0 on success; on failure
 * writes a reason into err and leaves *d unusable but safe to pass to
 * pm_dma_close(). A failure here is not fatal to pm-app: everything except
 * instruction streaming still works. */
int  pm_dma_open(struct pm_dma *d, char *err, size_t errlen);
void pm_dma_close(struct pm_dma *d);

/* Fill the staging buffer: n generated dummy words, or the first n words of a
 * file. Returns the number of words placed, or -1 with a reason in err. */
long pm_dma_fill_generated(struct pm_dma *d, uint8_t kernel, uint32_t n,
                           char *err, size_t errlen);
long pm_dma_fill_file(struct pm_dma *d, const char *path, uint32_t n,
                      char *err, size_t errlen);

/* Send n words from the staging buffer. The DMA asserts TLAST on the last beat,
 * which is what tells the manager the transfer is complete. Returns 0 on success,
 * -1 on a DMA error or timeout with a reason in err.
 *
 * The caller must have issued MODE_LOAD to the manager BEFORE calling this:
 * beats that arrive with no transfer open are dropped. */
int  pm_dma_send(struct pm_dma *d, uint32_t n, char *err, size_t errlen);

#endif
