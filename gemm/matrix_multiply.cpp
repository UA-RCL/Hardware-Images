
#include <assert.h>
#include <ap_axi_sdata.h>
#include <hls_math.h>
#include "hls_stream.h"
// #include "matrix_multiply.h"

typedef float data_t;

const unsigned A_ROWS = 512;
const unsigned A_COLS = 512;
const unsigned B_ROWS = 512;
const unsigned B_COLS = 512;
const unsigned C_ROWS = A_ROWS;
const unsigned C_COLS = B_COLS;

// How many total values are we going to need to stream in? i.e. how many entries are in both the A and B matrices?
#define INPUT_SIZE (A_ROWS*A_COLS + B_ROWS*B_COLS)

// How many total values are we going to need to stream out? i.e. how many entries are in the C matrix?
#define OUTPUT_SIZE  (C_ROWS * C_COLS)

void wrapper_mmult_hw(data_t *input1, data_t*input2, data_t* output) {

    data_t temp_sumr[C_COLS];

lreorder1:
    for (int i = 0; i < A_ROWS; i++) {
#pragma HLS LOOP_TRIPCOUNT min = C_ROWS max = C_ROWS
    lreorder2:
        for (int k = 0; k < B_ROWS; k++) {
#pragma HLS LOOP_TRIPCOUNT min = B_ROWS max = B_ROWS
        lreorder3:
            for (int j = 0; j < B_COLS; j++) {
                data_t resultr = (k == 0) ? 0 : temp_sumr[j];
                resultr += input1[i * A_ROWS + k] * input2[k * A_ROWS + j];
                temp_sumr[j] = resultr;
                if (k == B_ROWS - 1) {
                    output[i * C_ROWS + j] = resultr;
                }
            }
        }
    }
}

void matrix_multiply(data_t *input1, data_t *input2, data_t *output){

#pragma HLS INTERFACE m_axi offset=SLAVE bundle=gmem port=input1 max_read_burst_length = 256
#pragma HLS INTERFACE m_axi offset=SLAVE bundle=gmem port=input2 max_read_burst_length = 256
#pragma HLS INTERFACE m_axi offset=SLAVE bundle=gmem port=output max_read_burst_length = 256

#pragma HLS INTERFACE s_axilite port=input1  bundle=control
#pragma HLS INTERFACE s_axilite port=input2  bundle=control
#pragma HLS INTERFACE s_axilite port=output  bundle=control
#pragma HLS INTERFACE s_axilite port=return bundle=control


wrapper_mmult_hw(input1, input2, output);

}