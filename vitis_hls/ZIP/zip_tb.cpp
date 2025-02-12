/*****************************************************************************
 *
 *     Author: Xilinx, Inc.
 *
 *     This text contains proprietary, confidential information of
 *     Xilinx, Inc. , is distributed by under license from Xilinx,
 *     Inc., and may be used, copied and/or disclosed only pursuant to
 *     the terms of a valid license agreement with Xilinx, Inc.
 *
 *     XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS"
 *     AS A COURTESY TO YOU, SOLELY FOR USE IN DEVELOPING PROGRAMS AND
 *     SOLUTIONS FOR XILINX DEVICES.  BY PROVIDING THIS DESIGN, CODE,
 *     OR INFORMATION AS ONE POSSIBLE IMPLEMENTATION OF THIS FEATURE,
 *     APPLICATION OR STANDARD, XILINX IS MAKING NO REPRESENTATION
 *     THAT THIS IMPLEMENTATION IS FREE FROM ANY CLAIMS OF INFRINGEMENT,
 *     AND YOU ARE RESPONSIBLE FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE
 *     FOR YOUR IMPLEMENTATION.  XILINX EXPRESSLY DISCLAIMS ANY
 *     WARRANTY WHATSOEVER WITH RESPECT TO THE ADEQUACY OF THE
 *     IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO ANY WARRANTIES OR
 *     REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE FROM CLAIMS OF
 *     INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *     FOR A PARTICULAR PURPOSE.
 *
 *     Xilinx products are not intended for use in life support appliances,
 *     devices, or systems. Use in such applications is expressly prohibited.
 *
 *     (c) Copyright 2013-2014 Xilinx Inc.
 *     All rights reserved.
 *
 *****************************************************************************/

#include "zip.h"
#include <stdio.h>
// #include "hls/linear_algebra/utils/x_hls_matrix_utils.h"
// #include "hls/linear_algebra/utils/x_hls_matrix_tb_utils.h"

int main (void){

    ZIP_T A[MAX_SIZE];
    ZIP_T B[MAX_SIZE];
    ZIP_T C[MAX_SIZE];
    int size;
    int op;

    size = 10;
    for (int i = 0; i < size; i++) {
        A[i] = (i+1) * 1.0;
    }

    for (int i = 0; i < size; i++) {
        B[i] = (i+1) * 1.0;
    }

    op = 0; // ADD
    zip_add_top(A, B, C, size);

    printf("C:");
    for (int i = 0; i < size; i++) {
        printf("%f  ", C[i]);
    }
    printf("\n");

    op = 1; // SUB
    zip_sub_top(A, B, C, size);

    printf("C:");
    for (int i = 0; i < size; i++) {
        printf("%f  ", C[i]);
    }
    printf("\n");

    op = 2; // MULT
    zip_mult_top(A, B, C, size);

    printf("C:");
    for (int i = 0; i < size; i++) {
        printf("%f  ", C[i]);
    }
    printf("\n");

    op = 3; // DIV
    zip_div_top(A, B, C, size);

    printf("C:");
    for (int i = 0; i < size; i++) {
        printf("%f  ", C[i]);
    }
    printf("\n");

    op = 4; // COMP_MULT
    zip_comp_mult_top(A, B, C, size);

    printf("C:");
    for (int i = 0; i < size; i++) {
        printf("%f  ", C[i]);
    }
    printf("\n");
    return(0);
}

