# Vitis HLS build for the PE-array program manager.
#
# Invoked by ../build_IPs.sh, which cd's into this directory, runs
# `vitis_hls build.tcl`, then unzips solution1/impl/export.zip into
# ../IPs/program_manager/.
#
# src/, include/ and test/ in this directory are the design sources for this
# project -- edit them here.

open_project -reset program_manager_prj
set_top program_manager_top

# Grid geometry comes from the environment so `make ip GRID_DIM=n` resizes the
# array without editing sources; layout_config.h supplies the defaults when
# unset. PE_FLAT inlines the PE back into the manager (see pe_unit.hpp).
set cflags "-Iinclude"
if { [info exists env(GRID_DIM)] } {
    append cflags " -DDIM_X=$env(GRID_DIM) -DDIM_Y=$env(GRID_DIM)"
}
if { [info exists env(MAX_BANKS)] } {
    append cflags " -DMAX_BANKS=$env(MAX_BANKS)"
}
if { [info exists env(PE_FLAT)] } {
    append cflags " -DPE_FLAT"
}
puts ">> cflags: $cflags"

add_files src/program_manager.cpp -cflags $cflags
add_files -tb test/main_csim.cpp  -cflags $cflags

# -flow_target vivado packages for the IP catalog (a plain Vivado block design),
# as opposed to the Vitis .xo flow used for the Alveo target.
open_solution -reset "solution1" -flow_target vivado

# VCU128: xcvu37p, 100 MHz -- must match ddr4_0/addn_ui_clkout1, the peripheral
# clock driving the AXI-Lite slaves in VCU128_microblaze_bd.tcl.
set_part {xcvu37p-fsvh2892-2L-e}
create_clock -period 10.0 -name default

puts ">> RUNNING C-SIMULATION"
csim_design

puts ">> RUNNING SYNTHESIS"
csynth_design

# NOTE: no cosim_design. C/RTL co-simulation drives the RTL through
# ap_start/ap_done; this top is ap_ctrl_none with a free-running while(1), so
# cosim has no transaction boundary to wait on and hangs.

puts ">> EXPORTING IP CATALOG"
# Default vendor/library gives VLNV xilinx.com:hls:program_manager_top:1.0,
# which is what VCU128_microblaze_bd.tcl instantiates.
export_design -format ip_catalog -ipname program_manager_top -version 1.0

puts ">> HLS BUILD COMPLETE"
puts ">> Register map: program_manager_prj/solution1/impl/ip/drivers/program_manager_top_v1_0/src/xprogram_manager_top_hw.h"
puts ">> Keep the offsets in recipes-apps/pm-app/files/pm-app.c in step with it."
puts ">> If GRID_DIM changed, run `make layout-header` to regenerate pm_layout.h."
exit
