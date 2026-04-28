// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2024.1 (64-bit)
// Tool Version Limit: 2024.05
// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// Copyright 2022-2024 Advanced Micro Devices, Inc. All Rights Reserved.
// 
// ==============================================================
/***************************** Include Files *********************************/
#include "xcomplex_gemv_flt.h"

/************************** Function Implementation *************************/
#ifndef __linux__
int XComplex_gemv_flt_CfgInitialize(XComplex_gemv_flt *InstancePtr, XComplex_gemv_flt_Config *ConfigPtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(ConfigPtr != NULL);

    InstancePtr->Control_BaseAddress = ConfigPtr->Control_BaseAddress;
    InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

    return XST_SUCCESS;
}
#endif

void XComplex_gemv_flt_Start(XComplex_gemv_flt *InstancePtr) {
    u32 Data;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XComplex_gemv_flt_ReadReg(InstancePtr->Control_BaseAddress, XCOMPLEX_GEMV_FLT_CONTROL_ADDR_AP_CTRL) & 0x80;
    XComplex_gemv_flt_WriteReg(InstancePtr->Control_BaseAddress, XCOMPLEX_GEMV_FLT_CONTROL_ADDR_AP_CTRL, Data | 0x01);
}

u32 XComplex_gemv_flt_IsDone(XComplex_gemv_flt *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XComplex_gemv_flt_ReadReg(InstancePtr->Control_BaseAddress, XCOMPLEX_GEMV_FLT_CONTROL_ADDR_AP_CTRL);
    return (Data >> 1) & 0x1;
}

u32 XComplex_gemv_flt_IsIdle(XComplex_gemv_flt *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XComplex_gemv_flt_ReadReg(InstancePtr->Control_BaseAddress, XCOMPLEX_GEMV_FLT_CONTROL_ADDR_AP_CTRL);
    return (Data >> 2) & 0x1;
}

u32 XComplex_gemv_flt_IsReady(XComplex_gemv_flt *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XComplex_gemv_flt_ReadReg(InstancePtr->Control_BaseAddress, XCOMPLEX_GEMV_FLT_CONTROL_ADDR_AP_CTRL);
    // check ap_start to see if the pcore is ready for next input
    return !(Data & 0x1);
}

void XComplex_gemv_flt_EnableAutoRestart(XComplex_gemv_flt *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XComplex_gemv_flt_WriteReg(InstancePtr->Control_BaseAddress, XCOMPLEX_GEMV_FLT_CONTROL_ADDR_AP_CTRL, 0x80);
}

void XComplex_gemv_flt_DisableAutoRestart(XComplex_gemv_flt *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XComplex_gemv_flt_WriteReg(InstancePtr->Control_BaseAddress, XCOMPLEX_GEMV_FLT_CONTROL_ADDR_AP_CTRL, 0);
}

void XComplex_gemv_flt_Set_input1(XComplex_gemv_flt *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XComplex_gemv_flt_WriteReg(InstancePtr->Control_BaseAddress, XCOMPLEX_GEMV_FLT_CONTROL_ADDR_INPUT1_DATA, (u32)(Data));
    XComplex_gemv_flt_WriteReg(InstancePtr->Control_BaseAddress, XCOMPLEX_GEMV_FLT_CONTROL_ADDR_INPUT1_DATA + 4, (u32)(Data >> 32));
}

u64 XComplex_gemv_flt_Get_input1(XComplex_gemv_flt *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XComplex_gemv_flt_ReadReg(InstancePtr->Control_BaseAddress, XCOMPLEX_GEMV_FLT_CONTROL_ADDR_INPUT1_DATA);
    Data += (u64)XComplex_gemv_flt_ReadReg(InstancePtr->Control_BaseAddress, XCOMPLEX_GEMV_FLT_CONTROL_ADDR_INPUT1_DATA + 4) << 32;
    return Data;
}

void XComplex_gemv_flt_Set_input2(XComplex_gemv_flt *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XComplex_gemv_flt_WriteReg(InstancePtr->Control_BaseAddress, XCOMPLEX_GEMV_FLT_CONTROL_ADDR_INPUT2_DATA, (u32)(Data));
    XComplex_gemv_flt_WriteReg(InstancePtr->Control_BaseAddress, XCOMPLEX_GEMV_FLT_CONTROL_ADDR_INPUT2_DATA + 4, (u32)(Data >> 32));
}

