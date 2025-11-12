
#include <assert.h>
#include <ap_axi_sdata.h>
#include <hls_math.h>
#include "hls_stream.h"
//#include "complex_matrix_multiply.h"

typedef int data_t;
typedef long long accum_t;  // Use 64-bit for intermediate accumulations

typedef struct {
    data_t re;
    data_t im;
} cedr_cmplx_int_type;

// Saturation function: clamp 64-bit value to 32-bit int range
inline data_t saturate_to_int(accum_t value) {
    const accum_t INT_MAX_VAL = (accum_t)2147483647LL;
    const accum_t INT_MIN_VAL = (accum_t)(-2147483647LL - 1);
    if (value > INT_MAX_VAL) return INT_MAX_VAL;
    if (value < INT_MIN_VAL) return INT_MIN_VAL;
    return (data_t)value;
}

const unsigned A_ROWS = 8;
const unsigned A_COLS = 64;
const unsigned B_ROWS = 64;
const unsigned B_COLS = 8;
const unsigned C_ROWS = A_ROWS;
const unsigned C_COLS = B_COLS;

void wrapper_complex_gemm_int_hw(
    cedr_cmplx_int_type *input_1,
    cedr_cmplx_int_type *input_2,
    cedr_cmplx_int_type *output)
{
    accum_t temp_sumr[B_COLS];
    accum_t temp_sumi[B_COLS];

lreorder1:
    for (int i = 0; i < A_ROWS; i++) {
    lreorder2:
        for (int k = 0; k < B_ROWS; k++) {
        lreorder3:
            for (int j = 0; j < B_COLS; j++) {
#pragma HLS PIPELINE II=1
                accum_t resultr = (k == 0) ? 0 : temp_sumr[j];
                accum_t resulti = (k == 0) ? 0 : temp_sumi[j];

                data_t a_re = input_1[i * A_COLS + k].re;
                data_t a_im = input_1[i * A_COLS + k].im;
                data_t b_re = input_2[k * B_COLS + j].re;
                data_t b_im = input_2[k * B_COLS + j].im;

                // Use 64-bit intermediate for multiplications to prevent overflow
                accum_t prod_re_re = (accum_t)a_re * (accum_t)b_re;
                accum_t prod_im_im = (accum_t)a_im * (accum_t)b_im;
                accum_t prod_re_im = (accum_t)a_re * (accum_t)b_im;
                accum_t prod_im_re = (accum_t)a_im * (accum_t)b_re;

                resultr += prod_re_re - prod_im_im;
                resulti += prod_re_im + prod_im_re;

                temp_sumr[j] = resultr;
                temp_sumi[j] = resulti;

                if (k == B_ROWS - 1) {
                    // Saturate to 32-bit range to prevent undefined behavior
                    output[i * B_COLS + j].re = saturate_to_int(resultr);
                    output[i * B_COLS + j].im = saturate_to_int(resulti);
                }
            }
        }
    }
}

void complex_gemm_int(cedr_cmplx_int_type *input1, cedr_cmplx_int_type *input2, cedr_cmplx_int_type *output){

#pragma HLS INTERFACE m_axi offset=SLAVE bundle=gmem port=input1 max_read_burst_length = 256
#pragma HLS INTERFACE m_axi offset=SLAVE bundle=gmem port=input2 max_read_burst_length = 256
#pragma HLS INTERFACE m_axi offset=SLAVE bundle=gmem port=output max_read_burst_length = 256

#pragma HLS INTERFACE s_axilite port=input1  bundle=control
#pragma HLS INTERFACE s_axilite port=input2  bundle=control
#pragma HLS INTERFACE s_axilite port=output  bundle=control
#pragma HLS INTERFACE s_axilite port=return bundle=control


wrapper_complex_gemm_int_hw(input1, input2, output);

}