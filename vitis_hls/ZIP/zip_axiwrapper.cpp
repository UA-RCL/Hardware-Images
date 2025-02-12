#include <assert.h>
#include <ap_axi_sdata.h>
#include "hls_stream.h"
#include "zip.h"

//typedef ap_axiu<32,1,1,1> AXI_VAL;
typedef hls::axis<float,1,1,1> AXI_VAL;

// How many total values are we going to need to stream in? i.e. how many entries are in both the A and B matrices?
#define INPUT_SIZE ((2 * A_ROWS * A_COLS) + (2 * B_ROWS * B_COLS))

// How many total values are we going to need to stream out? i.e. how many entries are in the C matrix?
#define OUTPUT_SIZE  (2 * C_ROWS * C_COLS)

void wrapper_zip_hw(hls::stream<AXI_VAL> &in_stream, hls::stream<AXI_VAL> &out_stream, int size, int op) {
    ZIP_T A [MAX_SIZE];
    ZIP_T B [MAX_SIZE];
    ZIP_T C [MAX_SIZE];
    
    AXI_VAL val;
    // Stream in the input vector A
    for (int i = 0; i < 2 * size; i++) {
#pragma HLS LOOP_TRIPCOUNT min = 2 max = MAX_SIZE
#pragma HLS PIPELINE II=1
        val=in_stream.read();
        A[i] = val.data;
    }
    // Stream in the input vector B
    for (int i = 0; i < 2 * size; i++) {
#pragma HLS LOOP_TRIPCOUNT min = 2 max = MAX_SIZE
#pragma HLS PIPELINE II=1
        val=in_stream.read();
        B[i] = val.data;
    }

    // Do the zip
    switch(op){
        case 0:
            zip_add_top(A, B, C, size);
            break;
        case 1:
            zip_sub_top(A, B, C, size);
            break;
        case 2:
            zip_mult_top(A, B, C, size);
            break;
        case 3:
            zip_div_top(A, B, C, size);
            break;
        default:
            zip_add_top(A, B, C, size);
            break;
    }
    
    // Stream out the result matrices
    for (int i = 0; i < 2 * size; i++) {
#pragma HLS LOOP_TRIPCOUNT min = 2 max = MAX_SIZE
#pragma HLS PIPELINE II=1
        val.data = C[i];
        val.last = (i == (2 * size - 1));
        val.keep = 0xf;
        val.strb = 0xf;
        out_stream.write(val);
    }
}

void ZIP_HLS_accel (hls::stream<AXI_VAL> &INPUT_STREAM, hls::stream<AXI_VAL> &OUTPUT_STREAM, int size, int op){
#pragma HLS INTERFACE axis      port=OUTPUT_STREAM
#pragma HLS INTERFACE axis      port=INPUT_STREAM
#pragma HLS INTERFACE s_axilite port=size   bundle=ctrl
#pragma HLS INTERFACE s_axilite port=op     bundle=ctrl
#pragma HLS INTERFACE s_axilite port=return bundle=ctrl
// #pragma HLS INTERFACE ap_ctrl_none port=return

    wrapper_zip_hw(INPUT_STREAM, OUTPUT_STREAM, size ,op);

    return;
}
