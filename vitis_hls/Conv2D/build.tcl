open_project -reset conv_filter_prj
set_top Filter2DKernel
# add_files -tb ../src/hls_testbench.cpp  -cflags "-I../src"
# add_files -tb ../src/cmdlineparser.cpp  -cflags "-I../src"
# add_files -tb ../src/filter2d_sw.cpp    -cflags "-I../src"
add_files     ./filter2d_hw.cpp    -cflags "-I./"
add_files -tb ./filter2d_hw_tb.cpp -cflags "-I./"
open_solution "solution"
set_part {xc7z020clg484-1}
create_clock -period 1.000 -name default
set_clock_uncertainty 1
# config_flow -target vitis
csim_design
csynth_design
# cosim_design -trace_level all -enable_dataflow_profiling
export_design -rtl verilog -format ip_catalog -description "2D Convolution" -display_name "Conv2D"
exit
