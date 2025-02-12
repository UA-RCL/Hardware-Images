#include <assert.h>
#include <ap_axi_sdata.h>
#include <hls_math.h>
#include "hls_stream.h"
#include "matrix_multiply.h"

// typedef ap_axiu<32,4,5,5> AXI_VAL;
// typedef ap_axiu<32,1,1,1> AXI_VAL;
typedef hls::axis<float,1,1,1> AXI_VAL;

// hls::axis<ap_int<WData>, WUser, WId, WDest>

// Functions to insert and extract elements from an axi stream
// includes conversion to the correct data type (stolen from xilinx example xapp1170)
// template <typename T, int U, int TI, int TD>
// T pop_stream(ap_axiu <sizeof(T)*8,U,TI,TD> const &e)
// {
// #pragma HLS INLINE
// 
// 	assert(sizeof(T) == sizeof(int));
// 	union
// 	{
// 		int ival;
// 		T oval;
// 	} converter;
// 	converter.ival = e.data;
// 	T ret = converter.oval;
// 
// 	volatile ap_uint<sizeof(T)> strb = e.strb;
// 	volatile ap_uint<sizeof(T)> keep = e.keep;
// 	volatile ap_uint<U> user = e.user;
// 	volatile ap_uint<1> last = e.last;
// 	volatile ap_uint<TI> id = e.id;
// 	volatile ap_uint<TD> dest = e.dest;
// 
// 	return ret;
// }
// 
// template <typename T, int U, int TI, int TD>
// ap_axiu <sizeof(T)*8,U,TI,TD> push_stream(T const &v, bool last = false)
// {
// 	ap_axiu<sizeof(T)*8,U,TI,TD> e;
// 
// 	assert(sizeof(T) == sizeof(int));
// 	union
// 	{
// 		int oval;
// 		T ival;
// 	} converter;
// 	converter.ival = v;
// 	e.data = converter.oval;
// 
// 	// set it to sizeof(T) ones
// 	e.strb = -1;
// 	e.keep = 15;
// 	e.user = 0;
// 	e.last = last ? 1 : 0;
// 	e.id = 0;
// 	e.dest = 0;
// 	return e;
// }

// How many total values are we going to need to stream in? i.e. how many entries are in both the A and B matrices?
#define INPUT_SIZE (2*A_ROWS*A_COLS + 2*B_ROWS*B_COLS)

// How many total values are we going to need to stream out? i.e. how many entries are in the C matrix?
#define OUTPUT_SIZE  (2 * C_ROWS * C_COLS)

