// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2024.1 (64-bit)
// Tool Version Limit: 2024.05
// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// Copyright 2022-2024 Advanced Micro Devices, Inc. All Rights Reserved.
// 
// ==============================================================
#ifndef XCOMPLEX_GEMV_FLT_H
#define XCOMPLEX_GEMV_FLT_H

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
#include "xcomplex_gemv_flt_hw.h"

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
} XComplex_gemv_flt_Config;
#endif

typedef struct {
    u64 Control_BaseAddress;
    u32 IsReady;
} XComplex_gemv_flt;

typedef u32 word_type;

/***************** Macros (Inline Functions) Definitions *********************/
#ifndef __linux__
#define XComplex_gemv_flt_WriteReg(BaseAddress, RegOffset, Data) \
    Xil_Out32((BaseAddress) + (RegOffset), (u32)(Data))
#define XComplex_gemv_flt_ReadReg(BaseAddress, RegOffset) \
    Xil_In32((BaseAddress) + (RegOffset))
#else
#define XComplex_gemv_flt_WriteReg(BaseAddress, RegOffset, Data) \
    *(volatile u32*)((BaseAddress) + (RegOffset)) = (u32)(Data)
#define XComplex_gemv_flt_ReadReg(BaseAddress, RegOffset) \
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
int XComplex_gemv_flt_Initialize(XComplex_gemv_flt *InstancePtr, UINTPTR BaseAddress);
XComplex_gemv_flt_Config* XComplex_gemv_flt_LookupConfig(UINTPTR BaseAddress);
#else
int XComplex_gemv_flt_Initialize(XComplex_gemv_flt *InstancePtr, u16 DeviceId);
XComplex_gemv_flt_Config* XComplex_gemv_flt_LookupConfig(u16 DeviceId);
#endif
int XComplex_gemv_flt_CfgInitialize(XComplex_gemv_flt *InstancePtr, XComplex_gemv_flt_Config *ConfigPtr);
#else
int XComplex_gemv_flt_Initialize(XComplex_gemv_flt *InstancePtr, const char* InstanceName);
int XComplex_gemv_flt_Release(XComplex_gemv_flt *InstancePtr);
#endif

void XComplex_gemv_flt_Start(XComplex_gemv_flt *InstancePtr);
u32 XComplex_gemv_flt_IsDone(XComplex_gemv_flt *InstancePtr);
u32 XComplex_gemv_flt_IsIdle(XComplex_gemv_flt *InstancePtr);
u32 XComplex_gemv_flt_IsReady(XComplex_gemv_flt *InstancePtr);
void XComplex_gemv_flt_EnableAutoRestart(XComplex_gemv_flt *InstancePtr);
void XComplex_gemv_flt_DisableAutoRestart(XComplex_gemv_flt *InstancePtr);

void XComplex_gemv_flt_Set_input1(XComplex_gemv_flt *InstancePtr, u64 Data);
u64 XComplex_gemv_flt_Get_input1(XComplex_gemv_flt *InstancePtr);
void XComplex_gemv_flt_Set_input2(XComplex_gemv_flt *InstancePtr, u64 Data);
u64 XComplex_gemv_flt_Get_input2(XComplex_gemv_flt *InstancePtr);
void XComplex_gemv_flt_Set_output_r(XComplex_gemv_flt *InstancePtr, u64 Data);
u64 XComplex_gemv_flt_Get_output_r(XComplex_gemv_flt *InstancePtr);

void XComplex_gemv_flt_InterruptGlobalEnable(XComplex_gemv_flt *InstancePtr);
void XComplex_gemv_flt_InterruptGlobalDisable(XComplex_gemv_flt *InstancePtr);
void XComplex_gemv_flt_InterruptEnable(XComplex_gemv_flt *InstancePtr, u32 Mask);
void XComplex_gemv_flt_InterruptDisable(XComplex_gemv_flt *InstancePtr, u32 Mask);
void XComplex_gemv_flt_InterruptClear(XComplex_gemv_flt *InstancePtr, u32 Mask);
u32 XComplex_gemv_flt_InterruptGetEnabled(XComplex_gemv_flt *InstancePtr);
u32 XComplex_gemv_flt_InterruptGetStatus(XComplex_gemv_flt *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif
