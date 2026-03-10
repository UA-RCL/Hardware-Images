#include "hls_stream.h"
#include "ap_axi_sdata.h"
#include <hls_math.h>
#include <limits>

typedef ap_axiu<32,0,0,0> axis_t;
typedef float data_t;

void MaxPoolingKernel(
        hls::stream<axis_t> &input_stream,
        hls::stream<axis_t> &output_stream,
        int pool_size,
        int pool_stride,
        int input_h,
        int input_w)
{
#pragma HLS INTERFACE axis port=input_stream
#pragma HLS INTERFACE axis port=output_stream
#pragma HLS INTERFACE s_axilite port=pool_size   bundle=config
#pragma HLS INTERFACE s_axilite port=pool_stride bundle=config
#pragma HLS INTERFACE s_axilite port=input_h     bundle=config
#pragma HLS INTERFACE s_axilite port=input_w     bundle=config
#pragma HLS INTERFACE s_axilite port=return      bundle=config

    const int MAX_WIDTH = 256;
    const int MAX_POOL  = 8;

    data_t line_buffer[MAX_POOL][MAX_WIDTH];
#pragma HLS ARRAY_PARTITION variable=line_buffer complete dim=1

    int output_h = (input_h - pool_size) / pool_stride + 1;
    int output_w = (input_w - pool_size) / pool_stride + 1;

    // row_ptr tracks which line_buffer row to write into next
    int row_ptr = 0;

    for (int r = 0; r < input_h; r++) {
        for (int c = 0; c < input_w; c++) {
#pragma HLS PIPELINE II=1

            // 1. Read next pixel and store into the current line buffer row
            axis_t in_pkt = input_stream.read();
            data_t pixel  = *((float*)(&in_pkt.data));
            line_buffer[row_ptr][c] = pixel;

            // 2. Check whether this pixel completes a pooling window
            //    Window's bottom-right corner lands at (r, c)
            //    so we need r >= pool_size-1 and c >= pool_size-1
            //    and the stride alignment must hold for the OUTPUT indices
            int out_r = r - (pool_size - 1);
            int out_c = c - (pool_size - 1);

            if (out_r >= 0 && (out_r % pool_stride == 0) &&
                out_c >= 0 && (out_c % pool_stride == 0))
            {
                // 3. Walk the pool window.
                //    row offset i=0 → current row (row_ptr),
                //    row offset i=1 → one row earlier, etc.
                //    col offset j=0 → current col c,
                //    col offset j=1 → one col to the left, etc.
                data_t max_val = -std::numeric_limits<data_t>::infinity();

                for (int i = 0; i < pool_size; i++) {
#pragma HLS UNROLL
                    // Map i=0 to the row we JUST wrote (row_ptr),
                    // i=1 to the row before that, etc.
                    int buf_row = (row_ptr - i + MAX_POOL) % MAX_POOL;
                    for (int j = 0; j < pool_size; j++) {
#pragma HLS UNROLL
                        int buf_col = c - (pool_size - 1 - j);  // left-to-right order
                        data_t val = line_buffer[buf_row][buf_col];
                        if (val > max_val) max_val = val;
                    }
                }

                // 4. Emit output pixel
                axis_t out_pkt;
                float tmp = max_val;
                out_pkt.data  = *((ap_uint<32>*)(&tmp));
                out_pkt.keep  = -1;
                out_pkt.last  = 0;
                output_stream.write(out_pkt);
            }
        }

        // Advance the circular line buffer pointer after finishing each row
        row_ptr = (row_ptr + 1) % pool_size;
    }
}