
#include <assert.h>
#include <ap_axi_sdata.h>
#include <hls_math.h>
#include "hls_stream.h"


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
const unsigned A_COLS = 8;
const unsigned B_ROWS = 8;
const unsigned B_COLS = 1;
const unsigned C_ROWS = A_ROWS;
const unsigned C_COLS = B_COLS;

void wrapper_complex_gemv_int_hw(
    cedr_cmplx_int_type *input_1, // Matrix A
    cedr_cmplx_int_type *input_2, // Vector x
    cedr_cmplx_int_type *output   // Result y
) {
#pragma HLS INLINE off

    accum_t resultr, resulti;

row_loop:
    for (int i = 0; i < A_ROWS; i++) {
#pragma HLS LOOP_TRIPCOUNT min=A_ROWS max=A_ROWS
        resultr = 0;
        resulti = 0;

    col_loop:
        for (int j = 0; j < A_COLS; j++) {
#pragma HLS PIPELINE II=1
#pragma HLS LOOP_TRIPCOUNT min=A_COLS max=A_COLS
            data_t a_re = input_1[i * A_COLS + j].re;
            data_t a_im = input_1[i * A_COLS + j].im;
            data_t b_re = input_2[j].re;
            data_t b_im = input_2[j].im;

            // Use 64-bit intermediate for multiplications to prevent overflow
            accum_t prod_re_re = (accum_t)a_re * (accum_t)b_re;
            accum_t prod_im_im = (accum_t)a_im * (accum_t)b_im;
            accum_t prod_re_im = (accum_t)a_re * (accum_t)b_im;
            accum_t prod_im_re = (accum_t)a_im * (accum_t)b_re;

            // Complex multiply-accumulate
            resultr += prod_re_re - prod_im_im;
            resulti += prod_re_im + prod_im_re;
        }

        // Saturate to 32-bit range to prevent undefined behavior
        output[i].re = saturate_to_int(resultr);
        output[i].im = saturate_to_int(resulti);
    }
}

void complex_gemv_int(cedr_cmplx_int_type *input1, cedr_cmplx_int_type *input2, cedr_cmplx_int_type *output){

#pragma HLS INTERFACE m_axi offset=SLAVE bundle=gmem port=input1 max_read_burst_length = 256
#pragma HLS INTERFACE m_axi offset=SLAVE bundle=gmem port=input2 max_read_burst_length = 256
#pragma HLS INTERFACE m_axi offset=SLAVE bundle=gmem port=output max_read_burst_length = 256

#pragma HLS INTERFACE s_axilite port=input1  bundle=control
#pragma HLS INTERFACE s_axilite port=input2  bundle=control
#pragma HLS INTERFACE s_axilite port=output  bundle=control
#pragma HLS INTERFACE s_axilite port=return bundle=control


wrapper_complex_gemv_int_hw(input1, input2, output);

}