#include "zip.h"

/*******************************************************************************

Vitis Key Concept :

    This is a matrix multiplication based example which showcases the how
    reordering the loops helps achieve better pipeline initiation interval(II)
    and better performance.

*******************************************************************************/

void zip_add_top(ZIP_T A[MAX_SIZE],
        ZIP_T B[MAX_SIZE],
        ZIP_T C[MAX_SIZE],
        int size) {
    // Local memory to store input and output matrices
    // Calculate matrix multiplication using local data buffer based on input size
    // and write results into local buffer for C
zip_add_l:
    for (int i = 0; i < 2 * size; i++) {
#pragma HLS LOOP_TRIPCOUNT min = 2 max = MAX_SIZE
        C[i] = A[i] + B[i];
    }
}

void zip_sub_top(ZIP_T A[MAX_SIZE],
        ZIP_T B[MAX_SIZE],
        ZIP_T C[MAX_SIZE],
        int size) {
    // Local memory to store input and output matrices
    // Calculate matrix multiplication using local data buffer based on input size
    // and write results into local buffer for C
zip_sub_l:
    for (int i = 0; i < 2 * size; i++) {
#pragma HLS LOOP_TRIPCOUNT min = 2 max = MAX_SIZE
        C[i] = A[i] - B[i];
    }
}

void zip_div_top(ZIP_T A[MAX_SIZE],
        ZIP_T B[MAX_SIZE],
        ZIP_T C[MAX_SIZE],
        int size) {
    // Local memory to store input and output matrices
    // Calculate matrix multiplication using local data buffer based on input size
    // and write results into local buffer for C
zip_div_l:
    for (int i = 0; i < 2 * size; i += 2) {
#pragma HLS LOOP_TRIPCOUNT min = 2 max = MAX_SIZE
        C[i    ] = ( A[i] * B[i    ] + A[i + 1] * B[i + 1]) / (B[i] * B[i] + B[i + 1] * B[i + 1]);
        C[i + 1] = (-A[i] * B[i + 1] + A[i + 1] * B[i    ]) / (B[i] * B[i] + B[i + 1] * B[i + 1]);
    }
}

void zip_mult_top(ZIP_T A[MAX_SIZE],
        ZIP_T B[MAX_SIZE],
        ZIP_T C[MAX_SIZE],
        int size) {
    // Local memory to store input and output matrices
    // Calculate matrix multiplication using local data buffer based on input size
    // and write results into local buffer for C
zip_mult_l:
    for (int i = 0; i < 2 * size; i=i+2) {
#pragma HLS LOOP_TRIPCOUNT min = 2 max = MAX_SIZE
        C[i] = A[i] * B[i] - A[i+1] * B[i+1];
        C[i+1] = A[i] * B[i+1] + A[i+1] * B[i];
    }
}

