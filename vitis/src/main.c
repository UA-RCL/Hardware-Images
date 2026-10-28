/* RANC fixed-matrix / runtime-vector bare-metal verifier, A53 standalone.
 * UART is control/reporting only. Input/output spikes use AXI DMA through DDR.
 */
#include "ranc_reference.h"
#include "xparameters.h"
#include "xaxidma.h"
#include "xaxidma_hw.h"
#include "xil_cache.h"
#include "xil_io.h"
#include "xil_printf.h"
#include "xuartps.h"
#ifdef SDT
#include "xiltimer.h"
#else
#include "xtime_l.h"
#endif
#include "sleep.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define DMA_BASE 0x80000000U
#define GPIO_BASE 0x80010000U
#define UART_BASE 0xff010000U /* AUP preset: UART1, MIO 32..33. */
#define GPIO_DATA 0x00U
#define GPIO_TRI 0x04U
#define GPIO_STATUS 0x08U
#define GPIO_TRI2 0x0cU
#define ERROR_MASK 0xfcU
#define FRAME_TIMEOUT_US 500000U
#if defined(XPAR_XAXIDMA_0_BASEADDR) && XPAR_XAXIDMA_0_BASEADDR != DMA_BASE
#error "Build this application against the matching RANC DMA hardware platform"
#endif
#if defined(XPAR_XUARTPS_0_BASEADDR) && XPAR_XUARTPS_0_BASEADDR != UART_BASE
#error "This application requires the AUP UART1 platform"
#endif

static XAxiDma dma;
static XUartPs uart;
static uint32_t tx[RANC_MAX_PACKETS] __attribute__((aligned(64)));
/* A whole, dedicated cache line prevents invalidation of unrelated state. */
static uint32_t rx[16] __attribute__((aligned(64)));
static uint16_t expected[RANC_MAX_TICKS];
static int hardware_failed, hardware_ready;
static unsigned case_number;
static int initialize_hardware(void);

/* Bare-metal counter timing is for this standalone diagnostic application. */
static uint64_t now(void) { XTime t; XTime_GetTime(&t); return (uint64_t)t; }
static unsigned elapsed_us(uint64_t start)
{ return (unsigned)((now()-start)*1000000ULL/COUNTS_PER_SECOND); }
/* GPIO channel 1 drives run/reset/tick limit; channel 2 exposes status. */
static void control(uint32_t value) { Xil_Out32(GPIO_BASE+GPIO_DATA,value); }
static uint32_t status(void) { return Xil_In32(GPIO_BASE+GPIO_STATUS); }
static uint32_t dma_status(int receive)
{ return XAxiDma_ReadReg(DMA_BASE,(receive ? XAXIDMA_RX_OFFSET : XAXIDMA_TX_OFFSET)+XAXIDMA_SR_OFFSET); }

/* Bound busy polling and reject DMA or RANC sticky errors before accepting a frame. */
static int wait_dma(int direction)
{
    uint64_t start=now();
    for (;;) {
        if ((dma_status(0)|dma_status(1)) & XAXIDMA_ERR_ALL_MASK) return -1;
        if (status() & ERROR_MASK) return -2;
        if (!XAxiDma_Busy(&dma,direction)) return 0;
        if (elapsed_us(start)>FRAME_TIMEOUT_US) return -3;
    }
}

/* UART reporting keeps accelerator payloads on the DMA path. */
static void print_vector(const char *label,const int32_t *v,int length)
{
    int i;
    xil_printf("%s=[",label);
    for(i=0;i<length;++i) xil_printf("%s%d",i ? "," : "",(int)v[i]);
    xil_printf("]\r\n");
}

