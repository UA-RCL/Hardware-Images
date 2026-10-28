/* Linux transport for the same fixed mapping and event oracle as Vitis.
 * Exclusive access required. Fresh PL required after any failed/aborted run.
 */
#define _POSIX_C_SOURCE 200809L
#include "ranc_reference.h"
#include <errno.h>
#include <fcntl.h>
#include <glob.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

#define DMA_ADDR UINT64_C(0x80000000)
#define GPIO_ADDR UINT64_C(0x80010000)
#define ERRORS 0xfcU
#define DMA_ERRORS 0x770U
#define FAILED "/run/ranc-linux-needs-pl-reset"
static volatile uint32_t *dma, *gpio, *tx, *rx;
static uint64_t tx_phys, rx_phys;
static uint16_t expected[RANC_MAX_TICKS];
static unsigned case_number;

/* Standalone diagnostic failures terminate this process, unlike the SEQUOIA backend. */
static void die(const char *s) { perror(s); exit(1); }
/* Order noncached DMA memory and MMIO accesses across the CPU/device boundary. */
static void barrier(void)
{
#ifdef __aarch64__
    __asm__ volatile("dmb sy" ::: "memory");
#else
    __sync_synchronize();
#endif
}
/* Access byte-offset device registers with ordering barriers. */
static uint32_t rd(volatile uint32_t *p,unsigned off)
{ uint32_t v=p[off/4];barrier();return v; }
static void wr(volatile uint32_t *p,unsigned off,uint32_t v)
{ barrier();p[off/4]=v;barrier(); }
/* Monotonic time bounds polling and reports standalone job duration. */
static uint64_t now_us(void)
{ struct timespec t; if(clock_gettime(CLOCK_MONOTONIC,&t)) die("clock");return (uint64_t)t.tv_sec*1000000+t.tv_nsec/1000; }
/* Preserve required hardware delays if a signal interrupts nanosleep. */
static void pause_us(long us)
{ struct timespec t={us/1000000,(us%1000000)*1000};while(nanosleep(&t,&t)&&errno==EINTR){} }
/* Read sysfs physical addresses, sizes, and cache-mode attributes. */
static uint64_t number_file(const char *path)
{
    char b[128],*end;FILE *f=fopen(path,"r");
    if(!f) die(path);
    if(!fgets(b,sizeof b,f)) {fprintf(stderr,"Cannot read %s\n",path);exit(1);} fclose(f);
    errno=0;uint64_t v=strtoull(b,&end,0);
    if(errno||end==b) {fprintf(stderr,"Invalid number in %s\n",path);exit(1);}return v;
}
/* Map one shared device/buffer window for direct userspace access. */
static volatile uint32_t *map_fd(int fd,size_t size,off_t offset)
{ void *p=mmap(NULL,size,PROT_READ|PROT_WRITE,MAP_SHARED,fd,offset);if(p==MAP_FAILED)die("mmap");return p; }
/* Discover the UIO number by physical address, not by enumeration order. */
static volatile uint32_t *map_uio(uint64_t address)
{
    glob_t g;char path[256],dev[128];volatile uint32_t *result=NULL;
    if(glob("/sys/class/uio/uio*/maps/map0/addr",0,NULL,&g)) {fprintf(stderr,"No UIO devices found\n");exit(1);}
    for(size_t i=0;i<g.gl_pathc;i++) if(number_file(g.gl_pathv[i])==address) {
        unsigned index; if(sscanf(g.gl_pathv[i],"/sys/class/uio/uio%u/maps/map0/addr",&index)!=1)exit(1);
        snprintf(path,sizeof path,"/sys/class/uio/uio%u/maps/map0/offset",index);
        uint64_t offset=number_file(path);
        snprintf(path,sizeof path,"/sys/class/uio/uio%u/maps/map0/size",index);
        size_t size=(size_t)number_file(path);
        if(offset||size<4096) {fprintf(stderr,"Unexpected UIO layout\n");exit(1);}
        snprintf(dev,sizeof dev,"/dev/uio%u",index);int fd=open(dev,O_RDWR|O_SYNC);if(fd<0)die(dev);
        result=map_fd(fd,4096,0);close(fd);printf("UIO %s addr=0x%08" PRIx64 "\n",dev,address);break;
    }
    globfree(&g);
    if(!result) {fprintf(stderr,"Missing UIO at 0x%08" PRIx64 "\n",address);exit(1);}return result;
}
/* Validate HP0-reachable DDR and select noncached mappings before any DMA jobs. */
static volatile uint32_t *map_buffer(unsigned index,uint64_t *physical)
{
    char path[256],dev[64];
    snprintf(path,sizeof path,"/sys/class/u-dma-buf/udmabuf%u/phys_addr",index);
    *physical=number_file(path);
    snprintf(path,sizeof path,"/sys/class/u-dma-buf/udmabuf%u/size",index);
    if(number_file(path)<4096 || (*physical&63) || *physical>=UINT64_C(0x80000000)-4096) {
        fprintf(stderr,"Buffer must be aligned and inside HP0 low DDR (<2 GiB)\n");exit(1);
    }
    /* Driver maps O_SYNC memory noncached when sync_mode is 1. */
    snprintf(path,sizeof path,"/sys/class/u-dma-buf/udmabuf%u/sync_mode",index);
    FILE *f=fopen(path,"w");if(!f)die(path);
    int bad=fputs("1\n",f)<0; if(fclose(f))bad=1;if(bad)die("sync_mode");
    if(number_file(path)!=1) {fprintf(stderr,"Noncached mode unavailable\n");exit(1);}
    snprintf(dev,sizeof dev,"/dev/udmabuf%u",index);
    int fd=open(dev,O_RDWR|O_SYNC);if(fd<0)die(dev);
    volatile uint32_t *p=map_fd(fd,4096,0);close(fd);
    printf("BUFFER %s phys=0x%08" PRIx64 " noncached\n",dev,*physical);return p;
}
/* Dump GPIO and both DMA status registers to diagnose a failed transfer. */
static void report(void)
{ printf("REG control=%08x status=%08x mm2s=%08x s2mm=%08x\n",rd(gpio,0),rd(gpio,8),rd(dma,4),rd(dma,0x34)); }
/* A completed transfer is idle, not halted; timeout and sticky errors fail the test. */
static int wait_dma(unsigned offset)
{
    uint64_t start=now_us();
    for(;;) {
        uint32_t s=rd(dma,offset+4);
        if((rd(dma,4)|rd(dma,0x34))&DMA_ERRORS || rd(gpio,8)&ERRORS)return -1;
        if(s&1)return -2; /* Halted is not a completed transfer. */
        if(s&2)return 0;
        if(now_us()-start>2000000)return -3;
    }
}
/* Program simple DMA: offset 0 is TX, 0x30 is RX; length starts the transfer. */
static int start_dma(unsigned offset,uint64_t address,unsigned length)
{
    if(!(rd(dma,offset+4)&2) && !(rd(dma,offset+4)&1))return -1;
    wr(dma,offset+4,0x7000);wr(dma,offset,1);
    wr(dma,offset+0x18,(uint32_t)address);wr(dma,offset+0x1c,0);
    wr(dma,offset+0x28,length);return 0;
}
/* Set GPIO directions and reset simple DMA once before running a suite. */
static int init(void)
{
    wr(gpio,4,0);wr(gpio,12,UINT32_MAX);
    wr(gpio,0,2);pause_us(10);wr(gpio,0,0);pause_us(10);
    wr(dma,0,4);uint64_t start=now_us();
    while(rd(dma,0)&4)if(now_us()-start>500000)return -1;
    if((rd(dma,4)|rd(dma,0x34))&8) {fprintf(stderr,"Unexpected scatter-gather DMA\n");return -1;}
    return 0;
}
/* Print signed vectors for inspection alongside PASS/FAIL summaries. */
static void vector(const char *name,const int32_t *v,int n)
{ printf("%s=[",name);for(int i=0;i<n;i++)printf("%s%d",i?",":"",v[i]);puts("]"); }
/* Check direct VMM against the model, then verify every tagged FPGA frame and final y. */
static int run(const int32_t x[5],int cpu_only)
{
    uint32_t words[RANC_MAX_PACKETS];int32_t cpu[8],y[8]={0},peak;
    int packets=ranc_encode(x,words),ticks;
    if(packets<0) {fprintf(stderr,"Input range: [-15,15]\n");return 1;}
    ranc_cpu_vmm(x,cpu);ticks=ranc_reference(x,expected,RANC_MAX_TICKS,&peak);
    if(ticks<0) {fprintf(stderr,"Oracle failed: %d\n",ticks);return 1;}
    for(int t=0;t<ticks;t++)for(int j=0;j<8;j++)y[j]+=((expected[t]>>(2*j))&1)-((expected[t]>>(2*j+1))&1);
    if(memcmp(y,cpu,sizeof y)) {fprintf(stderr,"Oracle disagrees with direct VMM\n");return 1;}
    vector("x",x,5);vector("cpu_y",cpu,8);
    if(cpu_only) {printf("CPU PASS packets=%d ticks=%d peak=%d\n",packets,ticks,peak);return 0;}
    /* Persistent marker also catches an interrupted run; soft reset cannot
     * restore neuron RAM after an incomplete computation. */
    int marker=open(FAILED,O_WRONLY|O_CREAT|O_EXCL,0600);if(marker<0)die(FAILED);close(marker);
    memset(y,0,sizeof y);wr(gpio,0,2);pause_us(10);wr(gpio,0,0);pause_us(10);
    for(int i=0;i<packets;i++)tx[i]=words[i];
    barrier();
    /* Arm receive before ticks start, including the initial silent interval. */
    if(start_dma(0x30,rx_phys,4))goto fail;
    if(packets && (start_dma(0,tx_phys,(unsigned)packets*4)||wait_dma(0)))goto fail;
    uint64_t start=now_us();wr(gpio,0,((uint32_t)ticks<<16)|1);
    for(int t=0;t<ticks;t++) {
        if(wait_dma(0x30))goto fail;
        barrier();uint32_t word=rx[0],want=((uint32_t)(t+1)<<16)|expected[t];
        if(word!=want) {fprintf(stderr,"FAIL tick=%d actual=%08x expected=%08x\n",t+1,word,want);goto fail;}
        for(int j=0;j<8;j++)y[j]+=(int)((word>>(2*j))&1)-(int)((word>>(2*j+1))&1);
        /* TLAST ends every 4-byte frame; this is not one continuous receive DMA. */
        if(t+1<ticks && start_dma(0x30,rx_phys,4))goto fail;
    }
    wr(gpio,0,0);pause_us(2000);uint32_t state=rd(gpio,8);
    if((state&ERRORS)||(state>>16)!=(unsigned)ticks||memcmp(y,cpu,sizeof y))goto fail;
    vector("ranc_y",y,8);
    printf("RESULT case=%u mode=Linux-HW PASS packets=%d ticks=%d peak=%d hardware_us=%" PRIu64 "\n",++case_number,packets,ticks,peak,now_us()-start);
    if(unlink(FAILED))die("remove marker");
    return 0;
fail:
    wr(gpio,0,0);report();fprintf(stderr,"HW FAIL: reboot board to reload PL before another test\n");return 1;
}
/* Parse commands and acquire exclusive hardware ownership for physical test modes. */
int main(int argc,char **argv)
{
    int32_t x[5];int cpu=argc>1&&!strcmp(argv[1],"cpu");
    if(argc<2 || (strcmp(argv[1],"info")&&strcmp(argv[1],"quick")&&strcmp(argv[1],"v")&&!cpu))goto usage;
    if(!strcmp(argv[1],"v")||cpu) {
        if(argc!=7)goto usage;
        for(int i=0;i<5;i++) {char *end;errno=0;long n=strtol(argv[i+2],&end,10);if(errno||end==argv[i+2]||*end||n< -15||n>15)goto usage;x[i]=(int32_t)n;}
    } else if(argc!=2)goto usage;
    if(cpu)return run(x,1);
    if(geteuid()) {fprintf(stderr,"Run hardware commands with sudo\n");return 1;}
    int lock=open("/run/ranc-linux.lock",O_RDWR|O_CREAT,0600);if(lock<0)die("lock");
    if(flock(lock,LOCK_EX|LOCK_NB))die("another verifier is running");
    dma=map_uio(DMA_ADDR);gpio=map_uio(GPIO_ADDR);
    if(!strcmp(argv[1],"info")) {report();puts("INFO only: register reads do not verify VMM correctness");return 0;}
    if(!access(FAILED,F_OK)) {fprintf(stderr,"Earlier run failed/aborted: reboot board before retry\n");return 1;}
    tx=map_buffer(0,&tx_phys);rx=map_buffer(1,&rx_phys);
    if(tx_phys==rx_phys) {fprintf(stderr,"TX/RX buffers overlap\n");return 1;}
    if(init()) {report();return 1;}
    if(!strcmp(argv[1],"v"))return run(x,0);
    static const int32_t cases[][5]={{0,0,0,0,0},{0,2,-3,0,2},{0,0,0,0,1},{0,-1,1,0,0},{0,1,-1,0,0},{1,0,-1,2,0},{-1,0,0,0,0}};
    for(unsigned i=0;i<sizeof cases/sizeof cases[0];i++)if(run(cases[i],0))return 1;
    puts("SUITE quick PASS cases=7");return 0;
usage:
    fprintf(stderr,"Usage: %s info | quick | v x0 x1 x2 x3 x4 | cpu x0 x1 x2 x3 x4\n",argv[0]);return 2;
}
