// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2024.2 (64-bit)
// Tool Version Limit: 2024.11
// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// Copyright 2022-2024 Advanced Micro Devices, Inc. All Rights Reserved.
// 
// ==============================================================
// control
// 0x00 : reserved
// 0x04 : reserved
// 0x08 : reserved
// 0x0c : reserved
// 0x10 : Data signal of mode
//        bit 7~0 - mode[7:0] (Read/Write)
//        others  - reserved
// 0x14 : reserved
// 0x18 : Data signal of target_mode
//        bit 7~0 - target_mode[7:0] (Read/Write)
//        others  - reserved
// 0x1c : reserved
// 0x20 : Data signal of pe_x
//        bit 7~0 - pe_x[7:0] (Read/Write)
//        others  - reserved
// 0x24 : reserved
// 0x28 : Data signal of pe_y
//        bit 7~0 - pe_y[7:0] (Read/Write)
//        others  - reserved
// 0x2c : reserved
// 0x30 : Data signal of cluster_id
//        bit 7~0 - cluster_id[7:0] (Read/Write)
//        others  - reserved
// 0x34 : reserved
// 0x38 : Data signal of bank_id
//        bit 7~0 - bank_id[7:0] (Read/Write)
//        others  - reserved
// 0x3c : reserved
// 0x40 : Data signal of cycles
//        bit 31~0 - cycles[31:0] (Read/Write)
// 0x44 : reserved
// 0x48 : Data signal of valid_signal
//        bit 0  - valid_signal[0] (Read/Write)
//        others - reserved
// 0x4c : reserved
// 0x50 : Data signal of busy_mask_out
//        bit 31~0 - busy_mask_out[31:0] (Read)
// 0x54 : Control signal of busy_mask_out
//        bit 0  - busy_mask_out_ap_vld (Read/COR)
//        others - reserved
// 0x60 ~
// 0x67 : Memory 'busy_map_out_bits' (2 * 32b)
//        Word n : bit [31:0] - busy_map_out_bits[n]
// (SC = Self Clear, COR = Clear on Read, TOW = Toggle on Write, COH = Clear on Handshake)

#define XPROGRAM_MANAGER_TOP_CONTROL_ADDR_MODE_DATA              0x10
#define XPROGRAM_MANAGER_TOP_CONTROL_BITS_MODE_DATA              8
#define XPROGRAM_MANAGER_TOP_CONTROL_ADDR_TARGET_MODE_DATA       0x18
#define XPROGRAM_MANAGER_TOP_CONTROL_BITS_TARGET_MODE_DATA       8
#define XPROGRAM_MANAGER_TOP_CONTROL_ADDR_PE_X_DATA              0x20
#define XPROGRAM_MANAGER_TOP_CONTROL_BITS_PE_X_DATA              8
#define XPROGRAM_MANAGER_TOP_CONTROL_ADDR_PE_Y_DATA              0x28
#define XPROGRAM_MANAGER_TOP_CONTROL_BITS_PE_Y_DATA              8
#define XPROGRAM_MANAGER_TOP_CONTROL_ADDR_CLUSTER_ID_DATA        0x30
#define XPROGRAM_MANAGER_TOP_CONTROL_BITS_CLUSTER_ID_DATA        8
#define XPROGRAM_MANAGER_TOP_CONTROL_ADDR_BANK_ID_DATA           0x38
#define XPROGRAM_MANAGER_TOP_CONTROL_BITS_BANK_ID_DATA           8
#define XPROGRAM_MANAGER_TOP_CONTROL_ADDR_CYCLES_DATA            0x40
#define XPROGRAM_MANAGER_TOP_CONTROL_BITS_CYCLES_DATA            32
#define XPROGRAM_MANAGER_TOP_CONTROL_ADDR_VALID_SIGNAL_DATA      0x48
#define XPROGRAM_MANAGER_TOP_CONTROL_BITS_VALID_SIGNAL_DATA      1
#define XPROGRAM_MANAGER_TOP_CONTROL_ADDR_BUSY_MASK_OUT_DATA     0x50
#define XPROGRAM_MANAGER_TOP_CONTROL_BITS_BUSY_MASK_OUT_DATA     32
#define XPROGRAM_MANAGER_TOP_CONTROL_ADDR_BUSY_MASK_OUT_CTRL     0x54
#define XPROGRAM_MANAGER_TOP_CONTROL_ADDR_BUSY_MAP_OUT_BITS_BASE 0x60
#define XPROGRAM_MANAGER_TOP_CONTROL_ADDR_BUSY_MAP_OUT_BITS_HIGH 0x67
#define XPROGRAM_MANAGER_TOP_CONTROL_WIDTH_BUSY_MAP_OUT_BITS     32
#define XPROGRAM_MANAGER_TOP_CONTROL_DEPTH_BUSY_MAP_OUT_BITS     2

