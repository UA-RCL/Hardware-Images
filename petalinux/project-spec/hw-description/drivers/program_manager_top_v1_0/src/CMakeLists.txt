# ==============================================================
# Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2024.2 (64-bit)
# Tool Version Limit: 2024.11
# Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
# Copyright 2022-2024 Advanced Micro Devices, Inc. All Rights Reserved.
# 
# ==============================================================
cmake_minimum_required(VERSION 3.15)
project(program_manager_top)
find_package(common)
collector_create (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}")
collect (PROJECT_LIB_SOURCES xprogram_manager_top.c)
collect (PROJECT_LIB_SOURCES xprogram_manager_top_sinit.c)
collect (PROJECT_LIB_SOURCES xprogram_manager_top_linux.c)
collect (PROJECT_LIB_SOURCES xprogram_manager_top_g.c)
collector_list (_sources PROJECT_LIB_SOURCES)
collector_create (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}")
collect (PROJECT_LIB_HEADERS xprogram_manager_top.h)
collect (PROJECT_LIB_HEADERS xprogram_manager_top_hw.h)
collector_list (_headers PROJECT_LIB_HEADERS)
file(COPY ${_headers} DESTINATION ${CMAKE_BINARY_DIR}/include)
include_directories(${CMAKE_BINARY_DIR}/include)
add_library(program_manager_top STATIC ${_sources})
set_target_properties(program_manager_top PROPERTIES LINKER_LANGUAGE C)

