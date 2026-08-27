// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2024.2 (64-bit)
// Tool Version Limit: 2024.11
// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// Copyright 2022-2024 Advanced Micro Devices, Inc. All Rights Reserved.
// 
// ==============================================================
/***************************** Include Files *********************************/
#include "xprogram_manager_top.h"

/************************** Function Implementation *************************/
#ifndef __linux__
int XProgram_manager_top_CfgInitialize(XProgram_manager_top *InstancePtr, XProgram_manager_top_Config *ConfigPtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(ConfigPtr != NULL);

    InstancePtr->Control_BaseAddress = ConfigPtr->Control_BaseAddress;
    InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

    return XST_SUCCESS;
}
#endif

void XProgram_manager_top_Set_mode(XProgram_manager_top *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XProgram_manager_top_WriteReg(InstancePtr->Control_BaseAddress, XPROGRAM_MANAGER_TOP_CONTROL_ADDR_MODE_DATA, Data);
}

u32 XProgram_manager_top_Get_mode(XProgram_manager_top *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XProgram_manager_top_ReadReg(InstancePtr->Control_BaseAddress, XPROGRAM_MANAGER_TOP_CONTROL_ADDR_MODE_DATA);
    return Data;
}

void XProgram_manager_top_Set_target_mode(XProgram_manager_top *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XProgram_manager_top_WriteReg(InstancePtr->Control_BaseAddress, XPROGRAM_MANAGER_TOP_CONTROL_ADDR_TARGET_MODE_DATA, Data);
}

u32 XProgram_manager_top_Get_target_mode(XProgram_manager_top *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XProgram_manager_top_ReadReg(InstancePtr->Control_BaseAddress, XPROGRAM_MANAGER_TOP_CONTROL_ADDR_TARGET_MODE_DATA);
    return Data;
}

void XProgram_manager_top_Set_pe_x(XProgram_manager_top *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XProgram_manager_top_WriteReg(InstancePtr->Control_BaseAddress, XPROGRAM_MANAGER_TOP_CONTROL_ADDR_PE_X_DATA, Data);
}

u32 XProgram_manager_top_Get_pe_x(XProgram_manager_top *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XProgram_manager_top_ReadReg(InstancePtr->Control_BaseAddress, XPROGRAM_MANAGER_TOP_CONTROL_ADDR_PE_X_DATA);
    return Data;
}

void XProgram_manager_top_Set_pe_y(XProgram_manager_top *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XProgram_manager_top_WriteReg(InstancePtr->Control_BaseAddress, XPROGRAM_MANAGER_TOP_CONTROL_ADDR_PE_Y_DATA, Data);
}

u32 XProgram_manager_top_Get_pe_y(XProgram_manager_top *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XProgram_manager_top_ReadReg(InstancePtr->Control_BaseAddress, XPROGRAM_MANAGER_TOP_CONTROL_ADDR_PE_Y_DATA);
    return Data;
}

void XProgram_manager_top_Set_cluster_id(XProgram_manager_top *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XProgram_manager_top_WriteReg(InstancePtr->Control_BaseAddress, XPROGRAM_MANAGER_TOP_CONTROL_ADDR_CLUSTER_ID_DATA, Data);
}

u32 XProgram_manager_top_Get_cluster_id(XProgram_manager_top *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XProgram_manager_top_ReadReg(InstancePtr->Control_BaseAddress, XPROGRAM_MANAGER_TOP_CONTROL_ADDR_CLUSTER_ID_DATA);
    return Data;
}

void XProgram_manager_top_Set_bank_id(XProgram_manager_top *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XProgram_manager_top_WriteReg(InstancePtr->Control_BaseAddress, XPROGRAM_MANAGER_TOP_CONTROL_ADDR_BANK_ID_DATA, Data);
}

u32 XProgram_manager_top_Get_bank_id(XProgram_manager_top *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XProgram_manager_top_ReadReg(InstancePtr->Control_BaseAddress, XPROGRAM_MANAGER_TOP_CONTROL_ADDR_BANK_ID_DATA);
    return Data;
}

void XProgram_manager_top_Set_cycles(XProgram_manager_top *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XProgram_manager_top_WriteReg(InstancePtr->Control_BaseAddress, XPROGRAM_MANAGER_TOP_CONTROL_ADDR_CYCLES_DATA, Data);
}

u32 XProgram_manager_top_Get_cycles(XProgram_manager_top *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XProgram_manager_top_ReadReg(InstancePtr->Control_BaseAddress, XPROGRAM_MANAGER_TOP_CONTROL_ADDR_CYCLES_DATA);
    return Data;
}

void XProgram_manager_top_Set_valid_signal(XProgram_manager_top *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XProgram_manager_top_WriteReg(InstancePtr->Control_BaseAddress, XPROGRAM_MANAGER_TOP_CONTROL_ADDR_VALID_SIGNAL_DATA, Data);
}

u32 XProgram_manager_top_Get_valid_signal(XProgram_manager_top *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XProgram_manager_top_ReadReg(InstancePtr->Control_BaseAddress, XPROGRAM_MANAGER_TOP_CONTROL_ADDR_VALID_SIGNAL_DATA);
    return Data;
}

