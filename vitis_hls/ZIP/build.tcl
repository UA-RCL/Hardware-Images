open_project -reset ZIP_prj
set_top ZIP_HLS_accel
add_files zip.h
add_files zip.cpp
add_files zip_axiwrapper.cpp
add_files -tb zip_tb.cpp
open_solution "zip"
# -flow_target vivado
set_part {xczu3eg-sfvc784-2-e}
create_clock -period 1.000 -name default
set_clock_uncertainty 0.5
#source "./ZIP/zip/directives.tcl"
# csim_design
csynth_design
# cosim_design
export_design -format ip_catalog -description "ZIP" -display_name "ZIP"
