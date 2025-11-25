
#include <assert.h>
#include <ap_axi_sdata.h>
#include <hls_math.h>
#include "hls_stream.h"
//#include "complex_matrix_multiply.h"

typedef float data_t;

typedef float cedr_re_flt_type;
typedef struct {
    data_t re;
    data_t im;
} cedr_cmplx_flt_type;

typedef double cedr_re_dbl_type;
typedef struct {
    double re;
    double im;
} cedr_cmplx_dbl_type;

const unsigned A_ROWS = 8;
const unsigned A_COLS = 64;
const unsigned B_ROWS = 64;
const unsigned B_COLS = 8;
const unsigned C_ROWS = A_ROWS;
const unsigned C_COLS = B_COLS;

void wrapper_complex_gemm_flt_hw(
    cedr_cmplx_flt_type *input_1,
    cedr_cmplx_flt_type *input_2,
    cedr_cmplx_flt_type *output)
{
    data_t temp_sumr[B_COLS];
    data_t temp_sumi[B_COLS];

lreorder1:
    for (int i = 0; i < A_ROWS; i++) {
    lreorder2:
        for (int k = 0; k < B_ROWS; k++) {
        lreorder3:
            for (int j = 0; j < B_COLS; j++) {
#pragma HLS PIPELINE II=1
                data_t resultr = (k == 0) ? 0 : temp_sumr[j];
                data_t resulti = (k == 0) ? 0 : temp_sumi[j];

                data_t a_re = input_1[i * A_COLS + k].re;
                data_t a_im = input_1[i * A_COLS + k].im;
                data_t b_re = input_2[k * B_COLS + j].re;
                data_t b_im = input_2[k * B_COLS + j].im;

                resultr += a_re * b_re - a_im * b_im;
                resulti += a_re * b_im + a_im * b_re;

                temp_sumr[j] = resultr;
                temp_sumi[j] = resulti;

                if (k == B_ROWS - 1) {
                    output[i * B_COLS + j].re = resultr;
                    output[i * B_COLS + j].im = resulti;
                }
            }
        }
    }
}

void complex_gemm_flt(cedr_cmplx_dbl_type *input1, cedr_cmplx_dbl_type *input2, cedr_cmplx_dbl_type *output)
{
    #pragma HLS INTERFACE m_axi offset=SLAVE bundle=gmem port=input1  max_read_burst_length=256
    #pragma HLS INTERFACE m_axi offset=SLAVE bundle=gmem port=input2  max_read_burst_length=256
    #pragma HLS INTERFACE m_axi offset=SLAVE bundle=gmem port=output  max_read_burst_length=256

    #pragma HLS INTERFACE s_axilite port=input1  bundle=control
    #pragma HLS INTERFACE s_axilite port=input2  bundle=control
    #pragma HLS INTERFACE s_axilite port=output  bundle=control
    #pragma HLS INTERFACE s_axilite port=return bundle=control

    static cedr_cmplx_flt_type input1_flt[A_ROWS * A_COLS];
    static cedr_cmplx_flt_type input2_flt[A_COLS * B_COLS];
    static cedr_cmplx_flt_type output_flt[A_ROWS * B_COLS];

    convert_input1:
    for (size_t i = 0; i < A_ROWS * A_COLS; i++) {
        #pragma HLS pipeline II=1
        input1_flt[i].re = static_cast<cedr_re_flt_type>(input1[i].re);
        input1_flt[i].im = static_cast<cedr_re_flt_type>(input1[i].im);
    }

    convert_input2:
    for (size_t i = 0; i < A_COLS * B_COLS; i++) {
        #pragma HLS pipeline II=1
        input2_flt[i].re = static_cast<cedr_re_flt_type>(input2[i].re);
        input2_flt[i].im = static_cast<cedr_re_flt_type>(input2[i].im);
    }

    wrapper_complex_gemm_flt_hw(input1_flt, input2_flt, output_flt);

    convert_output:
    for (size_t i = 0; i < A_ROWS * B_COLS; i++) {
        #pragma HLS pipeline II=1
        output[i].re = static_cast<cedr_re_dbl_type>(output_flt[i].re);
        output[i].im = static_cast<cedr_re_dbl_type>(output_flt[i].im);
    }
}