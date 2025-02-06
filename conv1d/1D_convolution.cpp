#include <hls_stream.h>
#include <ap_int.h>

#define KERNEL_SIZE 3

void conv1d(float* input, int input_size, const float kernel[KERNEL_SIZE], float* output) {
    #pragma HLS INTERFACE m_axi port=input offset=slave
    #pragma HLS INTERFACE m_axi port=output offset=slave
    
    #pragma HLS INTERFACE mode=s_axilite port=return
    #pragma HLS INTERFACE mode=s_axilite port=input
    #pragma HLS INTERFACE mode=s_axilite port=input_size
    #pragma HLS INTERFACE mode=s_axilite port=kernel
    #pragma HLS INTERFACE mode=s_axilite port=output

    int n = KERNEL_SIZE / 2;

    for (int i = 0; i < input_size; i++) {
        #pragma HLS PIPELINE
        float sum = 0;
        for (int j = 0; j < KERNEL_SIZE; j++) {
            int index = i - n + j;
            if (index < 0 || index >= input_size) {
                continue;
            }
            sum += input[index] * kernel[j];
        }
        output[i] = sum;
    }
}