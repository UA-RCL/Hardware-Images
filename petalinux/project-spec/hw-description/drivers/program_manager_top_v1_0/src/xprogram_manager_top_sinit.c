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
#include "xprogram_manager_top.h"

extern XProgram_manager_top_Config XProgram_manager_top_ConfigTable[];

#ifdef SDT
XProgram_manager_top_Config *XProgram_manager_top_LookupConfig(UINTPTR BaseAddress) {
	XProgram_manager_top_Config *ConfigPtr = NULL;

	int Index;

	for (Index = (u32)0x0; XProgram_manager_top_ConfigTable[Index].Name != NULL; Index++) {
		if (!BaseAddress || XProgram_manager_top_ConfigTable[Index].Control_BaseAddress == BaseAddress) {
			ConfigPtr = &XProgram_manager_top_ConfigTable[Index];
			break;
		}
	}

	return ConfigPtr;
}

int XProgram_manager_top_Initialize(XProgram_manager_top *InstancePtr, UINTPTR BaseAddress) {
	XProgram_manager_top_Config *ConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	ConfigPtr = XProgram_manager_top_LookupConfig(BaseAddress);
	if (ConfigPtr == NULL) {
		InstancePtr->IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	return XProgram_manager_top_CfgInitialize(InstancePtr, ConfigPtr);
}
#else
XProgram_manager_top_Config *XProgram_manager_top_LookupConfig(u16 DeviceId) {
	XProgram_manager_top_Config *ConfigPtr = NULL;

	int Index;

	for (Index = 0; Index < XPAR_XPROGRAM_MANAGER_TOP_NUM_INSTANCES; Index++) {
		if (XProgram_manager_top_ConfigTable[Index].DeviceId == DeviceId) {
			ConfigPtr = &XProgram_manager_top_ConfigTable[Index];
			break;
		}
	}

	return ConfigPtr;
}

int XProgram_manager_top_Initialize(XProgram_manager_top *InstancePtr, u16 DeviceId) {
	XProgram_manager_top_Config *ConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	ConfigPtr = XProgram_manager_top_LookupConfig(DeviceId);
	if (ConfigPtr == NULL) {
		InstancePtr->IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	return XProgram_manager_top_CfgInitialize(InstancePtr, ConfigPtr);
}
#endif

#endif

