/*
 * (C) Copyright [2024] Hewlett Packard Enterprise Development LP
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the Software),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include <iostream>
#include <cstdio>
#include <hls_math.h>
#include "hls_stream.h"
#include "ap_axi_sdata.h"

#define MAX_INPUT_H 256
#define MAX_INPUT_W 256
#define MAX_KERNEL_H 5
#define MAX_KERNEL_W 5

typedef float data_t;
typedef hls::axis<data_t,1,1,1> data_axis_t;

using namespace std;

void pad_input(hls::stream<data_t> &pad_input_input, data_t *pad_input_output, int input_h, int input_w, int padding) {
    if (padding == 0) {
        for (int i = 0; i < input_h; i++) {
            for (int j = 0; j < input_w; j++) {
                #pragma HLS pipeline II=1
                pad_input_output[i * input_w + j] = pad_input_input.read();
            }
        }
        return;
    }

    // Zero-initialize padded output
    for (int i = 0; i < input_h + 2 * padding; i++) {
        for (int j = 0; j < input_w + 2 * padding; j++) {
            #pragma HLS pipeline II=1
            pad_input_output[i * (input_w + 2 * padding) + j] = 0;
        }
    }

    // Fill in the input pixels from stream into padded positions
    for (int i = 0; i < input_h; i++) {
        for (int j = 0; j < input_w; j++) {
            #pragma HLS pipeline II=1
            pad_input_output[(i + padding) * (input_w + 2 * padding) + j + padding] = pad_input_input.read();
        }
    }
}

extern "C" {

void conv2d(
        hls::stream<data_t>      &input_stream,
        hls::stream<data_t>      &kernel_stream,
        data_t                    conv2d_bias,
        int                       stride,
        int                       padding,
        int                       input_h,
        int                       input_w,
        int                       kernel_h,
        int                       kernel_w,
        hls::stream<data_axis_t> &output_stream)
{
#pragma HLS INTERFACE axis      port=input_stream
#pragma HLS INTERFACE axis      port=kernel_stream
#pragma HLS INTERFACE axis      port=output_stream
#pragma HLS INTERFACE s_axilite port=conv2d_bias   bundle=control
#pragma HLS INTERFACE s_axilite port=stride        bundle=control
#pragma HLS INTERFACE s_axilite port=padding       bundle=control
#pragma HLS INTERFACE s_axilite port=input_h       bundle=control
#pragma HLS INTERFACE s_axilite port=input_w       bundle=control
#pragma HLS INTERFACE s_axilite port=kernel_h      bundle=control
#pragma HLS INTERFACE s_axilite port=kernel_w      bundle=control
#pragma HLS INTERFACE s_axilite port=return        bundle=control

    // 256x256 input + up to 4 pixels of padding on each side (for a 5x5 kernel)
    data_t input_padded[(MAX_INPUT_H + MAX_KERNEL_H - 1) * (MAX_INPUT_W + MAX_KERNEL_W - 1)];
    data_t conv2d_kernel[MAX_KERNEL_H * MAX_KERNEL_W];
#pragma HLS ARRAY_PARTITION variable=conv2d_kernel complete dim=1

    // Read kernel coefficients from stream into local buffer
    load_kernel: for (int i = 0; i < kernel_h * kernel_w; i++) {
#pragma HLS PIPELINE
        conv2d_kernel[i] = kernel_stream.read();
    }

    // Pad input (reads from input_stream into local padded buffer)
    pad_input(input_stream, input_padded, input_h, input_w, padding);

    int output_h = (input_h + 2 * padding - kernel_h) / stride + 1;
    int output_w = (input_w + 2 * padding - kernel_w) / stride + 1;

    // Perform convolution and write results to output stream
    apply_conv: for (int i = 0; i < output_h; i++) {
        for (int j = 0; j < output_w; j++) {
            data_t tmp = 0;
            for (int k = 0; k < kernel_h; k++) {
                for (int l = 0; l < kernel_w; l++) {
#pragma HLS PIPELINE II=1
                    tmp += input_padded[(i * stride + k) * (input_w + 2 * padding) + j * stride + l]
                           * conv2d_kernel[k * kernel_w + l];
                }
            }
            data_axis_t out_pix;
            out_pix.data = tmp + conv2d_bias;
            out_pix.keep = -1;
            out_pix.last = ((i == output_h - 1) && (j == output_w - 1));
            output_stream.write(out_pix);
        }
    }
}

} // extern "C"