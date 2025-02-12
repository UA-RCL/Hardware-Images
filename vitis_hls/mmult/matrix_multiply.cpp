#include "matrix_multiply.h"

/**
* Copyright (C) 2019-2021 Xilinx, Inc
*
* Licensed under the Apache License, Version 2.0 (the "License"). You may
* not use this file except in compliance with the License. A copy of the
* License is located at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
* WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
* License for the specific language governing permissions and limitations
* under the License.
*/

/*******************************************************************************

Vitis Key Concept :

    This is a matrix multiplication based example which showcases the how
    reordering the loops helps achieve better pipeline initiation interval(II)
    and better performance.

*******************************************************************************/

// void matrix_multiply_top(MATRIX_T in1r[A_ROWS][A_COLS],
//                          MATRIX_T in1i[A_ROWS][A_COLS],
//                          MATRIX_T in2r[B_ROWS][B_COLS],
//                          MATRIX_T in2i[B_ROWS][B_COLS],
//                          MATRIX_T outr[C_ROWS][C_COLS],
//                          MATRIX_T outi[C_ROWS][C_COLS]) {
void matrix_multiply_top(MATRIX_T Ar[A_ROWS][A_COLS],
                         MATRIX_T Ai[A_ROWS][A_COLS],
                         MATRIX_T Br[B_ROWS][B_COLS],
                         MATRIX_T Bi[B_ROWS][B_COLS],
                         MATRIX_T Cr[C_ROWS][C_COLS],
                         MATRIX_T Ci[C_ROWS][C_COLS]) {
    // Local memory to store input and output matrices
    // Local memory is implemented as BRAM memory blocks
    // MATRIX_T Ar[A_ROWS][A_COLS];
    // MATRIX_T Ai[A_ROWS][A_COLS];
    // MATRIX_T Br[B_ROWS][B_COLS];
    // MATRIX_T Bi[B_ROWS][B_COLS];
    // MATRIX_T Cr[C_ROWS][C_COLS];
    // MATRIX_T Ci[C_ROWS][C_COLS];
    MATRIX_T temp_sumr[C_COLS];
    MATRIX_T temp_sumi[C_COLS];
// #pragma HLS ARRAY_PARTITION variable = Br dim = 2 complete
// #pragma HLS ARRAY_PARTITION variable = Bi dim = 2 complete
// #pragma HLS ARRAY_PARTITION variable = Cr dim = 2 complete
// #pragma HLS ARRAY_PARTITION variable = Ci dim = 2 complete
// #pragma HLS ARRAY_PARTITION variable = temp_sumr dim = 1 complete
// #pragma HLS ARRAY_PARTITION variable = temp_sumi dim = 1 complete

// Burst reads on input matrices from global memory
// Burst read for matrix A
// Auto-pipeline is going to apply pipeline to this loops
// readA:
//     // for (int itr = 0, i = 0, j = 0; itr < size * size; itr++, j++) {
//     for (int i = 0, j = 0; i < A_ROWS; j++) {
// #pragma HLS LOOP_TRIPCOUNT min = A_ROWS * A_COLS max = A_ROWS * A_COLS
//         if (j == A_COLS) {
//             j = 0;
//             i++;
//         }
//         Ar[i][j] = in1r[i][j];
//         Ai[i][j] = in1i[i][j];
//     }
// 
// // Burst read for matrix B
// readB:
//     // for (int itr = 0, i = 0, j = 0; itr < size * size; itr++, j++) {
//     for (int i = 0, j = 0; i < B_ROWS; j++) {
// #pragma HLS LOOP_TRIPCOUNT min = B_ROWS * B_COLS max = B_ROWS * B_COLS
//         if (j == B_COLS) {
//             j = 0;
//             i++;
//         }
//         Br[i][j] = in2r[i][j];
//         Bi[i][j] = in2i[i][j];
//     }

// Performs matrix multiply over matrices A and B and stores the result
// in C. All the matrices are square matrices of the form (size x size)

// Pipeline attribute is specified for the innermost loop (lreorder3)
// and the order of the loops lreorder2 and lreorder 3 are changed here.

// When the iteration variables j and k are interchanged between the loops,
// lreoder2 and lreorder3, the pipeline initiation interval (II) improves
// and becomes 1 (ideal).

// Also the reordering avoids creating an adder tree for calculating the
// sum(output) of a single output element

// lreorder1: for (int i = 0; i < size; i++) {
//     lreorder2: for (int j = 0; j < size; j++) {
//         lreorder3: for (int k = 0; k < MAX_SIZE; k++) {
//             int result = (k == 0) ? 0 : temp_sum[j];
//             result += A[i][k] * B[k][j];
//             temp_sum[j] = result;
//             if (k== size -1) C[i][j] = result;
//         }
//     }
// }

// The above code snippet of the Matrix Multiply kernel in which the loops
// lreorder2 and lreorder3 are not interchanged, gives a pipeline initiation
// interval (II) of 64

// Calculate matrix multiplication using local data buffer based on input size
// and write results into local buffer for C
lreorder1:
    for (int i = 0; i < A_ROWS; i++) {
// #pragma HLS LOOP_TRIPCOUNT min = c_size max = c_size
#pragma HLS LOOP_TRIPCOUNT min = C_ROWS max = C_ROWS
    lreorder2:
        for (int k = 0; k < B_ROWS; k++) {
#pragma HLS LOOP_TRIPCOUNT min = B_ROWS max = B_ROWS
        lreorder3:
            for (int j = 0; j < B_COLS; j++) {
                MATRIX_T resultr = (k == 0) ? 0 : temp_sumr[j];
                MATRIX_T resulti = (k == 0) ? 0 : temp_sumi[j];
                resultr += (Ar[i][k] * Br[k][j]) - (Ai[i][k] * Bi[k][j]);
                resulti += (Ar[i][k] * Bi[k][j]) + (Ai[i][k] * Br[k][j]);
                temp_sumr[j] = resultr;
                temp_sumi[j] = resulti;
                if (k == B_ROWS - 1) {
                    Cr[i][j] = resultr;
                    Ci[i][j] = resulti;
                }
            }
        }
    }

// Burst write from output matrices to global memory
// Burst write from matrix C
// writeC:
//     // for (int itr = 0, i = 0, j = 0; itr < size * size; itr++, j++) {
//     for (int i = 0, j = 0; i < C_ROWS; j++) {
// #pragma HLS LOOP_TRIPCOUNT min = C_ROWS * C_COLS max = C_ROWS * C_COLS
//         if (j == C_COLS) {
//             j = 0;
//             i++;
//         }
//         outr[i][j] = Cr[i][j];
//         outi[i][j] = Ci[i][j];
//     }
}