/* Cross-check direct VMM, reference spike stream, and every received hardware frame. */
static int run_case(const int32_t x[5],int cpu_only)
{
    int32_t cpu[8],decoded[8]={0},peak=0;
    int packets,ticks,t,j,rc=0;
    unsigned cpu_us,reference_us,hardware_us=0;
    uint64_t start;
    uint32_t errors=0;
    ++case_number;
    packets=ranc_encode(x,tx);
    if(packets<0) {xil_printf("ERROR: five integers in [-15,15] required\r\n");return -1;}
    start=now();ranc_cpu_vmm(x,cpu);cpu_us=elapsed_us(start);
    start=now();ticks=ranc_reference(x,expected,RANC_MAX_TICKS,&peak);reference_us=elapsed_us(start);
    if(ticks<0) {xil_printf("FAIL: CPU event oracle code=%d\r\n",ticks);return -1;}
    for(t=0;t<ticks;++t) for(j=0;j<8;++j)
        decoded[j]+=((expected[t]>>(2*j))&1)-((expected[t]>>(2*j+1))&1);
    for(j=0;j<8;++j) if(decoded[j]!=cpu[j]) {
        xil_printf("FAIL: CPU spike oracle disagrees with direct multiplication\r\n");return -1;
    }
    print_vector("x",x,5);print_vector("cpu_y",cpu,8);
    if(!cpu_only) {
        if(!hardware_failed && !hardware_ready) {
            hardware_ready=(initialize_hardware()==0);
            if(!hardware_ready) hardware_failed=1;
        }
        if(!hardware_ready || hardware_failed) {
            xil_printf("ERROR: hardware unavailable/previous failure; reload matching PL and restart app\r\n");return -1;
        }
        memset(decoded,0,sizeof decoded);
        /* Previous vector must have fully drained. Soft reset resets controllers,
         * tag/capture FIFO and tick counter, but DOES NOT initialize neuron RAM. */
        control(2);usleep(10);control(0);usleep(10);
        if(status() & ERROR_MASK) {rc=-4;goto fail;}
        /* Publish TX and discard stale RX cache lines before DMA accesses DDR. */
        Xil_DCacheFlushRange((UINTPTR)tx,sizeof tx);
        Xil_DCacheInvalidateRange((UINTPTR)rx,sizeof rx);
        rc=XAxiDma_SimpleTransfer(&dma,(UINTPTR)rx,4,XAXIDMA_DEVICE_TO_DMA);
        if(rc) goto fail;
        if(packets) {
            rc=XAxiDma_SimpleTransfer(&dma,(UINTPTR)tx,(unsigned)packets*4,XAXIDMA_DMA_TO_DEVICE);
            if(rc || (rc=wait_dma(XAXIDMA_DMA_TO_DEVICE))) goto fail;
        }
        start=now();
        control(((uint32_t)ticks<<16)|1U);
        for(t=0;t<ticks;++t) {
            uint32_t word;
            rc=wait_dma(XAXIDMA_DEVICE_TO_DMA);if(rc) goto fail;
            Xil_DCacheInvalidateRange((UINTPTR)rx,sizeof rx);
            word=rx[0];
            /* Check both frame order and the exact 16-bit reference spike mask. */
            if((word>>16)!=(unsigned)t+1 || (word&0xffffU)!=expected[t]) {
                xil_printf("FAIL tick=%d actual=%08x expected=%08x\r\n",t+1,(unsigned)word,
                           ((unsigned)(t+1)<<16)|expected[t]);rc=-5;goto fail;
            }
            for(j=0;j<8;++j) decoded[j]+=(int)((word>>(2*j))&1U)-(int)((word>>(2*j+1))&1U);
            /* Each frame has TLAST, so simple-mode receive must be rearmed. */
            if(t+1<ticks) {
                Xil_DCacheInvalidateRange((UINTPTR)rx,sizeof rx);
                rc=XAxiDma_SimpleTransfer(&dma,(UINTPTR)rx,4,XAXIDMA_DEVICE_TO_DMA);
                if(rc) goto fail;
            }
        }
        control(0);
        /* Let the final, already-quiescent computation tick finish before reset. */
        usleep(2000);hardware_us=elapsed_us(start);
        errors=status();
        if((errors & ERROR_MASK) || (errors>>16)!=(unsigned)ticks) {rc=-6;goto fail;}
        for(j=0;j<8;++j) if(decoded[j]!=cpu[j]) {rc=-7;goto fail;}
        print_vector("ranc_y",decoded,8);
    }
    xil_printf("RESULT case=%u mode=%s PASS packets=%d ticks=%d peak=%d cpu_us=%u reference_us=%u hardware_us=%u\r\n",
               case_number,cpu_only ? "CPU" : "HW",packets,ticks,(int)peak,cpu_us,reference_us,hardware_us);
    return 0;
fail:
    errors=status();control(0);hardware_failed=1;
    xil_printf("RESULT case=%u mode=HW FAIL code=%d status=%08x mm2s=%08x s2mm=%08x\r\n",
               case_number,rc,(unsigned)errors,(unsigned)dma_status(0),(unsigned)dma_status(1));
    return -1;
}

/* Match the AUP PROG/UART bridge: PS UART1, 115200 baud, default 8N1. */
static int initialize_uart(void)
{
    XUartPs_Config *ucfg;
#ifdef SDT
    ucfg=XUartPs_LookupConfig(UART_BASE);
#else
    ucfg=XUartPs_LookupConfig(XPAR_XUARTPS_0_DEVICE_ID);
#endif
    if(!ucfg || XUartPs_CfgInitialize(&uart,ucfg,ucfg->BaseAddress)!=XST_SUCCESS) return -1;
    if(XUartPs_SetBaudRate(&uart,115200)!=XST_SUCCESS) return -1;
    return 0;
}