// void wrapper_mmult_hw(MATRIX_T in_stream[INPUT_SIZE], MATRIX_T out_stream[OUTPUT_SIZE]) {
// void wrapper_mmult_hw(hls::stream<MATRIX_T> &in_stream, hls::stream<AXI_VAL> &out_stream) {
void wrapper_mmult_hw(hls::stream<AXI_VAL> &in_stream, hls::stream<AXI_VAL> &out_stream) {
	MATRIX_T Ar [A_ROWS][A_COLS];
	MATRIX_T Ai [A_ROWS][A_COLS];
	MATRIX_T Br [B_ROWS][B_COLS];
	MATRIX_T Bi [B_ROWS][B_COLS];
	// MATRIX_T Cr[C_ROWS][C_COLS];
	// MATRIX_T Ci[C_ROWS][C_COLS];
	MATRIX_T C[2 * C_ROWS][C_COLS];
	assert(sizeof(MATRIX_T)*8 == 32);

    AXI_VAL read_data;
	
	// Stream in the 4 input matrices
	for (int i = 0; i < A_ROWS; i++) {
#pragma HLS LOOP_TRIPCOUNT min = A_ROWS max = A_ROWS
		for (int j = 0; j < A_COLS; j++) {
#pragma HLS LOOP_TRIPCOUNT min = A_COLS max = A_COLS
			#pragma HLS PIPELINE II=1
			// int k = i*A_COLS + j;
			// Ar[i][j] = pop_stream<MATRIX_T,4,5,5>(in_stream[k]);
			in_stream.read(read_data);
			Ar[i][j] = read_data.data;
		}
	}
	for (int i = 0; i < A_ROWS; i++) {
#pragma HLS LOOP_TRIPCOUNT min = A_ROWS max = A_ROWS
		for (int j = 0; j < A_COLS; j++) {
#pragma HLS LOOP_TRIPCOUNT min = A_COLS max = A_COLS
			#pragma HLS PIPELINE II=1
			// int k = i*A_COLS + j + (A_ROWS * A_COLS);
			// Ai[i][j] = pop_stream<MATRIX_T,4,5,5>(in_stream[k]);
			in_stream.read(read_data);
			Ai[i][j] = read_data.data;
		}
	}
	
	for (int i = 0; i < B_ROWS; i++) {
#pragma HLS LOOP_TRIPCOUNT min = B_ROWS max = B_ROWS
		for (int j = 0; j < B_COLS; j++) {
#pragma HLS LOOP_TRIPCOUNT min = B_COLS max = B_COLS
			#pragma HLS PIPELINE II=1
			// int k = i*B_COLS + j + (2 * A_ROWS * A_COLS);
			// Br[i][j] = pop_stream<MATRIX_T,4,5,5>(in_stream[k]);
			in_stream.read(read_data);
			Br[i][j] = read_data.data;
		}
	}
	
	for (int i = 0; i < B_ROWS; i++) {
#pragma HLS LOOP_TRIPCOUNT min = B_ROWS max = B_ROWS
		for (int j = 0; j < B_COLS; j++) {
#pragma HLS LOOP_TRIPCOUNT min = B_COLS max = B_COLS
			#pragma HLS PIPELINE II=1
			// int k = i*B_COLS + j + (2 * A_ROWS * A_COLS) + (B_ROWS * B_COLS);
			// Bi[i][j] = pop_stream<MATRIX_T,4,5,5>(in_stream[k]);
			in_stream.read(read_data);
			Bi[i][j] = read_data.data;
		}
	}

	// Do the multiplication
	matrix_multiply_top(Ar,Ai,Br,Bi,&C[0],&C[C_ROWS]);
    AXI_VAL write_data;
	
// 	// Stream out the result matrices
// 	for (int i = 0; i < C_ROWS; i++) {
// #pragma HLS LOOP_TRIPCOUNT min = C_ROWS max = C_ROWS
// 		for (int j = 0; j < C_COLS; j++) {
// #pragma HLS LOOP_TRIPCOUNT min = C_COLS max = C_COLS
// 			#pragma HLS PIPELINE II=1
// 			// int k = i * C_COLS + j;
// 			// out_stream[k] = push_stream<MATRIX_T,4,5,5>(Cr[i][j], false);
//             write_data.data = Cr[i][j];
//             write_data.last = false;
// 			out_stream.write(write_data);
// 		}
// 	}
// 
// 	for (int i = 0; i < C_ROWS; i++) {
// #pragma HLS LOOP_TRIPCOUNT min = C_ROWS max = C_ROWS
// 		for (int j = 0; j < C_COLS; j++) {
// #pragma HLS LOOP_TRIPCOUNT min = C_COLS max = C_COLS
// 			#pragma HLS PIPELINE II=1
// 			int k = i * C_COLS + j + (C_ROWS * C_COLS);
// 			// out_stream[k] = push_stream<MATRIX_T,4,5,5>(Ci[i][j], k == (2*C_ROWS*C_COLS - 1));
//             write_data.data = Ci[i][j];
//             write_data.last = (k == (2*C_ROWS*C_COLS - 1));
// 			out_stream.write(write_data);
// 		}
// 	}

	for (int i = 0; i < 2 * C_ROWS; i++) {
#pragma HLS LOOP_TRIPCOUNT min = C_ROWS max = C_ROWS
		for (int j = 0; j < C_COLS; j++) {
#pragma HLS LOOP_TRIPCOUNT min = C_COLS max = C_COLS
			#pragma HLS PIPELINE II=1
			// int k = i * C_COLS + j + (C_ROWS * C_COLS);
			// out_stream[k] = push_stream<MATRIX_T,4,5,5>(Ci[i][j], k == (2*C_ROWS*C_COLS - 1));
            // write_data.data = *((unsigned int*)((void*)&C[i][j]));
            // write_data.data = hls::cast_IEEE754<int32_t>(C[i][j]);
            write_data.data = C[i][j];
            write_data.keep = 0xF;
            write_data.strb = 0xF;
            // write_data.last = ((i * C_COLS + j) == (2 * C_ROWS * C_COLS - 1));
            write_data.last = ((i == 2 * C_ROWS - 1) && (j == 2 * C_COLS - 1));
			out_stream.write(write_data);
            // out_stream[i * C_COLS + j] = C[i][j];
		}
	}
}

// void HLS_accel (hls::stream<MATRIX_T> &INPUT_STREAM, hls::stream<AXI_VAL> &OUTPUT_STREAM)
// void HLS_accel (hls::stream<MATRIX_T> &INPUT_STREAM, MATRIX_T OUTPUT_STREAM[OUTPUT_SIZE])
void HLS_accel (hls::stream<AXI_VAL> &INPUT_STREAM, hls::stream<AXI_VAL> &OUTPUT_STREAM)
{
#pragma HLS INTERFACE ap_ctrl_none port=return
#pragma HLS INTERFACE axis		port=OUTPUT_STREAM
#pragma HLS INTERFACE axis		port=INPUT_STREAM

	// wrapper_mmult_hw(INPUT_STREAM, OUTPUT_STREAM);
	wrapper_mmult_hw(INPUT_STREAM, OUTPUT_STREAM);

	return;
}
