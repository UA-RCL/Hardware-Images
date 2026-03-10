/*
 * Testbench for conv2d HLS kernel
 * Tests:
 *   1. Identity kernel (3x3), no padding, stride 1
 *   2. Uniform averaging kernel (3x3), with padding=1, stride 1
 *   3. Non-square-friendly: 5x5 kernel, padding=2, stride 2
 *   4. Single pixel input — edge case
 */

#include <iostream>
#include <cmath>
#include "hls_stream.h"
#include "ap_axi_sdata.h"

typedef float data_t;
typedef ap_axis<sizeof(data_t)*8, 0, 0, 0> data_axis_t;

// Declaration matching the original kernel:
// input_stream and kernel_stream are plain hls::stream<data_t>
// output_stream is hls::stream<data_axis_t>
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
    hls::stream<data_axis_t> &output_stream);
}

// ---------------------------------------------------------------
// Software reference convolution
// ---------------------------------------------------------------
void conv2d_sw(
    const data_t *input,
    const data_t *kernel,
    data_t        bias,
    int           stride,
    int           padding,
    int           input_h,
    int           input_w,
    int           kernel_h,
    int           kernel_w,
    data_t       *output)
{
    int output_h = (input_h + 2 * padding - kernel_h) / stride + 1;
    int output_w = (input_w + 2 * padding - kernel_w) / stride + 1;

    for (int i = 0; i < output_h; i++) {
        for (int j = 0; j < output_w; j++) {
            data_t tmp = 0;
            for (int k = 0; k < kernel_h; k++) {
                for (int l = 0; l < kernel_w; l++) {
                    int in_r = i * stride + k - padding;
                    int in_c = j * stride + l - padding;
                    data_t in_val = 0;
                    if (in_r >= 0 && in_r < input_h && in_c >= 0 && in_c < input_w)
                        in_val = input[in_r * input_w + in_c];
                    tmp += in_val * kernel[k * kernel_w + l];
                }
            }
            output[i * output_w + j] = tmp + bias;
        }
    }
}

// ---------------------------------------------------------------
// Helpers: input/kernel use plain stream<data_t>
//          output uses stream<data_axis_t>
// ---------------------------------------------------------------
void fill_input_stream(hls::stream<data_t> &s, const data_t *data, int n) {
    for (int i = 0; i < n; i++)
        s.write(data[i]);
}

void drain_output_stream(hls::stream<data_axis_t> &s, data_t *data, int n) {
    for (int i = 0; i < n; i++) {
        data_axis_t pkt = s.read();
        union { unsigned int u; float f; } cv;
        cv.u = (unsigned int)pkt.data;
        data[i] = cv.f;
    }
}

// ---------------------------------------------------------------
// Compare outputs with tolerance
// ---------------------------------------------------------------
bool compare(const data_t *hw, const data_t *sw, int n, const char *test_name, float tol = 1e-4f) {
    int errors = 0;
    for (int i = 0; i < n; i++) {
        float diff = fabs((float)hw[i] - (float)sw[i]);
        if (diff > tol) {
            if (errors < 5)
                std::cout << "  [MISMATCH] idx=" << i
                          << " hw=" << hw[i] << " sw=" << sw[i]
                          << " diff=" << diff << "\n";
            errors++;
        }
    }
    if (errors == 0) {
        std::cout << "  [PASS] " << test_name << "\n";
        return true;
    } else {
        std::cout << "  [FAIL] " << test_name << " — " << errors << "/" << n << " mismatches\n";
        return false;
    }
}

// ---------------------------------------------------------------
// Test 1: 3x3 identity kernel, 5x5 input, no padding, stride 1
// ---------------------------------------------------------------
bool test_identity() {
    std::cout << "\n[Test 1] 3x3 identity kernel, 5x5 input, padding=0, stride=1\n";

    const int IH = 5, IW = 5, KH = 3, KW = 3;
    const int stride = 1, padding = 0;
    const data_t bias = 0.0f;
    const int OH = (IH + 2*padding - KH) / stride + 1;
    const int OW = (IW + 2*padding - KW) / stride + 1;

    data_t input[IH * IW];
    for (int i = 0; i < IH * IW; i++) input[i] = (data_t)(i + 1);

    data_t kernel[KH * KW] = {
        0, 0, 0,
        0, 1, 0,
        0, 0, 0
    };

    data_t sw_out[OH * OW];
    conv2d_sw(input, kernel, bias, stride, padding, IH, IW, KH, KW, sw_out);

    hls::stream<data_t>      in_s, k_s;
    hls::stream<data_axis_t> out_s;
    fill_input_stream(in_s, input,  IH * IW);
    fill_input_stream(k_s,  kernel, KH * KW);
    conv2d(in_s, k_s, bias, stride, padding, IH, IW, KH, KW, out_s);

    data_t hw_out[OH * OW];
    drain_output_stream(out_s, hw_out, OH * OW);

    return compare(hw_out, sw_out, OH * OW, "Identity 3x3");
}

