// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2020.2 (64-bit)
// Copyright 1986-2020 Xilinx, Inc. All Rights Reserved.
// ==============================================================
/***************************** Include Files *********************************/
#include "xzip_hls_accel.h"

/************************** Function Implementation *************************/
#ifndef __linux__
int XZip_hls_accel_CfgInitialize(XZip_hls_accel *InstancePtr, XZip_hls_accel_Config *ConfigPtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(ConfigPtr != NULL);

    InstancePtr->Ctrl_BaseAddress = ConfigPtr->Ctrl_BaseAddress;
    InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

    return XST_SUCCESS;
}
#endif

void XZip_hls_accel_Start(XZip_hls_accel *InstancePtr) {
    u32 Data;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XZip_hls_accel_ReadReg(InstancePtr->Ctrl_BaseAddress, XZIP_HLS_ACCEL_CTRL_ADDR_AP_CTRL) & 0x80;
    XZip_hls_accel_WriteReg(InstancePtr->Ctrl_BaseAddress, XZIP_HLS_ACCEL_CTRL_ADDR_AP_CTRL, Data | 0x01);
}

u32 XZip_hls_accel_IsDone(XZip_hls_accel *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XZip_hls_accel_ReadReg(InstancePtr->Ctrl_BaseAddress, XZIP_HLS_ACCEL_CTRL_ADDR_AP_CTRL);
    return (Data >> 1) & 0x1;
}

u32 XZip_hls_accel_IsIdle(XZip_hls_accel *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XZip_hls_accel_ReadReg(InstancePtr->Ctrl_BaseAddress, XZIP_HLS_ACCEL_CTRL_ADDR_AP_CTRL);
    return (Data >> 2) & 0x1;
}

u32 XZip_hls_accel_IsReady(XZip_hls_accel *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XZip_hls_accel_ReadReg(InstancePtr->Ctrl_BaseAddress, XZIP_HLS_ACCEL_CTRL_ADDR_AP_CTRL);
    // check ap_start to see if the pcore is ready for next input
    return !(Data & 0x1);
}

void XZip_hls_accel_EnableAutoRestart(XZip_hls_accel *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XZip_hls_accel_WriteReg(InstancePtr->Ctrl_BaseAddress, XZIP_HLS_ACCEL_CTRL_ADDR_AP_CTRL, 0x80);
}

void XZip_hls_accel_DisableAutoRestart(XZip_hls_accel *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XZip_hls_accel_WriteReg(InstancePtr->Ctrl_BaseAddress, XZIP_HLS_ACCEL_CTRL_ADDR_AP_CTRL, 0);
}

void XZip_hls_accel_Set_size(XZip_hls_accel *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XZip_hls_accel_WriteReg(InstancePtr->Ctrl_BaseAddress, XZIP_HLS_ACCEL_CTRL_ADDR_SIZE_DATA, Data);
}

u32 XZip_hls_accel_Get_size(XZip_hls_accel *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XZip_hls_accel_ReadReg(InstancePtr->Ctrl_BaseAddress, XZIP_HLS_ACCEL_CTRL_ADDR_SIZE_DATA);
    return Data;
}

void XZip_hls_accel_Set_op(XZip_hls_accel *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XZip_hls_accel_WriteReg(InstancePtr->Ctrl_BaseAddress, XZIP_HLS_ACCEL_CTRL_ADDR_OP_DATA, Data);
}

u32 XZip_hls_accel_Get_op(XZip_hls_accel *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XZip_hls_accel_ReadReg(InstancePtr->Ctrl_BaseAddress, XZIP_HLS_ACCEL_CTRL_ADDR_OP_DATA);
    return Data;
}

void XZip_hls_accel_InterruptGlobalEnable(XZip_hls_accel *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XZip_hls_accel_WriteReg(InstancePtr->Ctrl_BaseAddress, XZIP_HLS_ACCEL_CTRL_ADDR_GIE, 1);
}

void XZip_hls_accel_InterruptGlobalDisable(XZip_hls_accel *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XZip_hls_accel_WriteReg(InstancePtr->Ctrl_BaseAddress, XZIP_HLS_ACCEL_CTRL_ADDR_GIE, 0);
}

void XZip_hls_accel_InterruptEnable(XZip_hls_accel *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XZip_hls_accel_ReadReg(InstancePtr->Ctrl_BaseAddress, XZIP_HLS_ACCEL_CTRL_ADDR_IER);
    XZip_hls_accel_WriteReg(InstancePtr->Ctrl_BaseAddress, XZIP_HLS_ACCEL_CTRL_ADDR_IER, Register | Mask);
}

void XZip_hls_accel_InterruptDisable(XZip_hls_accel *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XZip_hls_accel_ReadReg(InstancePtr->Ctrl_BaseAddress, XZIP_HLS_ACCEL_CTRL_ADDR_IER);
    XZip_hls_accel_WriteReg(InstancePtr->Ctrl_BaseAddress, XZIP_HLS_ACCEL_CTRL_ADDR_IER, Register & (~Mask));
}

void XZip_hls_accel_InterruptClear(XZip_hls_accel *InstancePtr, u32 Mask) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XZip_hls_accel_WriteReg(InstancePtr->Ctrl_BaseAddress, XZIP_HLS_ACCEL_CTRL_ADDR_ISR, Mask);
}

u32 XZip_hls_accel_InterruptGetEnabled(XZip_hls_accel *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XZip_hls_accel_ReadReg(InstancePtr->Ctrl_BaseAddress, XZIP_HLS_ACCEL_CTRL_ADDR_IER);
}

u32 XZip_hls_accel_InterruptGetStatus(XZip_hls_accel *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XZip_hls_accel_ReadReg(InstancePtr->Ctrl_BaseAddress, XZIP_HLS_ACCEL_CTRL_ADDR_ISR);
}

