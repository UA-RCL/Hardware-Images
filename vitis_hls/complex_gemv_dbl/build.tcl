open_project -reset complex_gemv_dbl
set_top complex_gemv_double
add_files     ./complex_gemv_dbl.cpp    -cflags "-I./"
open_solution "solution"
set_part {xczu3eg-sfvc784-2-e}
create_clock -period 1.000 -name default
set_clock_uncertainty 0.5
# config_flow -target vitis
csynth_design
# cosim_design -trace_level all -enable_dataflow_profiling
export_design -rtl verilog -format ip_catalog -description "Complex GEMV DBL" -display_name "Complex GEMV DBL"
exit
