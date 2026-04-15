#include <limits.h>
#include <ap_int.h>
#include <assert.h>
#include <ap_axi_sdata.h>
#include <hls_math.h>
#include "hls_stream.h"

#define CONV_2D_MAX 32767
#define CONV_2D_MIN 0
#define INT_MASK_SCALE 256

typedef int cedr_re_int_type;

// Max supported kernel size
#define MAX_KERNEL_SIZE 11

// Fixed image dimensions
#define IMAGE_HEIGHT 717
#define IMAGE_WIDTH 1276

extern "C" {
void cedr_conv_2d_int(int *input, int *mask, int mask_size, int *output) {
#pragma HLS INTERFACE m_axi port=input offset=slave bundle=gmem0
#pragma HLS INTERFACE m_axi port=mask offset=slave bundle=gmem1
#pragma HLS INTERFACE m_axi port=output offset=slave bundle=gmem2
#pragma HLS INTERFACE s_axilite port=input bundle=control
#pragma HLS INTERFACE s_axilite port=mask bundle=control
#pragma HLS INTERFACE s_axilite port=output bundle=control
#pragma HLS INTERFACE s_axilite port=mask_size bundle=control
#pragma HLS INTERFACE s_axilite port=return bundle=control

    // Fixed dimensions
    const int height = IMAGE_HEIGHT;
    const int width  = IMAGE_WIDTH;

    // Bounds check for kernel size
    if (mask_size > MAX_KERNEL_SIZE) {
        return;
    }

    int mask_elems = mask_size * mask_size;

    static cedr_re_int_type mask_int[MAX_KERNEL_SIZE * MAX_KERNEL_SIZE];

    for (int i = 0; i < mask_elems; i++) {
#pragma HLS PIPELINE II=1
        mask_int[i] = mask[i];
    }

    // Local Buffers
    int line_buffer[MAX_KERNEL_SIZE - 1][IMAGE_WIDTH];
#pragma HLS ARRAY_PARTITION variable=line_buffer complete dim=1

    int window[MAX_KERNEL_SIZE][MAX_KERNEL_SIZE];
#pragma HLS ARRAY_PARTITION variable=window complete dim=0

    int mask_local[MAX_KERNEL_SIZE][MAX_KERNEL_SIZE];
#pragma HLS ARRAY_PARTITION variable=mask_local complete dim=0

    int z = mask_size / 2;

    // Read mask into local memory
    for (int i = 0; i < mask_size; i++) {
        for (int j = 0; j < mask_size; j++) {
#pragma HLS PIPELINE II=1
            mask_local[i][j] = mask_int[i * mask_size + j];
        }
    }

    // Initialize line_buffer and window
    for (int i = 0; i < MAX_KERNEL_SIZE - 1; i++) {
        for (int j = 0; j < IMAGE_WIDTH; j++) {
#pragma HLS PIPELINE II=1
            line_buffer[i][j] = 0;
        }
    }

    for (int i = 0; i < MAX_KERNEL_SIZE; i++) {
        for (int j = 0; j < MAX_KERNEL_SIZE; j++) {
#pragma HLS PIPELINE II=1
            window[i][j] = 0;
        }
    }

    // ------------------------------------------------------------------
    // FIX 2: Clear output first so any unwritten tail/border pixels are 0,
    // not stale memory.
    // ------------------------------------------------------------------
    int total_elements = height * width;
    for (int i = 0; i < total_elements; i++) {
#pragma HLS PIPELINE II=1
        output[i] = 0;
    }

    // Pointers for current row/col
    int row = 0;
    int col = 0;

    for (int i = 0; i < total_elements; i++) {
#pragma HLS PIPELINE II=4
        // Read input from global memory
        int pixel = input[i];

        // Shift Window Column (move left)
        for (int k = 0; k < MAX_KERNEL_SIZE; k++) {
            for (int w_idx = 0; w_idx < MAX_KERNEL_SIZE - 1; w_idx++) {
                window[k][w_idx] = window[k][w_idx + 1];
            }
        }

        // Update Line Buffer Logic
        int val[MAX_KERNEL_SIZE];
#pragma HLS ARRAY_PARTITION variable=val complete dim=1

        val[0] = pixel;
        for (int k = 0; k < mask_size - 1; k++) {
            val[k + 1] = line_buffer[k][col];
        }

        // Update line buffer
        for (int k = 0; k < mask_size - 1; k++) {
            line_buffer[k][col] = val[k];
        }

        // Insert new column into window
        for (int r_w = 0; r_w < mask_size; r_w++) {
            window[r_w][mask_size - 1] = val[mask_size - 1 - r_w];
        }

        // Compute Result
        int center_r = row - z;
        int center_c = col - z;

        if (center_r >= 0 && center_c >= 0) {
            long long sum = 0;

            for (int kr = 0; kr < mask_size; kr++) {
                for (int kc = 0; kc < mask_size; kc++) {
                    int cur_pix_r = center_r + kr - z;
                    int cur_pix_c = center_c + kc - z;

                    if (cur_pix_r >= 0 && cur_pix_r < height &&
                        cur_pix_c >= 0 && cur_pix_c < width) {
                        sum += (long long)window[kr][kc] * (long long)mask_local[kr][kc];
                    }
                }
            }

            // Saturate exactly like CPU
            int final_val = static_cast<int>(
                (sum > CONV_2D_MAX) ? CONV_2D_MAX :
                ((sum < CONV_2D_MIN) ? CONV_2D_MIN : sum)
            );

            output[center_r * width + center_c] = final_val;
        }

        // Update row/col counters
        col++;
        if (col == width) {
            col = 0;
            row++;
        }
    }
}
}