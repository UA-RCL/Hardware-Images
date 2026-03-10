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
#include "xrelu.h"

extern XRelu_Config XRelu_ConfigTable[];

#ifdef SDT
XRelu_Config *XRelu_LookupConfig(UINTPTR BaseAddress) {
	XRelu_Config *ConfigPtr = NULL;

	int Index;

	for (Index = (u32)0x0; XRelu_ConfigTable[Index].Name != NULL; Index++) {
		if (!BaseAddress || XRelu_ConfigTable[Index].Control_BaseAddress == BaseAddress) {
			ConfigPtr = &XRelu_ConfigTable[Index];
			break;
		}
	}

	return ConfigPtr;
}

int XRelu_Initialize(XRelu *InstancePtr, UINTPTR BaseAddress) {
	XRelu_Config *ConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	ConfigPtr = XRelu_LookupConfig(BaseAddress);
	if (ConfigPtr == NULL) {
		InstancePtr->IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	return XRelu_CfgInitialize(InstancePtr, ConfigPtr);
}
#else
XRelu_Config *XRelu_LookupConfig(u16 DeviceId) {
	XRelu_Config *ConfigPtr = NULL;

	int Index;

	for (Index = 0; Index < XPAR_XRELU_NUM_INSTANCES; Index++) {
		if (XRelu_ConfigTable[Index].DeviceId == DeviceId) {
			ConfigPtr = &XRelu_ConfigTable[Index];
			break;
		}
	}

	return ConfigPtr;
}

int XRelu_Initialize(XRelu *InstancePtr, u16 DeviceId) {
	XRelu_Config *ConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	ConfigPtr = XRelu_LookupConfig(DeviceId);
	if (ConfigPtr == NULL) {
		InstancePtr->IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	return XRelu_CfgInitialize(InstancePtr, ConfigPtr);
}
#endif

#endif

