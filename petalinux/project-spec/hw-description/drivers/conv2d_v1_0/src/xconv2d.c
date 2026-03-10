// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2024.2 (64-bit)
// Tool Version Limit: 2024.11
// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// Copyright 2022-2024 Advanced Micro Devices, Inc. All Rights Reserved.
// 
// ==============================================================
/***************************** Include Files *********************************/
#include "xconv2d.h"

/************************** Function Implementation *************************/
#ifndef __linux__
int XConv2d_CfgInitialize(XConv2d *InstancePtr, XConv2d_Config *ConfigPtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(ConfigPtr != NULL);

    InstancePtr->Control_BaseAddress = ConfigPtr->Control_BaseAddress;
    InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

    return XST_SUCCESS;
}
#endif

void XConv2d_Start(XConv2d *InstancePtr) {
    u32 Data;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XConv2d_ReadReg(InstancePtr->Control_BaseAddress, XCONV2D_CONTROL_ADDR_AP_CTRL) & 0x80;
    XConv2d_WriteReg(InstancePtr->Control_BaseAddress, XCONV2D_CONTROL_ADDR_AP_CTRL, Data | 0x01);
}

u32 XConv2d_IsDone(XConv2d *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XConv2d_ReadReg(InstancePtr->Control_BaseAddress, XCONV2D_CONTROL_ADDR_AP_CTRL);
    return (Data >> 1) & 0x1;
}

u32 XConv2d_IsIdle(XConv2d *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XConv2d_ReadReg(InstancePtr->Control_BaseAddress, XCONV2D_CONTROL_ADDR_AP_CTRL);
    return (Data >> 2) & 0x1;
}

u32 XConv2d_IsReady(XConv2d *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XConv2d_ReadReg(InstancePtr->Control_BaseAddress, XCONV2D_CONTROL_ADDR_AP_CTRL);
    // check ap_start to see if the pcore is ready for next input
    return !(Data & 0x1);
}

void XConv2d_EnableAutoRestart(XConv2d *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XConv2d_WriteReg(InstancePtr->Control_BaseAddress, XCONV2D_CONTROL_ADDR_AP_CTRL, 0x80);
}

void XConv2d_DisableAutoRestart(XConv2d *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XConv2d_WriteReg(InstancePtr->Control_BaseAddress, XCONV2D_CONTROL_ADDR_AP_CTRL, 0);
}

void XConv2d_Set_conv2d_bias(XConv2d *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XConv2d_WriteReg(InstancePtr->Control_BaseAddress, XCONV2D_CONTROL_ADDR_CONV2D_BIAS_DATA, Data);
}

u32 XConv2d_Get_conv2d_bias(XConv2d *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XConv2d_ReadReg(InstancePtr->Control_BaseAddress, XCONV2D_CONTROL_ADDR_CONV2D_BIAS_DATA);
    return Data;
}

void XConv2d_Set_stride(XConv2d *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XConv2d_WriteReg(InstancePtr->Control_BaseAddress, XCONV2D_CONTROL_ADDR_STRIDE_DATA, Data);
}

u32 XConv2d_Get_stride(XConv2d *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XConv2d_ReadReg(InstancePtr->Control_BaseAddress, XCONV2D_CONTROL_ADDR_STRIDE_DATA);
    return Data;
}

void XConv2d_Set_padding(XConv2d *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XConv2d_WriteReg(InstancePtr->Control_BaseAddress, XCONV2D_CONTROL_ADDR_PADDING_DATA, Data);
}

u32 XConv2d_Get_padding(XConv2d *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XConv2d_ReadReg(InstancePtr->Control_BaseAddress, XCONV2D_CONTROL_ADDR_PADDING_DATA);
    return Data;
}

void XConv2d_Set_input_h(XConv2d *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XConv2d_WriteReg(InstancePtr->Control_BaseAddress, XCONV2D_CONTROL_ADDR_INPUT_H_DATA, Data);
}

u32 XConv2d_Get_input_h(XConv2d *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XConv2d_ReadReg(InstancePtr->Control_BaseAddress, XCONV2D_CONTROL_ADDR_INPUT_H_DATA);
    return Data;
}

void XConv2d_Set_input_w(XConv2d *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XConv2d_WriteReg(InstancePtr->Control_BaseAddress, XCONV2D_CONTROL_ADDR_INPUT_W_DATA, Data);
}

u32 XConv2d_Get_input_w(XConv2d *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XConv2d_ReadReg(InstancePtr->Control_BaseAddress, XCONV2D_CONTROL_ADDR_INPUT_W_DATA);
    return Data;
}

void XConv2d_Set_kernel_h(XConv2d *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XConv2d_WriteReg(InstancePtr->Control_BaseAddress, XCONV2D_CONTROL_ADDR_KERNEL_H_DATA, Data);
}

u32 XConv2d_Get_kernel_h(XConv2d *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XConv2d_ReadReg(InstancePtr->Control_BaseAddress, XCONV2D_CONTROL_ADDR_KERNEL_H_DATA);
    return Data;
}

void XConv2d_Set_kernel_w(XConv2d *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XConv2d_WriteReg(InstancePtr->Control_BaseAddress, XCONV2D_CONTROL_ADDR_KERNEL_W_DATA, Data);
}

u32 XConv2d_Get_kernel_w(XConv2d *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XConv2d_ReadReg(InstancePtr->Control_BaseAddress, XCONV2D_CONTROL_ADDR_KERNEL_W_DATA);
    return Data;
}

void XConv2d_InterruptGlobalEnable(XConv2d *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XConv2d_WriteReg(InstancePtr->Control_BaseAddress, XCONV2D_CONTROL_ADDR_GIE, 1);
}

void XConv2d_InterruptGlobalDisable(XConv2d *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XConv2d_WriteReg(InstancePtr->Control_BaseAddress, XCONV2D_CONTROL_ADDR_GIE, 0);
}

void XConv2d_InterruptEnable(XConv2d *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XConv2d_ReadReg(InstancePtr->Control_BaseAddress, XCONV2D_CONTROL_ADDR_IER);
    XConv2d_WriteReg(InstancePtr->Control_BaseAddress, XCONV2D_CONTROL_ADDR_IER, Register | Mask);
}

void XConv2d_InterruptDisable(XConv2d *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XConv2d_ReadReg(InstancePtr->Control_BaseAddress, XCONV2D_CONTROL_ADDR_IER);
    XConv2d_WriteReg(InstancePtr->Control_BaseAddress, XCONV2D_CONTROL_ADDR_IER, Register & (~Mask));
}

void XConv2d_InterruptClear(XConv2d *InstancePtr, u32 Mask) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XConv2d_WriteReg(InstancePtr->Control_BaseAddress, XCONV2D_CONTROL_ADDR_ISR, Mask);
}

u32 XConv2d_InterruptGetEnabled(XConv2d *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XConv2d_ReadReg(InstancePtr->Control_BaseAddress, XCONV2D_CONTROL_ADDR_IER);
}

u32 XConv2d_InterruptGetStatus(XConv2d *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XConv2d_ReadReg(InstancePtr->Control_BaseAddress, XCONV2D_CONTROL_ADDR_ISR);
}

