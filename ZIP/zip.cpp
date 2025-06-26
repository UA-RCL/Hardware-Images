#include <hls_stream.h>
#include <ap_int.h>

#define BUFFER_SIZE 4096

// typedef ap_int<DATA_WIDTH> data_t;

typedef float data_t;

enum Operation { ADD = 0, SUB = 1, MULT = 2, DIV = 3 , MULT_RE = 4};

extern "C" {
void zip(
    const data_t* input1,
    const data_t* input2,
    data_t* output,
    Operation op,
    int size
) {
    #pragma HLS INTERFACE m_axi port=input1 offset=slave bundle=float_in max_read_burst_length=16 num_read_outstanding=64
    #pragma HLS INTERFACE m_axi port=input2 offset=slave bundle=float_in max_read_burst_length=16 num_read_outstanding=64
    #pragma HLS INTERFACE m_axi port=output offset=slave bundle=float_out max_write_burst_length=16 num_write_outstanding=64

    #pragma HLS INTERFACE s_axilite port=input1 bundle=control
    #pragma HLS INTERFACE s_axilite port=input2 bundle=control
    #pragma HLS INTERFACE s_axilite port=output bundle=control
    #pragma HLS INTERFACE s_axilite port=op bundle=control
    #pragma HLS INTERFACE s_axilite port=size bundle=control
    #pragma HLS INTERFACE s_axilite port=return bundle=control

    data_t input1_bufffer[BUFFER_SIZE];
    data_t input2_bufffer[BUFFER_SIZE];
    data_t output_bufffer[BUFFER_SIZE];
    int chunk_size;
    for (int i = 0; i < size; i+=BUFFER_SIZE) {
        if ((i + BUFFER_SIZE) > size){
            chunk_size = size - i;
        }
        read_input1: for (int j = 0; j < chunk_size; j++) {
            #pragma HLS PIPELINE II=1
            input1_bufffer[j] = input1[i+j];
        }
        read_input2: for (int j = 0; j < chunk_size; j++) {
            #pragma HLS PIPELINE II=1
            input2_bufffer[j] = input2[i+j];
        }
        switch (op) {
            case ADD:
                add: for (int j = 0; j < chunk_size; j++) {
                    #pragma HLS PIPELINE II=1
                    output_bufffer[j] = input1_bufffer[j] + input2_bufffer[j];
                }
                break;
            case SUB:
                sub: for (int j = 0; j < chunk_size; j++) {
                    #pragma HLS PIPELINE II=1
                    output_bufffer[j] = input1_bufffer[j] - input2_bufffer[j];
                }
                break;
            case MULT:
                mult: for (int j = 0; j < chunk_size; j+=2) {
                    #pragma HLS PIPELINE II=1
                    output_bufffer[j]   = input1_bufffer[j] * input2_bufffer[j]   - input1_bufffer[j+1] * input2_bufffer[j+1];
                    output_bufffer[j+1] = input1_bufffer[j] * input2_bufffer[j+1] + input1_bufffer[j+1] * input2_bufffer[j];
                }
                break;
            case DIV:
                div: for (int j = 0; j < chunk_size; j++) {
                    #pragma HLS PIPELINE II=1
                    output_bufffer[j] = (input2_bufffer[j] != 0) ? (input1_bufffer[j] / input2_bufffer[j]) : 0;
                }
                break;
            case MULT_RE:
                mult_re: for (int j = 0; j < chunk_size; j++) {
                    #pragma HLS PIPELINE II=1
                    output_bufffer[j] = input1_bufffer[j] * input2_bufffer[j];
                }
                break;
            default:
                for (int j = 0; j < chunk_size; j++) {
                    #pragma HLS PIPELINE II=1
                    output_bufffer[j] = 0;
                }
        }
        write_output: for (int j = 0; j < chunk_size; j++) {
            #pragma HLS PIPELINE II=1
            output[i + j] = output_bufffer[j];
        }
    }
}
}
