#ifndef RANC_REFERENCE_H
#define RANC_REFERENCE_H
#include <stdint.h>
#define RANC_MAX_TICKS 40000
#define RANC_MAX_PACKETS 150
/* Setup/test oracle: writes one 16-bit channel mask per logical tick, including
 * initial silence and eight quiet drain ticks. peak is maximum |potential|.
 * Returns frame count or negative on invalid input, overflow, or failure to drain.
 * Uses shared scratch state: callers must serialize this reference helper. */
int ranc_reference(const int32_t x[5], uint16_t *masks, int capacity, int32_t *peak);
/* Fill 32-bit input words; returns 2*sum(|x|), or negative for x outside [-15,15]. */
int ranc_encode(const int32_t x[5], uint32_t words[RANC_MAX_PACKETS]);
/* Direct y[j]=sum_i x[i]*A[i][j]; independent mathematical correctness baseline. */
void ranc_cpu_vmm(const int32_t x[5], int32_t y[8]);
#endif
