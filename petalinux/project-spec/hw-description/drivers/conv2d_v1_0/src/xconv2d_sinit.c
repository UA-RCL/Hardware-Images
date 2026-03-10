// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2024.2 (64-bit)
// Tool Version Limit: 2024.11
// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// Copyright 2022-2024 Advanced Micro Devices, Inc. All Rights Reserved.
// 
// ==============================================================
#ifndef __linux__

#include "xstatus.h"
#ifdef SDT
#include "xparameters.h"
#endif
#include "xconv2d.h"

extern XConv2d_Config XConv2d_ConfigTable[];

#ifdef SDT
XConv2d_Config *XConv2d_LookupConfig(UINTPTR BaseAddress) {
	XConv2d_Config *ConfigPtr = NULL;

	int Index;

	for (Index = (u32)0x0; XConv2d_ConfigTable[Index].Name != NULL; Index++) {
		if (!BaseAddress || XConv2d_ConfigTable[Index].Control_BaseAddress == BaseAddress) {
			ConfigPtr = &XConv2d_ConfigTable[Index];
			break;
		}
	}

	return ConfigPtr;
}

int XConv2d_Initialize(XConv2d *InstancePtr, UINTPTR BaseAddress) {
	XConv2d_Config *ConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	ConfigPtr = XConv2d_LookupConfig(BaseAddress);
	if (ConfigPtr == NULL) {
		InstancePtr->IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	return XConv2d_CfgInitialize(InstancePtr, ConfigPtr);
}
#else
XConv2d_Config *XConv2d_LookupConfig(u16 DeviceId) {
	XConv2d_Config *ConfigPtr = NULL;

	int Index;

	for (Index = 0; Index < XPAR_XCONV2D_NUM_INSTANCES; Index++) {
		if (XConv2d_ConfigTable[Index].DeviceId == DeviceId) {
			ConfigPtr = &XConv2d_ConfigTable[Index];
			break;
		}
	}

	return ConfigPtr;
}

int XConv2d_Initialize(XConv2d *InstancePtr, u16 DeviceId) {
	XConv2d_Config *ConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	ConfigPtr = XConv2d_LookupConfig(DeviceId);
	if (ConfigPtr == NULL) {
		InstancePtr->IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	return XConv2d_CfgInitialize(InstancePtr, ConfigPtr);
}
#endif

#endif

