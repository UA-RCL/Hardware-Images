// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2024.2 (64-bit)
// Tool Version Limit: 2024.11
// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// Copyright 2022-2024 Advanced Micro Devices, Inc. All Rights Reserved.
// 
// ==============================================================
#ifndef XPROGRAM_MANAGER_TOP_H
#define XPROGRAM_MANAGER_TOP_H

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
#include "xprogram_manager_top_hw.h"

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
} XProgram_manager_top_Config;
#endif

typedef struct {
    u64 Control_BaseAddress;
    u32 IsReady;
} XProgram_manager_top;

typedef u32 word_type;

/***************** Macros (Inline Functions) Definitions *********************/
#ifndef __linux__
#define XProgram_manager_top_WriteReg(BaseAddress, RegOffset, Data) \
    Xil_Out32((BaseAddress) + (RegOffset), (u32)(Data))
#define XProgram_manager_top_ReadReg(BaseAddress, RegOffset) \
    Xil_In32((BaseAddress) + (RegOffset))
#else
#define XProgram_manager_top_WriteReg(BaseAddress, RegOffset, Data) \
    *(volatile u32*)((BaseAddress) + (RegOffset)) = (u32)(Data)
#define XProgram_manager_top_ReadReg(BaseAddress, RegOffset) \
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
int XProgram_manager_top_Initialize(XProgram_manager_top *InstancePtr, UINTPTR BaseAddress);
XProgram_manager_top_Config* XProgram_manager_top_LookupConfig(UINTPTR BaseAddress);
#else
int XProgram_manager_top_Initialize(XProgram_manager_top *InstancePtr, u16 DeviceId);
XProgram_manager_top_Config* XProgram_manager_top_LookupConfig(u16 DeviceId);
#endif
int XProgram_manager_top_CfgInitialize(XProgram_manager_top *InstancePtr, XProgram_manager_top_Config *ConfigPtr);
#else
int XProgram_manager_top_Initialize(XProgram_manager_top *InstancePtr, const char* InstanceName);
int XProgram_manager_top_Release(XProgram_manager_top *InstancePtr);
#endif


void XProgram_manager_top_Set_mode(XProgram_manager_top *InstancePtr, u32 Data);
u32 XProgram_manager_top_Get_mode(XProgram_manager_top *InstancePtr);
void XProgram_manager_top_Set_target_mode(XProgram_manager_top *InstancePtr, u32 Data);
u32 XProgram_manager_top_Get_target_mode(XProgram_manager_top *InstancePtr);
void XProgram_manager_top_Set_pe_x(XProgram_manager_top *InstancePtr, u32 Data);
u32 XProgram_manager_top_Get_pe_x(XProgram_manager_top *InstancePtr);
void XProgram_manager_top_Set_pe_y(XProgram_manager_top *InstancePtr, u32 Data);
u32 XProgram_manager_top_Get_pe_y(XProgram_manager_top *InstancePtr);
void XProgram_manager_top_Set_cluster_id(XProgram_manager_top *InstancePtr, u32 Data);
u32 XProgram_manager_top_Get_cluster_id(XProgram_manager_top *InstancePtr);
void XProgram_manager_top_Set_bank_id(XProgram_manager_top *InstancePtr, u32 Data);
u32 XProgram_manager_top_Get_bank_id(XProgram_manager_top *InstancePtr);
void XProgram_manager_top_Set_cycles(XProgram_manager_top *InstancePtr, u32 Data);
u32 XProgram_manager_top_Get_cycles(XProgram_manager_top *InstancePtr);
void XProgram_manager_top_Set_valid_signal(XProgram_manager_top *InstancePtr, u32 Data);
u32 XProgram_manager_top_Get_valid_signal(XProgram_manager_top *InstancePtr);
u32 XProgram_manager_top_Get_busy_mask_out(XProgram_manager_top *InstancePtr);
u32 XProgram_manager_top_Get_busy_mask_out_vld(XProgram_manager_top *InstancePtr);
u32 XProgram_manager_top_Get_busy_map_out_bits_BaseAddress(XProgram_manager_top *InstancePtr);
u32 XProgram_manager_top_Get_busy_map_out_bits_HighAddress(XProgram_manager_top *InstancePtr);
u32 XProgram_manager_top_Get_busy_map_out_bits_TotalBytes(XProgram_manager_top *InstancePtr);
u32 XProgram_manager_top_Get_busy_map_out_bits_BitWidth(XProgram_manager_top *InstancePtr);
u32 XProgram_manager_top_Get_busy_map_out_bits_Depth(XProgram_manager_top *InstancePtr);
u32 XProgram_manager_top_Write_busy_map_out_bits_Words(XProgram_manager_top *InstancePtr, int offset, word_type *data, int length);
u32 XProgram_manager_top_Read_busy_map_out_bits_Words(XProgram_manager_top *InstancePtr, int offset, word_type *data, int length);
u32 XProgram_manager_top_Write_busy_map_out_bits_Bytes(XProgram_manager_top *InstancePtr, int offset, char *data, int length);
u32 XProgram_manager_top_Read_busy_map_out_bits_Bytes(XProgram_manager_top *InstancePtr, int offset, char *data, int length);

#ifdef __cplusplus
}
#endif

#endif
