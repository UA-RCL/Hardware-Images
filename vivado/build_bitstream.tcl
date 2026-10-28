set hw [file normalize [file join [file dirname [info script]] ..]]
set project_dir [file join $hw build vivado]
if {[info exists ::env(RANC_PROJECT_DIR)]} {set project_dir [file normalize $::env(RANC_PROJECT_DIR)]}
open_project [file join $project_dir ranc_aup.xpr]
generate_target all [get_files ranc_system.bd]
if {![llength [get_runs -quiet ranc_system_ranc_0_synth_1]]} {
    create_ip_run [get_files ranc_system.bd]
}
# Module-reference OOC runs can retain a checkpoint after an external RTL edit.
# Invalidate it explicitly when RTL or core initialization changed.
set ranc_run [get_runs ranc_system_ranc_0_synth_1]
set checkpoint [file join [get_property DIRECTORY $ranc_run] ranc_system_ranc_0.dcp]
set refresh [expr {![file exists $checkpoint]}]
foreach source [concat [glob [file join $hw rtl *.v]] [glob [file join $hw data *.mem]]] {
    if {![file exists $checkpoint] || [file mtime $source] > [file mtime $checkpoint]} {
        set refresh 1
    }
}
if {$refresh} {
    reset_run $ranc_run
    reset_run synth_1
    puts "Refreshing RANC synthesis checkpoint after source changes"
}
# A previous interrupted implementation is also safe to restart.
if {[get_property PROGRESS [get_runs impl_1]] ne "100%"} {reset_run impl_1}
set stop_file [file join [get_property DIRECTORY [get_runs impl_1]] .stop.rst]
if {[file exists $stop_file]} {file delete $stop_file}
launch_runs synth_1 -jobs 4
wait_on_run synth_1
if {[get_property PROGRESS [get_runs synth_1]] ne "100%"} {error "Synthesis failed"}
open_run synth_1
report_utilization -file [file join $hw build utilization_synth.rpt]
report_timing_summary -file [file join $hw build timing_synth.rpt]
close_design
launch_runs impl_1 -to_step write_bitstream -jobs 4
wait_on_run impl_1
if {[get_property PROGRESS [get_runs impl_1]] ne "100%"} {error "Implementation failed"}
open_run impl_1
report_utilization -file [file join $hw build utilization_impl.rpt]
report_timing_summary -file [file join $hw build timing_impl.rpt]
set slack [get_property SLACK [get_timing_paths -delay_type max -max_paths 1]]
set hold_slack [get_property SLACK [get_timing_paths -delay_type min -max_paths 1]]
if {$slack < 0} {error "Timing not met: WNS=$slack"}
if {$hold_slack < 0} {error "Hold timing not met: WHS=$hold_slack"}
set blackboxes [get_cells -hierarchical -filter {IS_BLACKBOX == 1}]
if {[llength $blackboxes]} {error "Unresolved black boxes: $blackboxes"}
write_hw_platform -fixed -include_bit -force -file [file join $hw build ranc_aup.xsa]
exec python3 [file join $hw scripts share_hardware.py] --stage
puts "PASS: bitstream and XSA exported, WNS=$slack WHS=$hold_slack"
