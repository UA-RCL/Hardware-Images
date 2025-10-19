
#include <assert.h>
#include <ap_axi_sdata.h>
#include <hls_math.h>
#include "hls_stream.h"


typedef double data_t;

typedef struct {
    data_t re;
    data_t im;
} cedr_cmplx_double_type;

const unsigned A_ROWS = 8;
const unsigned A_COLS = 64;
const unsigned B_ROWS = 64;
const unsigned B_COLS = 8;
const unsigned C_ROWS = A_ROWS;
const unsigned C_COLS = B_COLS;

// How many total values are we going to need to stream in? i.e. how many entries are in both the A and B matrices?
#define INPUT_SIZE (A_ROWS*A_COLS + B_ROWS*B_COLS)

// How many total values are we going to need to stream out? i.e. how many entries are in the C matrix?
#define OUTPUT_SIZE  (C_ROWS * C_COLS)

void wrapper_complex_gemv_double_hw(
    cedr_cmplx_double_type *input_1, // Matrix A
    cedr_cmplx_double_type *input_2, // Vector x
    cedr_cmplx_double_type *output   // Result y
) {
#pragma HLS INLINE off

    data_t resultr, resulti;

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

            // Complex multiply-accumulate
            resultr += a_re * b_re - a_im * b_im;
            resulti += a_re * b_im + a_im * b_re;
        }

        // Write out final complex result for this row
        output[i].re = resultr;
        output[i].im = resulti;
    }
}

void complex_gemv_double(cedr_cmplx_double_type *input1, cedr_cmplx_double_type *input2, cedr_cmplx_double_type *output){

#pragma HLS INTERFACE m_axi offset=SLAVE bundle=gmem port=input1 max_read_burst_length = 256
#pragma HLS INTERFACE m_axi offset=SLAVE bundle=gmem port=input2 max_read_burst_length = 256
#pragma HLS INTERFACE m_axi offset=SLAVE bundle=gmem port=output max_read_burst_length = 256

#pragma HLS INTERFACE s_axilite port=input1  bundle=control
#pragma HLS INTERFACE s_axilite port=input2  bundle=control
#pragma HLS INTERFACE s_axilite port=output  bundle=control
#pragma HLS INTERFACE s_axilite port=return bundle=control


wrapper_complex_gemv_double_hw(input1, input2, output);

}