/* Set GPIO directions and initialize simple polling DMA once for all test vectors. */
static int initialize_hardware(void)
{
    XAxiDma_Config *cfg;
    uint64_t start;
    Xil_Out32(GPIO_BASE+GPIO_TRI,0);Xil_Out32(GPIO_BASE+GPIO_TRI2,0xffffffffU);
    control(2);usleep(10);control(0);
#ifdef SDT
    cfg=XAxiDma_LookupConfig(DMA_BASE);
#else
    cfg=XAxiDma_LookupConfigBaseAddr(DMA_BASE);
#endif
    if(!cfg || XAxiDma_CfgInitialize(&dma,cfg)!=XST_SUCCESS || XAxiDma_HasSg(&dma)) return -1;
    XAxiDma_Reset(&dma);start=now();
    while(!XAxiDma_ResetIsDone(&dma)) if(elapsed_us(start)>FRAME_TIMEOUT_US) return -1;
    XAxiDma_IntrDisable(&dma,XAXIDMA_IRQ_ALL_MASK,XAXIDMA_DMA_TO_DEVICE);
    XAxiDma_IntrDisable(&dma,XAXIDMA_IRQ_ALL_MASK,XAXIDMA_DEVICE_TO_DMA);
    return 0;
}

/* Describe the UART commands available without rebuilding the application. */
static void help(void)
{
    xil_printf("Commands:\r\n"
      "  cpu x0 x1 x2 x3 x4  : CPU multiplication + CPU spike oracle only\r\n"
      "  v x0 x1 x2 x3 x4    : DMA RANC versus both CPU references\r\n"
      "  quick               : six different vectors, same fixed matrix\r\n"
      "  stress count seed   : deterministic random vectors, full [-15,15]\r\n"
      "  info                : addresses, status and protocol\r\n"
      "  help                : show commands\r\n");
}

/* Reject malformed or overflowing UART integers before input encoding. */
static int parse_int(const char *s,int32_t *v)
{
    char *end;long value;
    if(!s) return -1;
    errno=0;value=strtol(s,&end,10);
    if(errno || !*s || *end || value<INT32_MIN || value>INT32_MAX) return -1;
    *v=(int32_t)value;return 0;
}

/* Dispatch user vectors and finite test suites; a failure latches hardware unavailable. */
static void command(char *line)
{
    char *word[8],*p;int n=0,i;
    int32_t x[5],count,seed;
    for(p=strtok(line," \t");p && n<8;p=strtok(NULL," \t")) word[n++]=p;
    if(!n) return;
    if(!strcmp(word[0],"help") && n==1) {help();return;}
    if(!strcmp(word[0],"info") && n==1) {
        xil_printf("A53 standalone UART1=115200,8N1 DMA=%08x GPIO=%08x initialized=%d failed=%d\r\n",
                   DMA_BASE,GPIO_BASE,hardware_ready,hardware_failed);
        if(hardware_ready) xil_printf("status=%08x\r\n",(unsigned)status());
        xil_printf("RX={tick16,mask16}; TLAST per word; matrix fixed; potential=16; buffer=256\r\n");return;
    }
    if((!strcmp(word[0],"v") || !strcmp(word[0],"cpu")) && n==6) {
        for(i=0;i<5;++i) if(parse_int(word[i+1],&x[i])) goto invalid;
        run_case(x,!strcmp(word[0],"cpu"));return;
    }
    if(!strcmp(word[0],"quick") && n==1) {
        static const int32_t cases[][5]={{0,0,0,0,0},{0,2,-3,0,2},{0,-1,1,0,0},
                                        {0,1,-1,0,0},{1,0,-1,2,0},{-1,0,0,0,0}};
        for(i=0;i<6;++i) if(run_case(cases[i],0)) {xil_printf("SUITE quick FAIL\r\n");return;}
        xil_printf("SUITE quick PASS cases=6\r\n");return;
    }
    if(!strcmp(word[0],"stress") && n==3) {
        uint32_t rng;
        if(parse_int(word[1],&count) || parse_int(word[2],&seed) || count<1 || count>100) goto invalid;
        rng=(uint32_t)seed;if(!rng) rng=1;
        for(i=0;i<count;++i) {
            int j;
            for(j=0;j<5;++j) {rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;x[j]=(int32_t)(rng%31)-15;}
            if(run_case(x,0)) {xil_printf("SUITE stress FAIL\r\n");return;}
        }
        xil_printf("SUITE stress PASS cases=%d seed=%d\r\n",(int)count,(int)seed);return;
    }
invalid:
    xil_printf("ERROR: invalid command; type help\r\n");
}

/* Initialize the console and service editable command lines until reset. */
int main(void)
{
    char line[160];unsigned used=0;int too_long=0;
    if(initialize_uart()) return 1;
    xil_printf("\r\nRANC UART verifier 2026-10-25 | fresh matching PL required\r\n");
    xil_printf("CPU commands ready; PL/DMA initialized on first HW command\r\n");help();
    xil_printf("READY\r\n> ");
    for(;;) {
        unsigned char ch=(unsigned char)inbyte();
        if(ch=='\r' || ch=='\n') {
            if(!used && !too_long) continue; /* CRLF / blank input. */
            xil_printf("\r\n");line[used]=0;
            if(too_long) xil_printf("ERROR: command too long\r\n");else command(line);
            used=0;too_long=0;xil_printf("> ");
        } else if(ch==8 || ch==127) {
            if(used) {--used;xil_printf("\b \b");}
        } else if(ch>=32 && ch<127) {
            if(used+1<sizeof line) {line[used++]=(char)ch;outbyte((char)ch);}else too_long=1;
        }
    }
}
