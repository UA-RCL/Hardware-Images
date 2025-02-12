// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2020.2 (64-bit)
// Copyright 1986-2020 Xilinx, Inc. All Rights Reserved.
// ==============================================================
// ctrl
// 0x00 : Control signals
//        bit 0  - ap_start (Read/Write/COH)
//        bit 1  - ap_done (Read/COR)
//        bit 2  - ap_idle (Read)
//        bit 3  - ap_ready (Read)
//        bit 7  - auto_restart (Read/Write)
//        others - reserved
// 0x04 : Global Interrupt Enable Register
//        bit 0  - Global Interrupt Enable (Read/Write)
//        others - reserved
// 0x08 : IP Interrupt Enable Register (Read/Write)
//        bit 0  - enable ap_done interrupt (Read/Write)
//        bit 1  - enable ap_ready interrupt (Read/Write)
//        others - reserved
// 0x0c : IP Interrupt Status Register (Read/TOW)
//        bit 0  - ap_done (COR/TOW)
//        bit 1  - ap_ready (COR/TOW)
//        others - reserved
// 0x10 : Data signal of size
//        bit 31~0 - size[31:0] (Read/Write)
// 0x14 : reserved
// 0x18 : Data signal of op
//        bit 31~0 - op[31:0] (Read/Write)
// 0x1c : reserved
// (SC = Self Clear, COR = Clear on Read, TOW = Toggle on Write, COH = Clear on Handshake)

#define XZIP_HLS_ACCEL_CTRL_ADDR_AP_CTRL   0x00
#define XZIP_HLS_ACCEL_CTRL_ADDR_GIE       0x04
#define XZIP_HLS_ACCEL_CTRL_ADDR_IER       0x08
#define XZIP_HLS_ACCEL_CTRL_ADDR_ISR       0x0c
#define XZIP_HLS_ACCEL_CTRL_ADDR_SIZE_DATA 0x10
#define XZIP_HLS_ACCEL_CTRL_BITS_SIZE_DATA 32
#define XZIP_HLS_ACCEL_CTRL_ADDR_OP_DATA   0x18
#define XZIP_HLS_ACCEL_CTRL_BITS_OP_DATA   32

