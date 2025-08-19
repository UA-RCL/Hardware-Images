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
#include "xzip_hls_accel.h"

extern XZip_hls_accel_Config XZip_hls_accel_ConfigTable[];

#ifdef SDT
XZip_hls_accel_Config *XZip_hls_accel_LookupConfig(UINTPTR BaseAddress) {
	XZip_hls_accel_Config *ConfigPtr = NULL;

	int Index;

	for (Index = (u32)0x0; XZip_hls_accel_ConfigTable[Index].Name != NULL; Index++) {
		if (!BaseAddress || XZip_hls_accel_ConfigTable[Index].Ctrl_BaseAddress == BaseAddress) {
			ConfigPtr = &XZip_hls_accel_ConfigTable[Index];
			break;
		}
	}

	return ConfigPtr;
}

int XZip_hls_accel_Initialize(XZip_hls_accel *InstancePtr, UINTPTR BaseAddress) {
	XZip_hls_accel_Config *ConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	ConfigPtr = XZip_hls_accel_LookupConfig(BaseAddress);
	if (ConfigPtr == NULL) {
		InstancePtr->IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	return XZip_hls_accel_CfgInitialize(InstancePtr, ConfigPtr);
}
#else
XZip_hls_accel_Config *XZip_hls_accel_LookupConfig(u16 DeviceId) {
	XZip_hls_accel_Config *ConfigPtr = NULL;

	int Index;

	for (Index = 0; Index < XPAR_XZIP_HLS_ACCEL_NUM_INSTANCES; Index++) {
		if (XZip_hls_accel_ConfigTable[Index].DeviceId == DeviceId) {
			ConfigPtr = &XZip_hls_accel_ConfigTable[Index];
			break;
		}
	}

	return ConfigPtr;
}

int XZip_hls_accel_Initialize(XZip_hls_accel *InstancePtr, u16 DeviceId) {
	XZip_hls_accel_Config *ConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	ConfigPtr = XZip_hls_accel_LookupConfig(DeviceId);
	if (ConfigPtr == NULL) {
		InstancePtr->IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	return XZip_hls_accel_CfgInitialize(InstancePtr, ConfigPtr);
}
#endif

#endif

