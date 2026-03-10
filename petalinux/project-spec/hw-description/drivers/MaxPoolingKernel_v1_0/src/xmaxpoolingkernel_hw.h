// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2024.2 (64-bit)
// Tool Version Limit: 2024.11
// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// Copyright 2022-2024 Advanced Micro Devices, Inc. All Rights Reserved.
// 
// ==============================================================
// config
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
// 0x10 : Data signal of pool_size
//        bit 31~0 - pool_size[31:0] (Read/Write)
// 0x14 : reserved
// 0x18 : Data signal of pool_stride
//        bit 31~0 - pool_stride[31:0] (Read/Write)
// 0x1c : reserved
// 0x20 : Data signal of input_h
//        bit 31~0 - input_h[31:0] (Read/Write)
// 0x24 : reserved
// 0x28 : Data signal of input_w
//        bit 31~0 - input_w[31:0] (Read/Write)
// 0x2c : reserved
// (SC = Self Clear, COR = Clear on Read, TOW = Toggle on Write, COH = Clear on Handshake)

#define XMAXPOOLINGKERNEL_CONFIG_ADDR_AP_CTRL          0x00
#define XMAXPOOLINGKERNEL_CONFIG_ADDR_GIE              0x04
#define XMAXPOOLINGKERNEL_CONFIG_ADDR_IER              0x08
#define XMAXPOOLINGKERNEL_CONFIG_ADDR_ISR              0x0c
#define XMAXPOOLINGKERNEL_CONFIG_ADDR_POOL_SIZE_DATA   0x10
#define XMAXPOOLINGKERNEL_CONFIG_BITS_POOL_SIZE_DATA   32
#define XMAXPOOLINGKERNEL_CONFIG_ADDR_POOL_STRIDE_DATA 0x18
#define XMAXPOOLINGKERNEL_CONFIG_BITS_POOL_STRIDE_DATA 32
#define XMAXPOOLINGKERNEL_CONFIG_ADDR_INPUT_H_DATA     0x20
#define XMAXPOOLINGKERNEL_CONFIG_BITS_INPUT_H_DATA     32
#define XMAXPOOLINGKERNEL_CONFIG_ADDR_INPUT_W_DATA     0x28
#define XMAXPOOLINGKERNEL_CONFIG_BITS_INPUT_W_DATA     32

