/* A53 standalone MNIST verification. Inputs and reference masks are embedded;
 * all tested predictions come from the physical RANC DMA output, never fallback.
 */
#include "xaxidma.h"
#include "xaxidma_hw.h"
#include "xil_cache.h"
#include "xil_io.h"
#include "xil_printf.h"
#include "xuartps.h"
#include "xiltimer.h"
#include "sleep.h"
#include "fixture.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#define DMA_BASE 0x80000000U
#define GPIO_BASE 0x80010000U
#define UART_BASE 0xff010000U
static XAxiDma dma;
static XUartPs uart;
/* Host-owned mailbox: images, flags, request sequence (written last).
 * CPU invalidates this separate cache line to observe live JTAG writes.
 */
volatile uint32_t mnist_run_config[16] __attribute__((aligned(64)))={MNIST_IMAGES,0,0};
/* Custom 28x28 binary image, pixel i in bit i%32 of word i/32.
 * Host owns both cache lines; UART can also fill them while the server is idle. */
volatile uint32_t mnist_image_words[32] __attribute__((aligned(64)))={0};
/* ABI v3: timings retain the v2 layout; words 31..35 add hardware tag,
 * acknowledged request, resident-ready flag, job count and idle heartbeat.
 */
volatile uint32_t mnist_board_result[64] __attribute__((aligned(64)))={0x4d4e4953U};
static uint16_t hardware_step;
static unsigned session_failed;
static uint32_t tx[512] __attribute__((aligned(64)));
static uint32_t rx[16] __attribute__((aligned(64)));

static uint64_t now_ticks(void) { XTime t; XTime_GetTime(&t); return (uint64_t)t; }
static uint64_t us(uint64_t ticks) {
  return ticks/COUNTS_PER_SECOND*1000000ULL+(ticks%COUNTS_PER_SECOND)*1000000ULL/COUNTS_PER_SECOND;
}
static void result64(unsigned word,uint64_t value) {
  mnist_board_result[word]=(uint32_t)value;
  mnist_board_result[word+1]=(uint32_t)(value>>32);
}
static void publish(unsigned state,unsigned correct,unsigned classified) {
  /* Publish completion last, after all result cache lines reach DDR. */
  mnist_board_result[1]=0;
  mnist_board_result[3]=correct; mnist_board_result[4]=classified;
  mnist_board_result[6]=Xil_In32(GPIO_BASE+8);
  Xil_DCacheFlushRange((UINTPTR)mnist_board_result,sizeof(mnist_board_result));
  mnist_board_result[1]=state;
  Xil_DCacheFlushRange((UINTPTR)mnist_board_result,64);
}
static int wait_dma(int direction) {
  uint64_t start=now_ticks();
  while(XAxiDma_Busy(&dma,direction)) {
    if(now_ticks()-start > COUNTS_PER_SECOND*2ULL) return -1;
    if(Xil_In32(GPIO_BASE+8)&0xfcU) return -2;
    if((XAxiDma_ReadReg(DMA_BASE,4)|XAxiDma_ReadReg(DMA_BASE,0x34))&XAXIDMA_ERR_ALL_MASK) return -3;
  }
  return 0;
}
/* Optional UART selection for a host-submitted interactive job. */
static unsigned prompt_images(unsigned fallback) {
  for(;;) {
    unsigned value=0, digits=0, invalid=0;
    xil_printf("Images 1..%u (Enter=%u): ",MNIST_IMAGES,fallback);
    for(;;) {
      unsigned char ch=XUartPs_RecvByte(UART_BASE);
      if(ch=='\r'||ch=='\n') break;
      XUartPs_SendByte(UART_BASE,ch);
      if(ch<'0'||ch>'9'||digits++>=5) invalid=1;
      else value=value*10+(ch-'0');
    }
    xil_printf("\r\n");
    if(!digits && !invalid) return fallback;
    if(!invalid && value>=1 && value<=MNIST_IMAGES) return value;
    xil_printf("Invalid image count\r\n");
  }
}
/* One step on the already initialized fabric. Hardware tags continue across
 * jobs; only the application's image index starts over. */
