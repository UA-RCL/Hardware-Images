#include "common.h"
#include "ap_axi_sdata.h"
#include "hls_stream.h"

#include "math.h"

void CONV_2D_ref(pix_t **input, const unsigned int *height, const unsigned int *width, float **mask, const unsigned int *mask_size, unsigned int **output_addr) {
  int i, j, k, l;
  int s, w;
  int z;

  // float sum;
  int sum;
  unsigned int *output = *output_addr;

  z = (*mask_size) / 2;

  for (i = 0; i < (*height); i++) {
    for (j = 0; j < (*width); j++) {
      // sum = 0.0;
      sum = 0;
      for (k = 0; k < (*mask_size); k++) {
        for (l = 0; l < (*mask_size); l++) {
          s = i + k - z;
          w = j + l - z;
          if ((s >= 0 && s < (*height)) && (w >= 0 && w < (*width))) {
            sum += (*input)[(*width) * s + w] * (*mask)[(*mask_size) * k + l];
          }
        }
      }
      // if ((int)round(sum) > CONV_2D_MAX) {
      //   output[(i * (*width)) + j] = CONV_2D_MAX;
      // } else if ((int)round(sum) < CONV_2D_MIN) {
      //   output[(i * (*width)) + j] = CONV_2D_MIN;
      // } else {
      //   output[(i * (*width)) + j] = (int)round(sum);
      // }
      if (sum > CONV_2D_MAX) {
        output[(i * (*width)) + j] = CONV_2D_MAX;
      } else if (sum < CONV_2D_MIN) {
        output[(i * (*width)) + j] = CONV_2D_MIN;
      } else {
        output[(i * (*width)) + j] = sum;
      }
      // printf("sum: %f, output: %d.\n", sum, (((int)round(sum) > CONV_2D_MAX) ? CONV_2D_MAX : (((int)round(sum) < CONV_2D_MIN) ? CONV_2D_MIN : (int)round(sum))));
      // output[(i * (*width)) + j] = ((round(sum) > CONV_2D_MAX) ? CONV_2D_MAX : ((round(sum) < CONV_2D_MIN) ? CONV_2D_MIN : round(sum)));
      // printf("IDK sum: %f, output: %d.\n", sum, output[(i * (*width)) + j]);
    }
  }
}

int main (void) {
  const float factor = 1;
  const short bias = 0;
  const short stride = 64;
  const unsigned int height = 25;
  const unsigned int width = 25;
  const unsigned int mask_size = 11;

  bool failed = false;
  unsigned int fail_cntr = 0;
  unsigned int zero_cntr = 0;
  unsigned int max_cntr = 0;
  unsigned int cntr = 0;
  unsigned int max_diff = 0;

  hls::stream<pix_t,2>   input_stream;
  hls::stream<pix_out_t> output_stream;

  // Allocate memory
  printf("[conv_2d] Allocating memory.\n");
  pix_t *input = (pix_t*)malloc(height * width * sizeof(pix_t));
  unsigned int *output_acc = (unsigned int*)malloc(height * width * sizeof(unsigned int));
  unsigned int *output_ref = (unsigned int*)malloc(height * width * sizeof(unsigned int));
  float *mask = (float*)malloc(mask_size * mask_size * sizeof(float));

  // Fill with test data
  printf("[conv_2d] Filling with test data.\n");
  for (unsigned int i = 0; i < height; i++) {
    for (unsigned int j = 0; j < width; j++) {
      if ((i >= 8) && (i <= 16) && (j >= 8) && (j <= 16)) {
        input[i * width + j] = 5;
        // input_stream.write(5);
      } else {
        input[i * width + j] = 1;
        // input_stream.write(5);
      }
    }
  }
  for (unsigned int i = 0; i < mask_size; i++) {
    for (unsigned int j = 0; j < mask_size; j++) {
      if ((j == 4) && (i >= 4) && (i <= 6))
        mask[i * mask_size + j] = 1;
      else if ((j == 6) && (i >= 4) && (i <= 6))
        mask[i * mask_size + j] = 1;
      else
        mask[i * mask_size + j] = 1;
    }
  }

  // Do accelerator convolution
  printf("[conv_2d] Calling convolution accelerator.\n");
  // void Filter2DKernel(
  //         const float              coeffs[MAX_COEFFS],
  //         float                    factor,
  //         short                    bias,
  //         unsigned short           width,
  //         unsigned short           height,
  //         unsigned short           stride,
  //         hls::stream<pix_t,2>    &input_stream,
  //         hls::stream<pix_out_t>  &output_stream);
  for (unsigned int i = 0; i < height * width; i++) {
    if (input_stream.full()) {
      printf("ERROR: input_stream is full!");
      exit(1);
    }
    input_stream.write(input[i]);
  }
  Filter2DKernel(mask, factor, bias, width, height, stride, input_stream, output_stream);
  for (unsigned int i = 0; i < height * width; i++) {
    if (output_stream.empty()) {
      printf("ERROR: output_stream is empty!");
      exit(1);
    }
    output_acc[i] = output_stream.read().data;
  }

  // Do CPU convolution
  printf("[conv_2d] Calling convolution reference.\n");
  CONV_2D_ref(&input, &height, &width, &mask, &mask_size, &output_ref);

  // Check outputs
  for (unsigned int i = 0; i < height * width; i++) {
    printf("[conv_2d] index: %d, accelerator: %d, reference: %d\n", i, output_acc[i], output_ref[i]);
    if (output_acc[i] != output_ref[i]) {
      failed = true;
      fail_cntr++;
      if (abs(output_acc[i] - output_ref[i]) > max_diff)
        max_diff = abs(output_acc[i] - output_ref[i]);
    } else if (output_ref[i] == 0) {
      zero_cntr++;
    } else if (output_ref[i] == CONV_2D_MAX) {
      max_cntr++;
    }
    cntr++;
  }

  if (failed)
    printf("[conv_2d] Standalone test failed with %d wrong out of %d, %d zeros, %d maxes, and a max difference of %d!\n", fail_cntr, cntr, zero_cntr, max_cntr, max_diff);
  else 
    printf("[conv_2d] Standalone test passed!\n");

  free(input);
  free(output_acc);
  free(output_ref);
  free(mask);
}
