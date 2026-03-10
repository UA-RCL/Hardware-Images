open_project -reset maxpool2d_prj
set_top MaxPoolingKernel
# add_files -tb ../src/hls_testbench.cpp  -cflags "-I../src"
# add_files -tb ../src/cmdlineparser.cpp  -cflags "-I../src"
# add_files -tb ../src/filter2d_sw.cpp    -cflags "-I../src"
add_files     ./maxpool2d.cpp    -cflags "-I./"
open_solution "solution"
set_part {xczu3eg-sfvc784-2-e}
create_clock -period 1.000 -name default
set_clock_uncertainty 0.5
# config_flow -target vitis
# csim_design
csynth_design
# cosim_design -trace_level all -enable_dataflow_profiling
export_design -rtl verilog -format ip_catalog -description "Maxpool2d Accelerator" -display_name "Maxpool2d"
exit
