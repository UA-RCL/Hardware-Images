// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2020.2 (64-bit)
// Copyright 1986-2020 Xilinx, Inc. All Rights Reserved.
// ==============================================================
#ifndef __linux__

#include "xstatus.h"
#include "xparameters.h"
#include "xzip_hls_accel.h"

extern XZip_hls_accel_Config XZip_hls_accel_ConfigTable[];

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

