#include "hls_stream.h"
#include "ap_int.h"

// Optimized GEMM kernel
void gemm_kernel(float* A, float* B, float* C, int M, int K, int N) {
    // Local buffers for block-based computation
    float localA[128][128];
    float localB[128][128];
    float localC[128][128] = {0}; // Initialize with zeros

    #pragma HLS ARRAY_PARTITION variable=localA complete dim=2
    #pragma HLS ARRAY_PARTITION variable=localB complete dim=1
    #pragma HLS ARRAY_PARTITION variable=localC complete dim=2

    // Process blocks of A, B, and C
    for (int i = 0; i < M; i += 128) {
        for (int j = 0; j < N; j += 128) {
            for (int k = 0; k < K; k += 128) {
                // Load block of A and B into local buffers
                for (int ii = 0; ii < 128; ii++) {
                    for (int kk = 0; kk < 128; kk++) {
                        #pragma HLS PIPELINE II=1
                        localA[ii][kk] = A[(i + ii) * K + (k + kk)];
                    }
                }
                for (int kk = 0; kk < 128; kk++) {
                    for (int jj = 0; jj < 128; jj++) {
                        #pragma HLS PIPELINE II=1
                        localB[kk][jj] = B[(k + kk) * N + (j + jj)];
                    }
                }

                // Compute localC = localA * localB
                for (int ii = 0; ii < 128; ii++) {
                    for (int jj = 0; jj < 128; jj++) {
                        #pragma HLS PIPELINE II=1
                        for (int kk = 0; kk < 128; kk++) {
                            #pragma HLS UNROLL factor=4
                            localC[ii][jj] += localA[ii][kk] * localB[kk][jj];
                        }
                    }
                }
            }

            // Write back localC to global memory
            for (int ii = 0; ii < 128; ii++) {
                for (int jj = 0; jj < 128; jj++) {
                    #pragma HLS PIPELINE II=1
                    C[(i + ii) * N + (j + jj)] = localC[ii][jj];
                }
            }
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
