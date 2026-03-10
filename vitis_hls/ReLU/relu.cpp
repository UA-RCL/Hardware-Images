#include "hls_stream.h"
#include "ap_axi_sdata.h"

typedef ap_axiu<32,0,0,0> axis_float_t;  // 32-bit float over AXIS

void ReLU(
        hls::stream<axis_float_t> &input_stream,
        hls::stream<axis_float_t> &output_stream,
        int size)
{
#pragma HLS INTERFACE axis port=input_stream
#pragma HLS INTERFACE axis port=output_stream
#pragma HLS INTERFACE s_axilite port=size bundle=control
#pragma HLS INTERFACE s_axilite port=return bundle=control

    for (int i = 0; i < size; i++) {
#pragma HLS PIPELINE II=1

        axis_float_t in_pkt  = input_stream.read();
        axis_float_t out_pkt;

        float in_val  = *((float*)(&in_pkt.data));
        float out_val = (in_val > 0.0f) ? in_val : 0.0f;

        out_pkt.data = *((ap_uint<32>*)(&out_val));
        out_pkt.keep = in_pkt.keep;
        out_pkt.last = (i == size-1);

        output_stream.write(out_pkt);
    }
}