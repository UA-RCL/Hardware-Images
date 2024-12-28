// #include <ap_int.h>
#include <stdint.h>

#define KERNEL_SIZE 7
#define PADDING 3  // Padding size

typedef float data_t;

void Filter2DKernel(data_t *input, data_t *output, data_t kernel[KERNEL_SIZE][KERNEL_SIZE], int rows, int cols) {
#pragma HLS INTERFACE m_axi port=input offset=slave bundle=gmem0 // Direct memory access for input
#pragma HLS INTERFACE m_axi port=output offset=slave bundle=gmem1 // Direct memory access for output

#pragma HLS INTERFACE s_axilite port=input bundle=control  // Kernel coefficients
#pragma HLS INTERFACE s_axilite port=output bundle=control  // Kernel coefficients
#pragma HLS INTERFACE s_axilite port=kernel bundle=control  // Kernel coefficients
#pragma HLS INTERFACE s_axilite port=rows bundle=control    // Input height
#pragma HLS INTERFACE s_axilite port=cols bundle=control    // Input width
#pragma HLS INTERFACE s_axilite port=return bundle=control  // Control interface

    const int padded_rows = rows + 2 * PADDING;
    const int padded_cols = cols + 2 * PADDING;

    data_t input_buffer[KERNEL_SIZE][1024]; // Buffer for sliding window
#pragma HLS ARRAY_PARTITION variable=input_buffer complete dim=1

    // Initialize sliding window buffer (First KERNEL_SIZE rows)
    for (int r = 0; r < KERNEL_SIZE - 1; r++) {
        for (int c = 0; c < padded_cols; c++) {
#pragma HLS PIPELINE
            if (r < PADDING || c < PADDING || r >= rows + PADDING || c >= cols + PADDING) {
                input_buffer[r][c] = 0;  // Pad with zeros
            } else {
                input_buffer[r][c] = input[(r - PADDING) * cols + (c - PADDING)];
            }
        }
    }

    // Sliding window convolution
    for (int r = 0; r < padded_rows; r++) {
        for (int c = 0; c < padded_cols; c++) {
#pragma HLS PIPELINE
            // Shift buffer to simulate sliding window
            for (int k = KERNEL_SIZE - 1; k > 0; k--) {
                input_buffer[k][c] = input_buffer[k - 1][c];
            }

            if (r < PADDING || c < PADDING || r >= rows + PADDING || c >= cols + PADDING) {
                input_buffer[0][c] = 0;  // Pad with zeros
            } else {
                input_buffer[0][c] = input[(r - PADDING) * cols + (c - PADDING)];
            }

            // Perform convolution only if the window is valid
            if (r >= KERNEL_SIZE - 1 && c >= KERNEL_SIZE - 1) {
                data_t acc = 0;
                for (int kr = 0; kr < KERNEL_SIZE; kr++) {
                    for (int kc = 0; kc < KERNEL_SIZE; kc++) {
                        acc += kernel[kr][kc] * input_buffer[kr][c - (KERNEL_SIZE - 1) + kc];
                    }
                }
                output[(r - KERNEL_SIZE + 1) * (cols - KERNEL_SIZE + 1) + (c - KERNEL_SIZE + 1)] = acc;
            }
        }
    }
}
