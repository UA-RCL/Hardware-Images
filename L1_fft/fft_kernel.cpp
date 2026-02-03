#include <hls_stream.h>
#include <ap_fixed.h>
#include <complex>

// 1. Include Vitis FFT Library
#include "vt_fft.hpp"

using namespace xf::dsp::fft;

// 2. Define FFT Parameters
struct params_float : ssr_fft_default_params {
    static const int N = 1024;
    static const int R = 4;
    static const fft_output_order_enum output_data_order = SSR_FFT_NATURAL;
    static const transform_direction_enum transform_direction = FORWARD_TRANSFORM;
    typedef float T_inner;
};

// 3. INTERNAL Type Definition 
typedef complex_wrapper<float> T_inner_complex;

extern "C" {
    void fft_kernel(
        std::complex<float> in[params_float::N],
        std::complex<float> out[params_float::N]
    ) {
        #pragma HLS INTERFACE m_axi port=in bundle=gmem0 depth=1024
        #pragma HLS INTERFACE m_axi port=out bundle=gmem1 depth=1024
        #pragma HLS INTERFACE s_axilite port=return

        // 4. Stream Declarations using the Wrapper
        hls::stream<T_inner_complex> fft_in_stream[params_float::R];
        hls::stream<T_inner_complex> fft_out_stream[params_float::R];

        #pragma HLS STREAM variable=fft_in_stream depth=16
        #pragma HLS STREAM variable=fft_out_stream depth=16
        #pragma HLS ARRAY_PARTITION variable=fft_in_stream complete dim=1
        #pragma HLS ARRAY_PARTITION variable=fft_out_stream complete dim=1

        #pragma HLS DATAFLOW

        // 5. Data Mover (Memory -> Stream)
        mm2s_loop: for (int i = 0; i < params_float::N / params_float::R; i++) {
            #pragma HLS PIPELINE II=1
            for (int r = 0; r < params_float::R; r++) {
                std::complex<float> val = in[i * params_float::R + r];
                // Using brace initialization matches most wrapper implementations
                T_inner_complex wrapped_val = { val.real(), val.imag() };
                fft_in_stream[r].write(wrapped_val);
            }
        }

        // 6. FFT Call (Using the wrapper type streams)
        xf::dsp::fft::fft<params_float>(fft_in_stream, fft_out_stream);

        // 7. Data Mover (Stream -> Memory)
        s2mm_loop: for (int i = 0; i < params_float::N / params_float::R; i++) {
            #pragma HLS PIPELINE II=1
            for (int r = 0; r < params_float::R; r++) {
                T_inner_complex wrapped_val = fft_out_stream[r].read();
                out[i * params_float::R + r] = std::complex<float>(wrapped_val.real(), wrapped_val.imag());
            }
        }
    }
}