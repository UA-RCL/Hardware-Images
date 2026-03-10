// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2024.2 (64-bit)
// Tool Version Limit: 2024.11
// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// Copyright 2022-2024 Advanced Micro Devices, Inc. All Rights Reserved.
// 
// ==============================================================
// control
// 0x00 : Control signals
//        bit 0  - ap_start (Read/Write/COH)
//        bit 1  - ap_done (Read/COR)
//        bit 2  - ap_idle (Read)
//        bit 3  - ap_ready (Read/COR)
//        bit 7  - auto_restart (Read/Write)
//        bit 9  - interrupt (Read)
//        others - reserved
// 0x04 : Global Interrupt Enable Register
//        bit 0  - Global Interrupt Enable (Read/Write)
//        others - reserved
// 0x08 : IP Interrupt Enable Register (Read/Write)
//        bit 0 - enable ap_done interrupt (Read/Write)
//        bit 1 - enable ap_ready interrupt (Read/Write)
//        others - reserved
// 0x0c : IP Interrupt Status Register (Read/TOW)
//        bit 0 - ap_done (Read/TOW)
//        bit 1 - ap_ready (Read/TOW)
//        others - reserved
// 0x10 : Data signal of conv2d_bias
//        bit 31~0 - conv2d_bias[31:0] (Read/Write)
// 0x14 : reserved
// 0x18 : Data signal of stride
//        bit 31~0 - stride[31:0] (Read/Write)
// 0x1c : reserved
// 0x20 : Data signal of padding
//        bit 31~0 - padding[31:0] (Read/Write)
// 0x24 : reserved
// 0x28 : Data signal of input_h
//        bit 31~0 - input_h[31:0] (Read/Write)
// 0x2c : reserved
// 0x30 : Data signal of input_w
//        bit 31~0 - input_w[31:0] (Read/Write)
// 0x34 : reserved
// 0x38 : Data signal of kernel_h
//        bit 31~0 - kernel_h[31:0] (Read/Write)
// 0x3c : reserved
// 0x40 : Data signal of kernel_w
//        bit 31~0 - kernel_w[31:0] (Read/Write)
// 0x44 : reserved
// (SC = Self Clear, COR = Clear on Read, TOW = Toggle on Write, COH = Clear on Handshake)

#define XCONV2D_CONTROL_ADDR_AP_CTRL          0x00
#define XCONV2D_CONTROL_ADDR_GIE              0x04
#define XCONV2D_CONTROL_ADDR_IER              0x08
#define XCONV2D_CONTROL_ADDR_ISR              0x0c
#define XCONV2D_CONTROL_ADDR_CONV2D_BIAS_DATA 0x10
#define XCONV2D_CONTROL_BITS_CONV2D_BIAS_DATA 32
#define XCONV2D_CONTROL_ADDR_STRIDE_DATA      0x18
#define XCONV2D_CONTROL_BITS_STRIDE_DATA      32
#define XCONV2D_CONTROL_ADDR_PADDING_DATA     0x20
#define XCONV2D_CONTROL_BITS_PADDING_DATA     32
#define XCONV2D_CONTROL_ADDR_INPUT_H_DATA     0x28
#define XCONV2D_CONTROL_BITS_INPUT_H_DATA     32
#define XCONV2D_CONTROL_ADDR_INPUT_W_DATA     0x30
#define XCONV2D_CONTROL_BITS_INPUT_W_DATA     32
#define XCONV2D_CONTROL_ADDR_KERNEL_H_DATA    0x38
#define XCONV2D_CONTROL_BITS_KERNEL_H_DATA    32
#define XCONV2D_CONTROL_ADDR_KERNEL_W_DATA    0x40
#define XCONV2D_CONTROL_BITS_KERNEL_W_DATA    32