u64 XComplex_gemv_flt_Get_input2(XComplex_gemv_flt *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XComplex_gemv_flt_ReadReg(InstancePtr->Control_BaseAddress, XCOMPLEX_GEMV_FLT_CONTROL_ADDR_INPUT2_DATA);
    Data += (u64)XComplex_gemv_flt_ReadReg(InstancePtr->Control_BaseAddress, XCOMPLEX_GEMV_FLT_CONTROL_ADDR_INPUT2_DATA + 4) << 32;
    return Data;
}

void XComplex_gemv_flt_Set_output_r(XComplex_gemv_flt *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XComplex_gemv_flt_WriteReg(InstancePtr->Control_BaseAddress, XCOMPLEX_GEMV_FLT_CONTROL_ADDR_OUTPUT_R_DATA, (u32)(Data));
    XComplex_gemv_flt_WriteReg(InstancePtr->Control_BaseAddress, XCOMPLEX_GEMV_FLT_CONTROL_ADDR_OUTPUT_R_DATA + 4, (u32)(Data >> 32));
}

u64 XComplex_gemv_flt_Get_output_r(XComplex_gemv_flt *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XComplex_gemv_flt_ReadReg(InstancePtr->Control_BaseAddress, XCOMPLEX_GEMV_FLT_CONTROL_ADDR_OUTPUT_R_DATA);
    Data += (u64)XComplex_gemv_flt_ReadReg(InstancePtr->Control_BaseAddress, XCOMPLEX_GEMV_FLT_CONTROL_ADDR_OUTPUT_R_DATA + 4) << 32;
    return Data;
}

void XComplex_gemv_flt_InterruptGlobalEnable(XComplex_gemv_flt *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XComplex_gemv_flt_WriteReg(InstancePtr->Control_BaseAddress, XCOMPLEX_GEMV_FLT_CONTROL_ADDR_GIE, 1);
}

void XComplex_gemv_flt_InterruptGlobalDisable(XComplex_gemv_flt *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XComplex_gemv_flt_WriteReg(InstancePtr->Control_BaseAddress, XCOMPLEX_GEMV_FLT_CONTROL_ADDR_GIE, 0);
}

void XComplex_gemv_flt_InterruptEnable(XComplex_gemv_flt *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XComplex_gemv_flt_ReadReg(InstancePtr->Control_BaseAddress, XCOMPLEX_GEMV_FLT_CONTROL_ADDR_IER);
    XComplex_gemv_flt_WriteReg(InstancePtr->Control_BaseAddress, XCOMPLEX_GEMV_FLT_CONTROL_ADDR_IER, Register | Mask);
}

void XComplex_gemv_flt_InterruptDisable(XComplex_gemv_flt *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XComplex_gemv_flt_ReadReg(InstancePtr->Control_BaseAddress, XCOMPLEX_GEMV_FLT_CONTROL_ADDR_IER);
    XComplex_gemv_flt_WriteReg(InstancePtr->Control_BaseAddress, XCOMPLEX_GEMV_FLT_CONTROL_ADDR_IER, Register & (~Mask));
}

void XComplex_gemv_flt_InterruptClear(XComplex_gemv_flt *InstancePtr, u32 Mask) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XComplex_gemv_flt_WriteReg(InstancePtr->Control_BaseAddress, XCOMPLEX_GEMV_FLT_CONTROL_ADDR_ISR, Mask);
}

u32 XComplex_gemv_flt_InterruptGetEnabled(XComplex_gemv_flt *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XComplex_gemv_flt_ReadReg(InstancePtr->Control_BaseAddress, XCOMPLEX_GEMV_FLT_CONTROL_ADDR_IER);
}

u32 XComplex_gemv_flt_InterruptGetStatus(XComplex_gemv_flt *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XComplex_gemv_flt_ReadReg(InstancePtr->Control_BaseAddress, XCOMPLEX_GEMV_FLT_CONTROL_ADDR_ISR);
}

