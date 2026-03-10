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
#include "xmaxpoolingkernel.h"

extern XMaxpoolingkernel_Config XMaxpoolingkernel_ConfigTable[];

#ifdef SDT
XMaxpoolingkernel_Config *XMaxpoolingkernel_LookupConfig(UINTPTR BaseAddress) {
	XMaxpoolingkernel_Config *ConfigPtr = NULL;

	int Index;

	for (Index = (u32)0x0; XMaxpoolingkernel_ConfigTable[Index].Name != NULL; Index++) {
		if (!BaseAddress || XMaxpoolingkernel_ConfigTable[Index].Config_BaseAddress == BaseAddress) {
			ConfigPtr = &XMaxpoolingkernel_ConfigTable[Index];
			break;
		}
	}

	return ConfigPtr;
}

int XMaxpoolingkernel_Initialize(XMaxpoolingkernel *InstancePtr, UINTPTR BaseAddress) {
	XMaxpoolingkernel_Config *ConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	ConfigPtr = XMaxpoolingkernel_LookupConfig(BaseAddress);
	if (ConfigPtr == NULL) {
		InstancePtr->IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	return XMaxpoolingkernel_CfgInitialize(InstancePtr, ConfigPtr);
}
#else
XMaxpoolingkernel_Config *XMaxpoolingkernel_LookupConfig(u16 DeviceId) {
	XMaxpoolingkernel_Config *ConfigPtr = NULL;

	int Index;

	for (Index = 0; Index < XPAR_XMAXPOOLINGKERNEL_NUM_INSTANCES; Index++) {
		if (XMaxpoolingkernel_ConfigTable[Index].DeviceId == DeviceId) {
			ConfigPtr = &XMaxpoolingkernel_ConfigTable[Index];
			break;
		}
	}

	return ConfigPtr;
}

int XMaxpoolingkernel_Initialize(XMaxpoolingkernel *InstancePtr, u16 DeviceId) {
	XMaxpoolingkernel_Config *ConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	ConfigPtr = XMaxpoolingkernel_LookupConfig(DeviceId);
	if (ConfigPtr == NULL) {
		InstancePtr->IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	return XMaxpoolingkernel_CfgInitialize(InstancePtr, ConfigPtr);
}
#endif

#endif

