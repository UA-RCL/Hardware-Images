# create_project.tcl
# Simple project recreation script for fft_xdma_pl_only

# ------------------------------------------------------------
# Locate repository/project root relative to this script
# ------------------------------------------------------------
set script_dir [file normalize [file dirname [info script]]]
set origin_dir [file normalize "$script_dir/.."]

# ------------------------------------------------------------
# User settings
# ------------------------------------------------------------
set proj_name "fft_xdma_pl_only"
set part_name "xczu19eg-ffvc1760-2-i"

set bd_name  "fft_xdma_block_design"
set top_name "${bd_name}_wrapper"

# Build directory
set build_dir "$origin_dir/build"

# Source locations
set src_dir    "$origin_dir/src"
set constr_dir "$origin_dir/constraints"
set bd_dir     "$origin_dir/bd"

# ------------------------------------------------------------
# Create project
# ------------------------------------------------------------
file mkdir $build_dir

create_project $proj_name "$build_dir/$proj_name" -part $part_name -force

set proj_dir [get_property directory [current_project]]

# ------------------------------------------------------------
# Basic project properties
# ------------------------------------------------------------
set_property default_lib xil_defaultlib [current_project]
set_property simulator_language Mixed [current_project]
set_property target_language Verilog [current_project]
set_property xpm_libraries {XPM_CDC XPM_FIFO XPM_MEMORY} [current_project]

# Optional, but useful for IP cache/output organization
set_property ip_output_repo "$proj_dir/${proj_name}.cache/ip" [current_project]
set_property sim.central_dir "$proj_dir/${proj_name}.ip_user_files" [current_project]

# ------------------------------------------------------------
# Add RTL sources
# ------------------------------------------------------------
add_files -norecurse -fileset sources_1 [list \
    "$src_dir/axi_stream_v_mult.v" \
]

# This Tcl file appeared in your generated project script, but it was disabled.
# Add it only if you still want it tracked in the project.
set lane_swap_tcl "$src_dir/z7_gt_lane_swap.tcl"
if {[file exists $lane_swap_tcl]} {
    add_files -norecurse -fileset sources_1 $lane_swap_tcl
    set file_obj [get_files -of_objects [get_filesets sources_1] $lane_swap_tcl]
    set_property file_type TCL $file_obj
    set_property is_enabled false $file_obj
}

# ------------------------------------------------------------
# Create block design from Tcl
# ------------------------------------------------------------
source "$origin_dir/scripts/create_bd.tcl"

validate_bd_design
save_bd_design

set bd_file [get_files -quiet "*${bd_name}.bd"]

if {[llength $bd_file] == 0} {
    error "Could not find generated block design file: ${bd_name}.bd"
}

# ------------------------------------------------------------
# Create HDL wrapper
# ------------------------------------------------------------
set wrapper_path [make_wrapper -files $bd_file -top]
add_files -norecurse -fileset sources_1 $wrapper_path

set_property top $top_name [get_filesets sources_1]
set_property top_auto_set false [get_filesets sources_1]

# ------------------------------------------------------------
# Add constraints
# ------------------------------------------------------------
add_files -norecurse -fileset constrs_1 [list \
    "$constr_dir/z19_fft_pcie.xdc" \
]

# implementation_only.xdc was present but disabled in the generated script.
set impl_xdc "$constr_dir/implementation_only.xdc"
if {[file exists $impl_xdc]} {
    add_files -norecurse -fileset constrs_1 $impl_xdc
    set file_obj [get_files -of_objects [get_filesets constrs_1] $impl_xdc]
    set_property file_type XDC $file_obj
    set_property is_enabled false $file_obj
    set_property used_in implementation $file_obj
    set_property used_in_synthesis false $file_obj
}

# ------------------------------------------------------------
# Create synthesis and implementation runs
# ------------------------------------------------------------
if {[string equal [get_runs -quiet synth_1] ""]} {
    create_run synth_1 \
        -part $part_name \
        -flow {Vivado Synthesis 2023} \
        -strategy {Vivado Synthesis Defaults} \
        -constrset constrs_1
}

if {[string equal [get_runs -quiet impl_1] ""]} {
    create_run impl_1 \
        -part $part_name \
        -flow {Vivado Implementation 2023} \
        -strategy {Vivado Implementation Defaults} \
        -constrset constrs_1 \
        -parent_run synth_1
}

current_run -synthesis [get_runs synth_1]
current_run -implementation [get_runs impl_1]

# ------------------------------------------------------------
# Final update
# ------------------------------------------------------------
update_compile_order -fileset sources_1

puts "INFO: Project created successfully: $proj_name"
puts "INFO: Project directory: $proj_dir"
