// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2024.2 (64-bit)
// Tool Version Limit: 2024.11
// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// Copyright 2022-2024 Advanced Micro Devices, Inc. All Rights Reserved.
// 
// ==============================================================
#ifndef XCONV2D_H
#define XCONV2D_H

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
#include "xconv2d_hw.h"

/**************************** Type Definitions ******************************/
#ifdef __linux__
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
#else
typedef struct {
#ifdef SDT
    char *Name;
#else
    u16 DeviceId;
#endif
    u64 Control_BaseAddress;
} XConv2d_Config;
#endif

typedef struct {
    u64 Control_BaseAddress;
    u32 IsReady;
} XConv2d;

typedef u32 word_type;

/***************** Macros (Inline Functions) Definitions *********************/
#ifndef __linux__
#define XConv2d_WriteReg(BaseAddress, RegOffset, Data) \
    Xil_Out32((BaseAddress) + (RegOffset), (u32)(Data))
#define XConv2d_ReadReg(BaseAddress, RegOffset) \
    Xil_In32((BaseAddress) + (RegOffset))
#else
#define XConv2d_WriteReg(BaseAddress, RegOffset, Data) \
    *(volatile u32*)((BaseAddress) + (RegOffset)) = (u32)(Data)
#define XConv2d_ReadReg(BaseAddress, RegOffset) \
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
#ifdef SDT
int XConv2d_Initialize(XConv2d *InstancePtr, UINTPTR BaseAddress);
XConv2d_Config* XConv2d_LookupConfig(UINTPTR BaseAddress);
#else
int XConv2d_Initialize(XConv2d *InstancePtr, u16 DeviceId);
XConv2d_Config* XConv2d_LookupConfig(u16 DeviceId);
#endif
int XConv2d_CfgInitialize(XConv2d *InstancePtr, XConv2d_Config *ConfigPtr);
#else
int XConv2d_Initialize(XConv2d *InstancePtr, const char* InstanceName);
int XConv2d_Release(XConv2d *InstancePtr);
#endif

void XConv2d_Start(XConv2d *InstancePtr);
u32 XConv2d_IsDone(XConv2d *InstancePtr);
u32 XConv2d_IsIdle(XConv2d *InstancePtr);
u32 XConv2d_IsReady(XConv2d *InstancePtr);
void XConv2d_EnableAutoRestart(XConv2d *InstancePtr);
void XConv2d_DisableAutoRestart(XConv2d *InstancePtr);

void XConv2d_Set_conv2d_bias(XConv2d *InstancePtr, u32 Data);
u32 XConv2d_Get_conv2d_bias(XConv2d *InstancePtr);
void XConv2d_Set_stride(XConv2d *InstancePtr, u32 Data);
u32 XConv2d_Get_stride(XConv2d *InstancePtr);
void XConv2d_Set_padding(XConv2d *InstancePtr, u32 Data);
u32 XConv2d_Get_padding(XConv2d *InstancePtr);
void XConv2d_Set_input_h(XConv2d *InstancePtr, u32 Data);
u32 XConv2d_Get_input_h(XConv2d *InstancePtr);
void XConv2d_Set_input_w(XConv2d *InstancePtr, u32 Data);
u32 XConv2d_Get_input_w(XConv2d *InstancePtr);
void XConv2d_Set_kernel_h(XConv2d *InstancePtr, u32 Data);
u32 XConv2d_Get_kernel_h(XConv2d *InstancePtr);
void XConv2d_Set_kernel_w(XConv2d *InstancePtr, u32 Data);
u32 XConv2d_Get_kernel_w(XConv2d *InstancePtr);

void XConv2d_InterruptGlobalEnable(XConv2d *InstancePtr);
void XConv2d_InterruptGlobalDisable(XConv2d *InstancePtr);
void XConv2d_InterruptEnable(XConv2d *InstancePtr, u32 Mask);
void XConv2d_InterruptDisable(XConv2d *InstancePtr, u32 Mask);
void XConv2d_InterruptClear(XConv2d *InstancePtr, u32 Mask);
u32 XConv2d_InterruptGetEnabled(XConv2d *InstancePtr);
u32 XConv2d_InterruptGetStatus(XConv2d *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif
