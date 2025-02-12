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

#include "matrix_multiply.h"
#include "hls/linear_algebra/utils/x_hls_matrix_utils.h"
#include "hls/linear_algebra/utils/x_hls_matrix_tb_utils.h"

int main (void){

  MATRIX_T A_r[A_ROWS][A_COLS];
  MATRIX_T B_r[B_ROWS][B_COLS];
  MATRIX_T A_i[A_ROWS][A_COLS];
  MATRIX_T B_i[B_ROWS][B_COLS];
  MATRIX_T C_r[C_ROWS][C_COLS];
  MATRIX_T C_i[C_ROWS][C_COLS];

  // A_r[0][0] = 10.0;  A_r[0][1] = 20.0;  A_r[0][2] = 30.0;
  // A_r[1][0] = 40.0;  A_r[1][1] = 50.0;  A_r[1][2] = 60.0;
  // A_r[2][0] = 70.0;  A_r[2][1] = 80.0;  A_r[2][2] = 90.0;

  // B_r[0][0] = 1.0;  B_r[0][1] = 2.0;  B_r[0][2] = 3.0;
  // B_r[1][0] = 4.0;  B_r[1][1] = 5.0;  B_r[1][2] = 6.0;
  // B_r[2][0] = 7.0;  B_r[2][1] = 8.0;  B_r[2][2] = 9.0;

  // A_i[0][0] = 0.0;  A_i[0][1] = 0.0;  A_i[0][2] = 0.0;
  // A_i[1][0] = 0.0;  A_i[1][1] = 0.0;  A_i[1][2] = 0.0;
  // A_i[2][0] = 0.0;  A_i[2][1] = 0.0;  A_i[2][2] = 0.0;

  // B_i[0][0] = 0.0;  B_i[0][1] = 0.0;  B_i[0][2] = 0.0;
  // B_i[1][0] = 0.0;  B_i[1][1] = 0.0;  B_i[1][2] = 0.0;
  // B_i[2][0] = 0.0;  B_i[2][1] = 0.0;  B_i[2][2] = 0.0;

  for (int i = 0; i < A_ROWS; i++) {
      for (int j = 0; j < A_COLS; j++) {
	      A_r[i][j] = 1.0;
	      A_i[i][j] = 0.0;
	  }
  }

  for (int i = 0; i < B_ROWS; i++) {
      for (int j = 0; j < B_COLS; j++) {
	      B_r[i][j] = 1.0;
	      B_i[i][j] = 0.0;
	  }
  }

  matrix_multiply_top(A_r, A_i, B_r, B_i, C_r, C_i);

  printf("A = \n");
  hls::print_matrix<A_ROWS, A_COLS, MATRIX_T, hls::NoTranspose>(A_r, "   ");

  printf("B = \n");
  hls::print_matrix<B_ROWS, B_COLS, MATRIX_T, hls::NoTranspose>(B_r, "   ");

  printf("C = \n");
  hls::print_matrix<C_ROWS, C_COLS, MATRIX_T, hls::NoTranspose>(C_r, "   ");
  hls::print_matrix<C_ROWS, C_COLS, MATRIX_T, hls::NoTranspose>(C_i, "   ");

  // Generate error ratio and compare against threshold value
  // - LAPACK error measurement method
  // - Threshold taken from LAPACK test functions
//  if ( difference_ratio<A_ROWS, A_COLS>(A,C) > 30.0 ) {
//    return(1);
//  }
  return(0);
}

