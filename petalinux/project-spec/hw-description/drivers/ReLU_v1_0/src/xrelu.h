// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2024.2 (64-bit)
// Tool Version Limit: 2024.11
// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// Copyright 2022-2024 Advanced Micro Devices, Inc. All Rights Reserved.
// 
// ==============================================================
#ifndef XRELU_H
#define XRELU_H

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
#include "xrelu_hw.h"

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
} XRelu_Config;
#endif

typedef struct {
    u64 Control_BaseAddress;
    u32 IsReady;
} XRelu;

typedef u32 word_type;

/***************** Macros (Inline Functions) Definitions *********************/
#ifndef __linux__
#define XRelu_WriteReg(BaseAddress, RegOffset, Data) \
    Xil_Out32((BaseAddress) + (RegOffset), (u32)(Data))
#define XRelu_ReadReg(BaseAddress, RegOffset) \
    Xil_In32((BaseAddress) + (RegOffset))
#else
#define XRelu_WriteReg(BaseAddress, RegOffset, Data) \
    *(volatile u32*)((BaseAddress) + (RegOffset)) = (u32)(Data)
#define XRelu_ReadReg(BaseAddress, RegOffset) \
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
int XRelu_Initialize(XRelu *InstancePtr, UINTPTR BaseAddress);
XRelu_Config* XRelu_LookupConfig(UINTPTR BaseAddress);
#else
int XRelu_Initialize(XRelu *InstancePtr, u16 DeviceId);
XRelu_Config* XRelu_LookupConfig(u16 DeviceId);
#endif
int XRelu_CfgInitialize(XRelu *InstancePtr, XRelu_Config *ConfigPtr);
#else
int XRelu_Initialize(XRelu *InstancePtr, const char* InstanceName);
int XRelu_Release(XRelu *InstancePtr);
#endif

void XRelu_Start(XRelu *InstancePtr);
u32 XRelu_IsDone(XRelu *InstancePtr);
u32 XRelu_IsIdle(XRelu *InstancePtr);
u32 XRelu_IsReady(XRelu *InstancePtr);
void XRelu_EnableAutoRestart(XRelu *InstancePtr);
void XRelu_DisableAutoRestart(XRelu *InstancePtr);

void XRelu_Set_size(XRelu *InstancePtr, u32 Data);
u32 XRelu_Get_size(XRelu *InstancePtr);

void XRelu_InterruptGlobalEnable(XRelu *InstancePtr);
void XRelu_InterruptGlobalDisable(XRelu *InstancePtr);
void XRelu_InterruptEnable(XRelu *InstancePtr, u32 Mask);
void XRelu_InterruptDisable(XRelu *InstancePtr, u32 Mask);
void XRelu_InterruptClear(XRelu *InstancePtr, u32 Mask);
u32 XRelu_InterruptGetEnabled(XRelu *InstancePtr);
u32 XRelu_InterruptGetStatus(XRelu *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif
