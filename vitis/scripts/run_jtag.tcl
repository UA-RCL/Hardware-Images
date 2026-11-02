# Explicit board operation: xsdb run_jtag.tcl <bitstream> <psu_init.tcl> <app.elf>
# Connect only the intended target; this script resets/programs it and starts A53.
if {$argc != 3} {error "Usage: xsdb run_jtag.tcl <bitstream> <psu_init.tcl> <app.elf>"}
lassign $argv bit init elf
foreach path [list $bit $init $elf] {
    if {![file isfile $path]} {error "Missing file: $path"}
}
connect -url tcp:127.0.0.1:3121
targets -set -filter {name =~ "PSU"}
rst -system
after 3000
fpga -file $bit
source $init
psu_init
psu_ps_pl_isolation_removal
psu_ps_pl_reset_config
after 1000
targets -set -filter {name =~ "Cortex-A53 #0"}
rst -processor
dow $elf
if {[info exists ::env(MNIST_RUN_CONFIG_ADDRESS)]} {
    set cfg $::env(MNIST_RUN_CONFIG_ADDRESS)
    mwr $cfg $::env(MNIST_RUN_IMAGES)
    mwr [expr {$cfg+4}] $::env(MNIST_RUN_FLAGS)
    mwr [expr {$cfg+8}] 0
}
con
puts "MNIST resident application started; UART1 115200 8N1: run 100"
