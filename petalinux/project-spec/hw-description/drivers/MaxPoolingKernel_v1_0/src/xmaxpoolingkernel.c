// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2024.2 (64-bit)
// Tool Version Limit: 2024.11
// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// Copyright 2022-2024 Advanced Micro Devices, Inc. All Rights Reserved.
// 
// ==============================================================
/***************************** Include Files *********************************/
#include "xmaxpoolingkernel.h"

/************************** Function Implementation *************************/
#ifndef __linux__
int XMaxpoolingkernel_CfgInitialize(XMaxpoolingkernel *InstancePtr, XMaxpoolingkernel_Config *ConfigPtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(ConfigPtr != NULL);

    InstancePtr->Config_BaseAddress = ConfigPtr->Config_BaseAddress;
    InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

    return XST_SUCCESS;
}
#endif

void XMaxpoolingkernel_Start(XMaxpoolingkernel *InstancePtr) {
    u32 Data;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XMaxpoolingkernel_ReadReg(InstancePtr->Config_BaseAddress, XMAXPOOLINGKERNEL_CONFIG_ADDR_AP_CTRL) & 0x80;
    XMaxpoolingkernel_WriteReg(InstancePtr->Config_BaseAddress, XMAXPOOLINGKERNEL_CONFIG_ADDR_AP_CTRL, Data | 0x01);
}

u32 XMaxpoolingkernel_IsDone(XMaxpoolingkernel *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XMaxpoolingkernel_ReadReg(InstancePtr->Config_BaseAddress, XMAXPOOLINGKERNEL_CONFIG_ADDR_AP_CTRL);
    return (Data >> 1) & 0x1;
}

u32 XMaxpoolingkernel_IsIdle(XMaxpoolingkernel *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XMaxpoolingkernel_ReadReg(InstancePtr->Config_BaseAddress, XMAXPOOLINGKERNEL_CONFIG_ADDR_AP_CTRL);
    return (Data >> 2) & 0x1;
}

u32 XMaxpoolingkernel_IsReady(XMaxpoolingkernel *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XMaxpoolingkernel_ReadReg(InstancePtr->Config_BaseAddress, XMAXPOOLINGKERNEL_CONFIG_ADDR_AP_CTRL);
    // check ap_start to see if the pcore is ready for next input
    return !(Data & 0x1);
}

void XMaxpoolingkernel_EnableAutoRestart(XMaxpoolingkernel *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XMaxpoolingkernel_WriteReg(InstancePtr->Config_BaseAddress, XMAXPOOLINGKERNEL_CONFIG_ADDR_AP_CTRL, 0x80);
}

void XMaxpoolingkernel_DisableAutoRestart(XMaxpoolingkernel *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XMaxpoolingkernel_WriteReg(InstancePtr->Config_BaseAddress, XMAXPOOLINGKERNEL_CONFIG_ADDR_AP_CTRL, 0);
}

void XMaxpoolingkernel_Set_pool_size(XMaxpoolingkernel *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XMaxpoolingkernel_WriteReg(InstancePtr->Config_BaseAddress, XMAXPOOLINGKERNEL_CONFIG_ADDR_POOL_SIZE_DATA, Data);
}

u32 XMaxpoolingkernel_Get_pool_size(XMaxpoolingkernel *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XMaxpoolingkernel_ReadReg(InstancePtr->Config_BaseAddress, XMAXPOOLINGKERNEL_CONFIG_ADDR_POOL_SIZE_DATA);
    return Data;
}

void XMaxpoolingkernel_Set_pool_stride(XMaxpoolingkernel *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XMaxpoolingkernel_WriteReg(InstancePtr->Config_BaseAddress, XMAXPOOLINGKERNEL_CONFIG_ADDR_POOL_STRIDE_DATA, Data);
}

u32 XMaxpoolingkernel_Get_pool_stride(XMaxpoolingkernel *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XMaxpoolingkernel_ReadReg(InstancePtr->Config_BaseAddress, XMAXPOOLINGKERNEL_CONFIG_ADDR_POOL_STRIDE_DATA);
    return Data;
}

void XMaxpoolingkernel_Set_input_h(XMaxpoolingkernel *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XMaxpoolingkernel_WriteReg(InstancePtr->Config_BaseAddress, XMAXPOOLINGKERNEL_CONFIG_ADDR_INPUT_H_DATA, Data);
}

u32 XMaxpoolingkernel_Get_input_h(XMaxpoolingkernel *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XMaxpoolingkernel_ReadReg(InstancePtr->Config_BaseAddress, XMAXPOOLINGKERNEL_CONFIG_ADDR_INPUT_H_DATA);
    return Data;
}

void XMaxpoolingkernel_Set_input_w(XMaxpoolingkernel *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XMaxpoolingkernel_WriteReg(InstancePtr->Config_BaseAddress, XMAXPOOLINGKERNEL_CONFIG_ADDR_INPUT_W_DATA, Data);
}

u32 XMaxpoolingkernel_Get_input_w(XMaxpoolingkernel *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XMaxpoolingkernel_ReadReg(InstancePtr->Config_BaseAddress, XMAXPOOLINGKERNEL_CONFIG_ADDR_INPUT_W_DATA);
    return Data;
}

void XMaxpoolingkernel_InterruptGlobalEnable(XMaxpoolingkernel *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XMaxpoolingkernel_WriteReg(InstancePtr->Config_BaseAddress, XMAXPOOLINGKERNEL_CONFIG_ADDR_GIE, 1);
}

void XMaxpoolingkernel_InterruptGlobalDisable(XMaxpoolingkernel *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XMaxpoolingkernel_WriteReg(InstancePtr->Config_BaseAddress, XMAXPOOLINGKERNEL_CONFIG_ADDR_GIE, 0);
}

void XMaxpoolingkernel_InterruptEnable(XMaxpoolingkernel *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XMaxpoolingkernel_ReadReg(InstancePtr->Config_BaseAddress, XMAXPOOLINGKERNEL_CONFIG_ADDR_IER);
    XMaxpoolingkernel_WriteReg(InstancePtr->Config_BaseAddress, XMAXPOOLINGKERNEL_CONFIG_ADDR_IER, Register | Mask);
}

void XMaxpoolingkernel_InterruptDisable(XMaxpoolingkernel *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XMaxpoolingkernel_ReadReg(InstancePtr->Config_BaseAddress, XMAXPOOLINGKERNEL_CONFIG_ADDR_IER);
    XMaxpoolingkernel_WriteReg(InstancePtr->Config_BaseAddress, XMAXPOOLINGKERNEL_CONFIG_ADDR_IER, Register & (~Mask));
}

void XMaxpoolingkernel_InterruptClear(XMaxpoolingkernel *InstancePtr, u32 Mask) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XMaxpoolingkernel_WriteReg(InstancePtr->Config_BaseAddress, XMAXPOOLINGKERNEL_CONFIG_ADDR_ISR, Mask);
}

u32 XMaxpoolingkernel_InterruptGetEnabled(XMaxpoolingkernel *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XMaxpoolingkernel_ReadReg(InstancePtr->Config_BaseAddress, XMAXPOOLINGKERNEL_CONFIG_ADDR_IER);
}

u32 XMaxpoolingkernel_InterruptGetStatus(XMaxpoolingkernel *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XMaxpoolingkernel_ReadReg(InstancePtr->Config_BaseAddress, XMAXPOOLINGKERNEL_CONFIG_ADDR_ISR);
}