// ---------------------------------------------------------------
// Test 2: 3x3 box kernel, 6x6 input, padding=1, stride=1
// ---------------------------------------------------------------
bool test_box_blur() {
    std::cout << "\n[Test 2] 3x3 box kernel, 6x6 input, padding=1, stride=1\n";

    const int IH = 6, IW = 6, KH = 3, KW = 3;
    const int stride = 1, padding = 1;
    const data_t bias = 0.5f;
    const int OH = (IH + 2*padding - KH) / stride + 1;
    const int OW = (IW + 2*padding - KW) / stride + 1;

    data_t input[IH * IW];
    for (int i = 0; i < IH * IW; i++) input[i] = (data_t)(i % 7 + 1);

    data_t kernel[KH * KW];
    for (int i = 0; i < KH * KW; i++) kernel[i] = 1.0f / 9.0f;

    data_t sw_out[OH * OW];
    conv2d_sw(input, kernel, bias, stride, padding, IH, IW, KH, KW, sw_out);

    hls::stream<data_t>      in_s, k_s;
    hls::stream<data_axis_t> out_s;
    fill_input_stream(in_s, input,  IH * IW);
    fill_input_stream(k_s,  kernel, KH * KW);
    conv2d(in_s, k_s, bias, stride, padding, IH, IW, KH, KW, out_s);

    data_t hw_out[OH * OW];
    drain_output_stream(out_s, hw_out, OH * OW);

    return compare(hw_out, sw_out, OH * OW, "Box blur 3x3 with padding+bias");
}

// ---------------------------------------------------------------
// Test 3: 5x5 kernel, 10x10 input, padding=2, stride=2
// ---------------------------------------------------------------
bool test_5x5_stride2() {
    std::cout << "\n[Test 3] 5x5 kernel, 10x10 input, padding=2, stride=2\n";

    const int IH = 10, IW = 10, KH = 5, KW = 5;
    const int stride = 2, padding = 2;
    const data_t bias = -1.0f;
    const int OH = (IH + 2*padding - KH) / stride + 1;
    const int OW = (IW + 2*padding - KW) / stride + 1;

    data_t input[IH * IW];
    for (int i = 0; i < IH * IW; i++) input[i] = (data_t)(i * 0.1f);

    data_t kernel[KH * KW] = {
         1,  4,  6,  4, 1,
         4, 16, 24, 16, 4,
         6, 24, 36, 24, 6,
         4, 16, 24, 16, 4,
         1,  4,  6,  4, 1
    };
    for (int i = 0; i < KH * KW; i++) kernel[i] /= 256.0f;

    data_t sw_out[OH * OW];
    conv2d_sw(input, kernel, bias, stride, padding, IH, IW, KH, KW, sw_out);

    hls::stream<data_t>      in_s, k_s;
    hls::stream<data_axis_t> out_s;
    fill_input_stream(in_s, input,  IH * IW);
    fill_input_stream(k_s,  kernel, KH * KW);
    conv2d(in_s, k_s, bias, stride, padding, IH, IW, KH, KW, out_s);

    data_t hw_out[OH * OW];
    drain_output_stream(out_s, hw_out, OH * OW);

    return compare(hw_out, sw_out, OH * OW, "5x5 Gaussian stride=2");
}

// ---------------------------------------------------------------
// Test 4: Single pixel input — edge case
// ---------------------------------------------------------------
bool test_single_pixel() {
    std::cout << "\n[Test 4] 1x1 input, 1x1 kernel, padding=0, stride=1\n";

    const int IH = 1, IW = 1, KH = 1, KW = 1;
    const int stride = 1, padding = 0;
    const data_t bias = 3.0f;

    data_t input[1]  = { 7.0f };
    data_t kernel[1] = { 2.0f };

    data_t sw_out[1];
    conv2d_sw(input, kernel, bias, stride, padding, IH, IW, KH, KW, sw_out);

    hls::stream<data_t>      in_s, k_s;
    hls::stream<data_axis_t> out_s;
    fill_input_stream(in_s, input,  1);
    fill_input_stream(k_s,  kernel, 1);
    conv2d(in_s, k_s, bias, stride, padding, IH, IW, KH, KW, out_s);

    data_t hw_out[1];
    drain_output_stream(out_s, hw_out, 1);

    std::cout << "  Expected: " << sw_out[0] << "  Got: " << hw_out[0] << "\n";
    return compare(hw_out, sw_out, 1, "Single pixel");
}

// ---------------------------------------------------------------
// main
// ---------------------------------------------------------------
int main() {
    std::cout << "========================================\n";
    std::cout << "  conv2d HLS Testbench\n";
    std::cout << "========================================\n";

    int passed = 0, total = 4;

    if (test_identity())     passed++;
    if (test_box_blur())     passed++;
    if (test_5x5_stride2())  passed++;
    if (test_single_pixel()) passed++;

    std::cout << "\n========================================\n";
    std::cout << "  Results: " << passed << "/" << total << " tests passed\n";
    std::cout << "========================================\n";

    return (passed == total) ? 0 : 1;
}