u32 XProgram_manager_top_Get_busy_mask_out(XProgram_manager_top *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XProgram_manager_top_ReadReg(InstancePtr->Control_BaseAddress, XPROGRAM_MANAGER_TOP_CONTROL_ADDR_BUSY_MASK_OUT_DATA);
    return Data;
}

u32 XProgram_manager_top_Get_busy_mask_out_vld(XProgram_manager_top *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XProgram_manager_top_ReadReg(InstancePtr->Control_BaseAddress, XPROGRAM_MANAGER_TOP_CONTROL_ADDR_BUSY_MASK_OUT_CTRL);
    return Data & 0x1;
}

u32 XProgram_manager_top_Get_cfg_status(XProgram_manager_top *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XProgram_manager_top_ReadReg(InstancePtr->Control_BaseAddress, XPROGRAM_MANAGER_TOP_CONTROL_ADDR_CFG_STATUS_DATA);
    return Data;
}

u32 XProgram_manager_top_Get_cfg_status_vld(XProgram_manager_top *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XProgram_manager_top_ReadReg(InstancePtr->Control_BaseAddress, XPROGRAM_MANAGER_TOP_CONTROL_ADDR_CFG_STATUS_CTRL);
    return Data & 0x1;
}

u32 XProgram_manager_top_Get_busy_map_out_bits_BaseAddress(XProgram_manager_top *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return (InstancePtr->Control_BaseAddress + XPROGRAM_MANAGER_TOP_CONTROL_ADDR_BUSY_MAP_OUT_BITS_BASE);
}

u32 XProgram_manager_top_Get_busy_map_out_bits_HighAddress(XProgram_manager_top *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return (InstancePtr->Control_BaseAddress + XPROGRAM_MANAGER_TOP_CONTROL_ADDR_BUSY_MAP_OUT_BITS_HIGH);
}

u32 XProgram_manager_top_Get_busy_map_out_bits_TotalBytes(XProgram_manager_top *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return (XPROGRAM_MANAGER_TOP_CONTROL_ADDR_BUSY_MAP_OUT_BITS_HIGH - XPROGRAM_MANAGER_TOP_CONTROL_ADDR_BUSY_MAP_OUT_BITS_BASE + 1);
}

u32 XProgram_manager_top_Get_busy_map_out_bits_BitWidth(XProgram_manager_top *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XPROGRAM_MANAGER_TOP_CONTROL_WIDTH_BUSY_MAP_OUT_BITS;
}

u32 XProgram_manager_top_Get_busy_map_out_bits_Depth(XProgram_manager_top *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XPROGRAM_MANAGER_TOP_CONTROL_DEPTH_BUSY_MAP_OUT_BITS;
}

u32 XProgram_manager_top_Write_busy_map_out_bits_Words(XProgram_manager_top *InstancePtr, int offset, word_type *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length)*4 > (XPROGRAM_MANAGER_TOP_CONTROL_ADDR_BUSY_MAP_OUT_BITS_HIGH - XPROGRAM_MANAGER_TOP_CONTROL_ADDR_BUSY_MAP_OUT_BITS_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(int *)(InstancePtr->Control_BaseAddress + XPROGRAM_MANAGER_TOP_CONTROL_ADDR_BUSY_MAP_OUT_BITS_BASE + (offset + i)*4) = *(data + i);
    }
    return length;
}

u32 XProgram_manager_top_Read_busy_map_out_bits_Words(XProgram_manager_top *InstancePtr, int offset, word_type *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length)*4 > (XPROGRAM_MANAGER_TOP_CONTROL_ADDR_BUSY_MAP_OUT_BITS_HIGH - XPROGRAM_MANAGER_TOP_CONTROL_ADDR_BUSY_MAP_OUT_BITS_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(data + i) = *(int *)(InstancePtr->Control_BaseAddress + XPROGRAM_MANAGER_TOP_CONTROL_ADDR_BUSY_MAP_OUT_BITS_BASE + (offset + i)*4);
    }
    return length;
}

u32 XProgram_manager_top_Write_busy_map_out_bits_Bytes(XProgram_manager_top *InstancePtr, int offset, char *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length) > (XPROGRAM_MANAGER_TOP_CONTROL_ADDR_BUSY_MAP_OUT_BITS_HIGH - XPROGRAM_MANAGER_TOP_CONTROL_ADDR_BUSY_MAP_OUT_BITS_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(char *)(InstancePtr->Control_BaseAddress + XPROGRAM_MANAGER_TOP_CONTROL_ADDR_BUSY_MAP_OUT_BITS_BASE + offset + i) = *(data + i);
    }
    return length;
}

u32 XProgram_manager_top_Read_busy_map_out_bits_Bytes(XProgram_manager_top *InstancePtr, int offset, char *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length) > (XPROGRAM_MANAGER_TOP_CONTROL_ADDR_BUSY_MAP_OUT_BITS_HIGH - XPROGRAM_MANAGER_TOP_CONTROL_ADDR_BUSY_MAP_OUT_BITS_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(data + i) = *(char *)(InstancePtr->Control_BaseAddress + XPROGRAM_MANAGER_TOP_CONTROL_ADDR_BUSY_MAP_OUT_BITS_BASE + offset + i);
    }
    return length;
}

