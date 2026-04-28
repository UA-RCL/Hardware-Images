// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2024.1 (64-bit)
// Tool Version Limit: 2024.05
// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// Copyright 2022-2024 Advanced Micro Devices, Inc. All Rights Reserved.
// 
// ==============================================================
#ifndef __linux__

#include "xstatus.h"
#ifdef SDT
#include "xparameters.h"
#endif
#include "xcomplex_gemv_flt.h"

extern XComplex_gemv_flt_Config XComplex_gemv_flt_ConfigTable[];

#ifdef SDT
XComplex_gemv_flt_Config *XComplex_gemv_flt_LookupConfig(UINTPTR BaseAddress) {
	XComplex_gemv_flt_Config *ConfigPtr = NULL;

	int Index;

	for (Index = (u32)0x0; XComplex_gemv_flt_ConfigTable[Index].Name != NULL; Index++) {
		if (!BaseAddress || XComplex_gemv_flt_ConfigTable[Index].Control_BaseAddress == BaseAddress) {
			ConfigPtr = &XComplex_gemv_flt_ConfigTable[Index];
			break;
		}
	}

	return ConfigPtr;
}

int XComplex_gemv_flt_Initialize(XComplex_gemv_flt *InstancePtr, UINTPTR BaseAddress) {
	XComplex_gemv_flt_Config *ConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	ConfigPtr = XComplex_gemv_flt_LookupConfig(BaseAddress);
	if (ConfigPtr == NULL) {
		InstancePtr->IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	return XComplex_gemv_flt_CfgInitialize(InstancePtr, ConfigPtr);
}
#else
XComplex_gemv_flt_Config *XComplex_gemv_flt_LookupConfig(u16 DeviceId) {
	XComplex_gemv_flt_Config *ConfigPtr = NULL;

	int Index;

	for (Index = 0; Index < XPAR_XCOMPLEX_GEMV_FLT_NUM_INSTANCES; Index++) {
		if (XComplex_gemv_flt_ConfigTable[Index].DeviceId == DeviceId) {
			ConfigPtr = &XComplex_gemv_flt_ConfigTable[Index];
			break;
		}
	}

	return ConfigPtr;
}

int XComplex_gemv_flt_Initialize(XComplex_gemv_flt *InstancePtr, u16 DeviceId) {
	XComplex_gemv_flt_Config *ConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	ConfigPtr = XComplex_gemv_flt_LookupConfig(DeviceId);
	if (ConfigPtr == NULL) {
		InstancePtr->IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	return XComplex_gemv_flt_CfgInitialize(InstancePtr, ConfigPtr);
}
#endif

#endif

