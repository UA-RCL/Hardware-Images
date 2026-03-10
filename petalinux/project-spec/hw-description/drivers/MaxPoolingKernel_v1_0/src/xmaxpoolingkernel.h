// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2024.2 (64-bit)
// Tool Version Limit: 2024.11
// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// Copyright 2022-2024 Advanced Micro Devices, Inc. All Rights Reserved.
// 
// ==============================================================
#ifndef XMAXPOOLINGKERNEL_H
#define XMAXPOOLINGKERNEL_H

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
#include "xmaxpoolingkernel_hw.h"

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
    u64 Config_BaseAddress;
} XMaxpoolingkernel_Config;
#endif

typedef struct {
    u64 Config_BaseAddress;
    u32 IsReady;
} XMaxpoolingkernel;

typedef u32 word_type;

/***************** Macros (Inline Functions) Definitions *********************/
#ifndef __linux__
#define XMaxpoolingkernel_WriteReg(BaseAddress, RegOffset, Data) \
    Xil_Out32((BaseAddress) + (RegOffset), (u32)(Data))
#define XMaxpoolingkernel_ReadReg(BaseAddress, RegOffset) \
    Xil_In32((BaseAddress) + (RegOffset))
#else
#define XMaxpoolingkernel_WriteReg(BaseAddress, RegOffset, Data) \
    *(volatile u32*)((BaseAddress) + (RegOffset)) = (u32)(Data)
#define XMaxpoolingkernel_ReadReg(BaseAddress, RegOffset) \
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
int XMaxpoolingkernel_Initialize(XMaxpoolingkernel *InstancePtr, UINTPTR BaseAddress);
XMaxpoolingkernel_Config* XMaxpoolingkernel_LookupConfig(UINTPTR BaseAddress);
#else
int XMaxpoolingkernel_Initialize(XMaxpoolingkernel *InstancePtr, u16 DeviceId);
XMaxpoolingkernel_Config* XMaxpoolingkernel_LookupConfig(u16 DeviceId);
#endif
int XMaxpoolingkernel_CfgInitialize(XMaxpoolingkernel *InstancePtr, XMaxpoolingkernel_Config *ConfigPtr);
#else
int XMaxpoolingkernel_Initialize(XMaxpoolingkernel *InstancePtr, const char* InstanceName);
int XMaxpoolingkernel_Release(XMaxpoolingkernel *InstancePtr);
#endif

void XMaxpoolingkernel_Start(XMaxpoolingkernel *InstancePtr);
u32 XMaxpoolingkernel_IsDone(XMaxpoolingkernel *InstancePtr);
u32 XMaxpoolingkernel_IsIdle(XMaxpoolingkernel *InstancePtr);
u32 XMaxpoolingkernel_IsReady(XMaxpoolingkernel *InstancePtr);
void XMaxpoolingkernel_EnableAutoRestart(XMaxpoolingkernel *InstancePtr);
void XMaxpoolingkernel_DisableAutoRestart(XMaxpoolingkernel *InstancePtr);

void XMaxpoolingkernel_Set_pool_size(XMaxpoolingkernel *InstancePtr, u32 Data);
u32 XMaxpoolingkernel_Get_pool_size(XMaxpoolingkernel *InstancePtr);
void XMaxpoolingkernel_Set_pool_stride(XMaxpoolingkernel *InstancePtr, u32 Data);
u32 XMaxpoolingkernel_Get_pool_stride(XMaxpoolingkernel *InstancePtr);
void XMaxpoolingkernel_Set_input_h(XMaxpoolingkernel *InstancePtr, u32 Data);
u32 XMaxpoolingkernel_Get_input_h(XMaxpoolingkernel *InstancePtr);
void XMaxpoolingkernel_Set_input_w(XMaxpoolingkernel *InstancePtr, u32 Data);
u32 XMaxpoolingkernel_Get_input_w(XMaxpoolingkernel *InstancePtr);

void XMaxpoolingkernel_InterruptGlobalEnable(XMaxpoolingkernel *InstancePtr);
void XMaxpoolingkernel_InterruptGlobalDisable(XMaxpoolingkernel *InstancePtr);
void XMaxpoolingkernel_InterruptEnable(XMaxpoolingkernel *InstancePtr, u32 Mask);
void XMaxpoolingkernel_InterruptDisable(XMaxpoolingkernel *InstancePtr, u32 Mask);
void XMaxpoolingkernel_InterruptClear(XMaxpoolingkernel *InstancePtr, u32 Mask);
u32 XMaxpoolingkernel_InterruptGetEnabled(XMaxpoolingkernel *InstancePtr);
u32 XMaxpoolingkernel_InterruptGetStatus(XMaxpoolingkernel *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif
