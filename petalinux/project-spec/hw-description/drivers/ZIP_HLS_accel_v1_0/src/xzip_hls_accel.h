// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2020.2 (64-bit)
// Copyright 1986-2020 Xilinx, Inc. All Rights Reserved.
// ==============================================================
#ifndef XZIP_HLS_ACCEL_H
#define XZIP_HLS_ACCEL_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#ifndef __linux__
#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"
#include "xil_io.h"
#else
#include <stdint.h>
#include <assert.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stddef.h>
#endif
#include "xzip_hls_accel_hw.h"

/**************************** Type Definitions ******************************/
#ifdef __linux__
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
#else
typedef struct {
    u16 DeviceId;
    u32 Ctrl_BaseAddress;
} XZip_hls_accel_Config;
#endif

typedef struct {
    u64 Ctrl_BaseAddress;
    u32 IsReady;
} XZip_hls_accel;

typedef u32 word_type;

/***************** Macros (Inline Functions) Definitions *********************/
#ifndef __linux__
#define XZip_hls_accel_WriteReg(BaseAddress, RegOffset, Data) \
    Xil_Out32((BaseAddress) + (RegOffset), (u32)(Data))
#define XZip_hls_accel_ReadReg(BaseAddress, RegOffset) \
    Xil_In32((BaseAddress) + (RegOffset))
#else
#define XZip_hls_accel_WriteReg(BaseAddress, RegOffset, Data) \
    *(volatile u32*)((BaseAddress) + (RegOffset)) = (u32)(Data)
#define XZip_hls_accel_ReadReg(BaseAddress, RegOffset) \
    *(volatile u32*)((BaseAddress) + (RegOffset))

#define Xil_AssertVoid(expr)    assert(expr)
#define Xil_AssertNonvoid(expr) assert(expr)

#define XST_SUCCESS             0
#define XST_DEVICE_NOT_FOUND    2
#define XST_OPEN_DEVICE_FAILED  3
#define XIL_COMPONENT_IS_READY  1
#endif

/************************** Function Prototypes *****************************/
#ifndef __linux__
int XZip_hls_accel_Initialize(XZip_hls_accel *InstancePtr, u16 DeviceId);
XZip_hls_accel_Config* XZip_hls_accel_LookupConfig(u16 DeviceId);
int XZip_hls_accel_CfgInitialize(XZip_hls_accel *InstancePtr, XZip_hls_accel_Config *ConfigPtr);
#else
int XZip_hls_accel_Initialize(XZip_hls_accel *InstancePtr, const char* InstanceName);
int XZip_hls_accel_Release(XZip_hls_accel *InstancePtr);
#endif

void XZip_hls_accel_Start(XZip_hls_accel *InstancePtr);
u32 XZip_hls_accel_IsDone(XZip_hls_accel *InstancePtr);
u32 XZip_hls_accel_IsIdle(XZip_hls_accel *InstancePtr);
u32 XZip_hls_accel_IsReady(XZip_hls_accel *InstancePtr);
void XZip_hls_accel_EnableAutoRestart(XZip_hls_accel *InstancePtr);
void XZip_hls_accel_DisableAutoRestart(XZip_hls_accel *InstancePtr);

void XZip_hls_accel_Set_size(XZip_hls_accel *InstancePtr, u32 Data);
u32 XZip_hls_accel_Get_size(XZip_hls_accel *InstancePtr);
void XZip_hls_accel_Set_op(XZip_hls_accel *InstancePtr, u32 Data);
u32 XZip_hls_accel_Get_op(XZip_hls_accel *InstancePtr);

void XZip_hls_accel_InterruptGlobalEnable(XZip_hls_accel *InstancePtr);
void XZip_hls_accel_InterruptGlobalDisable(XZip_hls_accel *InstancePtr);
void XZip_hls_accel_InterruptEnable(XZip_hls_accel *InstancePtr, u32 Mask);
void XZip_hls_accel_InterruptDisable(XZip_hls_accel *InstancePtr, u32 Mask);
void XZip_hls_accel_InterruptClear(XZip_hls_accel *InstancePtr, u32 Mask);
u32 XZip_hls_accel_InterruptGetEnabled(XZip_hls_accel *InstancePtr);
u32 XZip_hls_accel_InterruptGetStatus(XZip_hls_accel *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif
