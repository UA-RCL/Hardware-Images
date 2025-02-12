############################################################
## This file is generated automatically by Vitis HLS.
## Please DO NOT edit it.
## Copyright 1986-2020 Xilinx, Inc. All Rights Reserved.
############################################################
open_project -reset mmult
set_top HLS_accel
add_files matrix_multiply.h
add_files matrix_multiply.cpp
add_files matrix_multiply_axiwrapper.cpp
add_files -tb matrix_multiply_tb.cpp
open_solution -reset solution1
# Zedboard
# set_part {xc7z020clg484-1}
# Zynq Ultrascale+ ZCU102
set_part {xc7z020clg484-1}
create_clock -period 1.000 -name default
set_clock_uncertainty 1
#source "./strm_enq/strm_enq/directives.tcl"
csim_design
csynth_design
#cosim_design
export_design -rtl verilog -format ip_catalog -description "Complex MMULT IP" -display_name "complex_mmult"