static int step(unsigned count) {
  Xil_DCacheFlushRange((UINTPTR)tx,sizeof(tx));
  Xil_DCacheFlushRange((UINTPTR)rx,sizeof(rx));
  if(XAxiDma_SimpleTransfer(&dma,(UINTPTR)rx,36,XAXIDMA_DEVICE_TO_DMA)!=XST_SUCCESS) return -1;
  if(count) {
    if(XAxiDma_SimpleTransfer(&dma,(UINTPTR)tx,count*4,XAXIDMA_DMA_TO_DEVICE)!=XST_SUCCESS ||
       wait_dma(XAXIDMA_DMA_TO_DEVICE)) return -1;
  }
  Xil_Out32(GPIO_BASE,1);
  if(wait_dma(XAXIDMA_DEVICE_TO_DMA)) return -1;
  Xil_Out32(GPIO_BASE,0);
  Xil_DCacheInvalidateRange((UINTPTR)rx,sizeof(rx));
  ++hardware_step;
  mnist_board_result[31]=hardware_step;
  return rx[0]!=hardware_step || (Xil_In32(GPIO_BASE+8)&0xfcU) ? -1 : 0;
}
/* Drain the two feed-forward layers and check their steady bias response. */
static int flush_pipeline(void) {
  for(unsigned n=0;n<2;++n) {
    if(step(0)) return -1;
    if(n==1) for(unsigned w=0;w<8;++w)
      if(rx[w+1]!=mnist_expected[MNIST_ROWS-1][w]) return -1;
  }
  return 0;
}
/* Flatten 25 class voters into ten counts; ties select the lowest class. */
static unsigned classify(unsigned votes[10]) {
  unsigned prediction=0;
  memset(votes,0,10*sizeof(*votes));
  for(unsigned channel=0;channel<250;++channel)
    votes[channel%10]+=(rx[1+channel/32]>>(channel%32))&1U;
  for(unsigned c=1;c<10;++c) if(votes[c]>votes[prediction]) prediction=c;
  return prediction;
}
/* Runtime input uses the same four overlapping 16x16 windows as the original
 * MNIST packet stream. No label or expected prediction enters the FPGA. */
static int run_custom(unsigned request) {
  uint64_t start=now_ticks();
  unsigned count=0,votes[10];
  mnist_board_result[32]=request;
  mnist_board_result[36]=1;
  mnist_board_result[37]=0xffffffffU;
  mnist_board_result[5]=1;
  mnist_board_result[20]=2;
  ++mnist_board_result[34];
  publish(0,0,0);
  for(unsigned core=0;core<4;++core) {
    unsigned y0=(core/2)*12,x0=(core%2)*12;
    for(unsigned axon=0;axon<256;++axon) {
      unsigned pixel=(y0+axon/16)*28+x0+axon%16;
      if((mnist_image_words[pixel/32]>>(pixel%32))&1U) {
        /* The existing 512-entry staging buffer reserves one slot. Reject
         * dense images before DMA rather than silently dropping spikes. */
        if(count==511) {
          xil_printf("ERROR image exceeds 511 input packets\r\n");
          mnist_board_result[38]=512; publish(4,0,0); return 1;
        }
        tx[count++]=(core<<21)|(axon<<4);
      }
    }
  }
  mnist_board_result[38]=count;
  if(flush_pipeline()) goto fail;
  uint64_t inference_start=now_ticks();
  mnist_board_result[2]=1;
  if(step(count)) goto fail;
  for(unsigned w=0;w<8;++w) if(rx[w+1]!=mnist_expected[MNIST_ROWS-1][w]) goto fail;
  mnist_board_result[2]=2;
  if(step(0)) goto fail;
  unsigned prediction=classify(votes);
  uint64_t end=now_ticks();
  mnist_board_result[37]=prediction;
  for(unsigned c=0;c<10;++c) mnist_board_result[39+c]=votes[c];
  for(unsigned w=0;w<8;++w) mnist_board_result[49+w]=rx[w+1];
  result64(57,us(end-inference_start)); result64(59,us(end-start));
  xil_printf("MNIST_IMAGE prediction=%u packets=%u inference_us=%u job_us=%u votes=[",
    prediction,count,(unsigned)us(end-inference_start),(unsigned)us(end-start));
  for(unsigned c=0;c<10;++c) xil_printf(c?",%u":"%u",votes[c]);
  xil_printf("]\r\n");
  /* Success means a physical inference completed; an arbitrary image has no
   * built-in ground truth. The host can check all spikes against simulation. */
  publish(1,0,1); return 0;
fail:
  session_failed=1; Xil_Out32(GPIO_BASE,0); publish(2,0,0);
  xil_printf("MNIST_IMAGE FAIL status=%08x; reload required\r\n",Xil_In32(GPIO_BASE+8));
  return 1;
}

