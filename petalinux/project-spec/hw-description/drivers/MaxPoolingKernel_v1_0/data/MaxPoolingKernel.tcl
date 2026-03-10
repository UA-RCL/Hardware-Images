# ==============================================================
# Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2024.2 (64-bit)
# Tool Version Limit: 2024.11
# Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
# Copyright 2022-2024 Advanced Micro Devices, Inc. All Rights Reserved.
# 
# ==============================================================
proc generate {drv_handle} {
    xdefine_include_file $drv_handle "xparameters.h" "XMaxpoolingkernel" \
        "NUM_INSTANCES" \
        "DEVICE_ID" \
        "C_S_AXI_CONFIG_BASEADDR" \
        "C_S_AXI_CONFIG_HIGHADDR"

    xdefine_config_file $drv_handle "xmaxpoolingkernel_g.c" "XMaxpoolingkernel" \
        "DEVICE_ID" \
        "C_S_AXI_CONFIG_BASEADDR"

    xdefine_canonical_xpars $drv_handle "xparameters.h" "XMaxpoolingkernel" \
        "DEVICE_ID" \
        "C_S_AXI_CONFIG_BASEADDR" \
        "C_S_AXI_CONFIG_HIGHADDR"
}

