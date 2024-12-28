#include "hls_stream.h"
#include "ap_int.h"

void gemm_kernel(float* A, float* B, float* C, int M, int K, int N) {
    // Outer loop for rows of A and columns of B
    for (int i = 0; i < M; i++) {
        #pragma HLS PIPELINE II=1
        for (int j = 0; j < N; j++) {
            float sum = 0;
            // Inner loop for matrix multiplication (A * B)
            for (int k = 0; k < K; k++) {
                #pragma HLS UNROLL factor=4
                sum += A[i * K + k] * B[k * N + j];
            }
            C[i * N + j] = sum;
        }
    }
}

extern "C" {
    void GEMM(float* A, float* B, float* C, int M, int K, int N) {
        #pragma HLS INTERFACE m_axi port=A offset=slave bundle=gmem
        #pragma HLS INTERFACE m_axi port=B offset=slave bundle=gmem
        #pragma HLS INTERFACE m_axi port=C offset=slave bundle=gmem

        #pragma HLS INTERFACE s_axilite port=A bundle=control
        #pragma HLS INTERFACE s_axilite port=B bundle=control
        #pragma HLS INTERFACE s_axilite port=C bundle=control

        #pragma HLS INTERFACE s_axilite port=M bundle=control
        #pragma HLS INTERFACE s_axilite port=K bundle=control
        #pragma HLS INTERFACE s_axilite port=N bundle=control
        #pragma HLS INTERFACE s_axilite port=return bundle=control
        gemm_kernel(A, B, C, M, K, N);
    }
}