static int run_images(unsigned images,unsigned flags,unsigned request) {
  uint64_t app_start=now_ticks(),test_start=0,test_end=0;
  uint64_t submitted[2]={0},latency_sum=0,min_latency=UINT64_MAX,max_latency=0,first_latency=0;
  unsigned offset=0,correct=0,classified=0,expected_correct=0;
  mnist_board_result[7]=3;
  mnist_board_result[32]=request;
  mnist_board_result[36]=0;
  ++mnist_board_result[34];
  result64(22,COUNTS_PER_SECOND);
  mnist_board_result[30]=flags;
  publish(0,0,0);
  if(images<1||images>MNIST_IMAGES) goto fail;
  if(flags&1) images=prompt_images(images);
  mnist_board_result[5]=images;
  /* One initial pipeline step plus one step per classified image. Stop after
   * the last requested prediction; later full-dataset rows need other inputs.
   */
  unsigned steps=images+1;
  mnist_board_result[20]=steps;
#ifdef MNIST_HAS_LABELS
  for(unsigned i=0;i<images;++i) expected_correct+=mnist_predictions[i]==mnist_labels[i];
#endif
  mnist_board_result[21]=expected_correct;
  xil_printf("RANC MNIST images=%u steps=%u verbose=%u\r\n",images,steps,!!(flags&2));
  /* This specific two-layer feed-forward mapping has absolute-reset neurons.
   * Two blank steps drain old image events and establish steady bias activity.
   * A controller reset alone would not clear neuron RAM. */
  if(flush_pipeline()) goto fail;
  publish(0,0,0);
  test_start=now_ticks();
  for(unsigned t=1;t<=steps;++t) {
    mnist_board_result[2]=t;
    unsigned count=t<=images?mnist_counts[t-1]:0;
    /* Two-entry timestamp ring follows the one-step input/output pipeline. */
    if(t<=images) submitted[(t-1)%2]=now_ticks();
    for(unsigned p=0;p<count;++p) tx[p]=mnist_packets[offset++];
    if(step(count)) goto fail;
    const uint32_t *expected=t==1?mnist_expected[MNIST_ROWS-1]:mnist_expected[t];
    for(unsigned w=0;w<8;++w) if(rx[w+1]!=expected[w]) {
      xil_printf("FAIL tick=%u word=%u actual=%08x expected=%08x\r\n",t,w,rx[w+1],expected[w]);
      goto fail;
    }
    if(t>=2) {
      unsigned votes[10],prediction=classify(votes);
#ifdef MNIST_HAS_LABELS
      unsigned image=t-2;
      if(prediction!=mnist_predictions[image]) goto fail;
      correct+=prediction==mnist_labels[image];
#endif
      ++classified;
      uint64_t latency=now_ticks()-submitted[(t-2)%2];
      latency_sum+=latency;
      if(latency<min_latency) min_latency=latency;
      if(latency>max_latency) max_latency=latency;
      if(classified==1) first_latency=latency;
    }
    if(t%1000==0) publish(0,correct,classified);
    if((flags&2) && (t%1000==0 || t==steps))
      xil_printf("PASS tick=%u classified=%u\r\n",t,classified);
  }
  test_end=now_ticks();
  if(classified!=images || correct!=expected_correct) goto fail;
  /* Timers end before final UART reporting. Test includes DMA/cache/control,
   * exact-mask and prediction checks; application also includes setup/prompt.
   */
  result64(8,us(test_end-app_start)); result64(10,us(test_end-test_start));
  result64(12,us(latency_sum)/images); result64(14,us(test_end-test_start)/images);
  result64(16,us(test_start-app_start)); result64(18,us(latency_sum));
  result64(24,us(first_latency)); result64(26,us(min_latency)); result64(28,us(max_latency));
  xil_printf("MNIST_ACCURACY correct=%u total=%u expected_correct=%u\r\n",correct,classified,expected_correct);
  xil_printf("MNIST_TIMING application_us=%u test_us=%u setup_us=%u avg_image_latency_us=%u amortized_us_per_image=%u\r\n",
    (unsigned)us(test_end-app_start),(unsigned)us(test_end-test_start),(unsigned)us(test_start-app_start),
    (unsigned)(us(latency_sum)/images),(unsigned)(us(test_end-test_start)/images));
  xil_printf("MNIST_STREAM PASS images=%u exact simulator spikes\r\n",images);
  publish(1,correct,classified);
  return 0;
fail:
  session_failed=1;
  Xil_Out32(GPIO_BASE,0);
  publish(2,correct,classified);
  xil_printf("MNIST_STREAM FAIL tick=%u status=%08x; reload PL\r\n",mnist_board_result[2],Xil_In32(GPIO_BASE+8));
  return 1;
}

/* Keep PS, PL and DMA resident. UART and JTAG are alternative command sources;
 * the host owns the config cache line and writes request sequence last. */
