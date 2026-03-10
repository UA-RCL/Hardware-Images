open_project -reset conv_filter_prj
set_top conv2d
add_files -tb ./conv2d_tb.cpp -cflags "-I./"
add_files     ./conv2d.cpp    -cflags "-I./"
open_solution "solution"
set_part {xczu3eg-sfvc784-2-e}
create_clock -period 1.000 -name default
set_clock_uncertainty 0.5
# config_flow -target vitis
csim_design
csynth_design
# cosim_design -trace_level all -enable_dataflow_profiling
export_design -rtl verilog -format ip_catalog -description "2D Convolution" -display_name "Conv2D"
exit
