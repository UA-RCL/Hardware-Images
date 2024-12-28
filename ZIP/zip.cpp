#include <hls_stream.h>
#include <ap_int.h>

// #define DATA_WIDTH 32

// typedef ap_int<DATA_WIDTH> data_t;

typedef float data_t;

enum Operation { ADD = 0, SUB = 1, MULT = 2, DIV = 3 };

void zip(
    const data_t* input1,
    const data_t* input2,
    data_t* output,
    Operation op,
    int size
) {
    #pragma HLS INTERFACE m_axi port=input1 offset=slave bundle=hbm0
    #pragma HLS INTERFACE m_axi port=input2 offset=slave bundle=hbm1
    #pragma HLS INTERFACE m_axi port=output offset=slave bundle=hbm2

    #pragma HLS INTERFACE s_axilite port=input1 bundle=control
    #pragma HLS INTERFACE s_axilite port=input2 bundle=control
    #pragma HLS INTERFACE s_axilite port=output bundle=control
    #pragma HLS INTERFACE s_axilite port=op bundle=control
    #pragma HLS INTERFACE s_axilite port=size bundle=control
    #pragma HLS INTERFACE s_axilite port=return bundle=control

    data_t a, b, result;

    for (int i = 0; i < size; i++) {
        #pragma HLS PIPELINE II=1
        a = input1[i];
        b = input2[i];

        switch (op) {
            case ADD:
                result = a + b;
                break;
            case SUB:
                result = a - b;
                break;
            case MULT:
                result = a * b;
                break;
            case DIV:
                result = (b != 0) ? (a / b) : 0;
                break;
            default:
                result = 0; // Default case for safety
        }

        output[i] = result;
    }
}