int main(void) {
  XUartPs_Config *uc=XUartPs_LookupConfig(UART_BASE);
  XAxiDma_Config *dc=XAxiDma_LookupConfig(DMA_BASE);
  mnist_board_result[7]=3;
  if(!uc || XUartPs_CfgInitialize(&uart,uc,UART_BASE)!=XST_SUCCESS ||
     XUartPs_SetBaudRate(&uart,115200)!=XST_SUCCESS || !dc ||
     XAxiDma_CfgInitialize(&dma,dc)!=XST_SUCCESS || XAxiDma_HasSg(&dma)) {
    publish(2,0,0); return 1;
  }
  XAxiDma_IntrDisable(&dma,XAXIDMA_IRQ_ALL_MASK,XAXIDMA_DMA_TO_DEVICE);
  XAxiDma_IntrDisable(&dma,XAXIDMA_IRQ_ALL_MASK,XAXIDMA_DEVICE_TO_DMA);
  Xil_Out32(GPIO_BASE+4,0); Xil_Out32(GPIO_BASE+12,0xffffffffU);
  Xil_Out32(GPIO_BASE,2); usleep(10); Xil_Out32(GPIO_BASE,0); usleep(10);
  hardware_step=(uint16_t)(Xil_In32(GPIO_BASE+8)>>16);
  /* BSS startup dirties this host-owned buffer. A53 BSP invalidation uses
   * clean+invalidate, so flush zeros BEFORE publishing readiness; otherwise
   * the first host upload can be overwritten by stale startup cache lines. */
  Xil_DCacheFlushRange((UINTPTR)mnist_image_words,sizeof(mnist_image_words));
  mnist_board_result[33]=1;
  mnist_board_result[61]=0x494d4731U; /* IMG1: runtime bitmap capability. */
  publish(3,0,0);
  xil_printf("MNIST READY: run N (1..%u), image HEX196, info, help\r\n> ",MNIST_IMAGES);
  unsigned last_request=0,used=0;
  char line[256];
  uint64_t heartbeat=now_ticks();
  for(;;) {
    Xil_DCacheInvalidateRange((UINTPTR)mnist_run_config,sizeof(mnist_run_config));
    unsigned request=mnist_run_config[2];
    if(request!=last_request) {
      unsigned images=mnist_run_config[0],flags=mnist_run_config[1];
      last_request=request;
      if(session_failed) xil_printf("ERROR reload required\r\n");
      else if(flags&4) {
        Xil_DCacheInvalidateRange((UINTPTR)mnist_image_words,sizeof(mnist_image_words));
        run_custom(request);
      } else run_images(images,flags,request);
      xil_printf("> ");
    }
    if(XUartPs_IsReceiveData(UART_BASE)) {
      char ch=XUartPs_RecvByte(UART_BASE);
      if(ch=='\r'||ch=='\n') {
        line[used]=0;
        if(used) {
          xil_printf("\r\n");
          if(!strncmp(line,"run ",4)) {
            char *end; unsigned long n=strtoul(line+4,&end,10);
            if(*end || n<1 || n>MNIST_IMAGES) xil_printf("Invalid count\r\n");
            else if(session_failed) xil_printf("ERROR reload required\r\n");
            else run_images((unsigned)n,0,0);
          } else if(!strncmp(line,"image ",6)) {
            unsigned valid=strlen(line+6)==196;
            memset((void *)mnist_image_words,0,sizeof(mnist_image_words));
            for(unsigned i=0;valid && i<196;++i) {
              char ch=line[6+i];
              unsigned digit=ch>='0'&&ch<='9'?ch-'0':ch>='a'&&ch<='f'?ch-'a'+10:ch>='A'&&ch<='F'?ch-'A'+10:16;
              if(digit==16) valid=0;
              else mnist_image_words[i/8]|=digit<<((i%8/2)*8+(i%2?0:4));
            }
            /* Return clean cache ownership even after a malformed UART line,
             * so the next JTAG upload cannot be overwritten by parser writes. */
            Xil_DCacheFlushRange((UINTPTR)mnist_image_words,sizeof(mnist_image_words));
            if(!valid) xil_printf("Invalid image: require 196 hex digits\r\n");
            else if(session_failed) xil_printf("ERROR reload required\r\n");
            else {
              run_custom(0);
            }
          } else if(!strcmp(line,"info"))
            xil_printf("resident=1 failed=%u runs=%u hardware_step=%u\r\n",session_failed,mnist_board_result[34],hardware_step);
          else xil_printf("Commands: run N, image HEX196, info, help\r\n");
          xil_printf("> "); used=0;
        }
      } else if(ch==8 || ch==127) { if(used) --used; }
      else if(used<sizeof(line)-1) {line[used++]=ch; XUartPs_SendByte(UART_BASE,ch);}
    }
    if(now_ticks()-heartbeat>COUNTS_PER_SECOND/50) {
      heartbeat=now_ticks(); ++mnist_board_result[35];
      Xil_DCacheFlushRange((UINTPTR)&mnist_board_result[32],64);
    }
    if(!XUartPs_IsReceiveData(UART_BASE)) usleep(100);
  }
}
