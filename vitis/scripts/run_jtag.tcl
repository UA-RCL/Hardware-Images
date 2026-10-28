# Explicit board operation: xsdb run_jtag.tcl <bitstream> <psu_init.tcl> <app.elf>
# Connect only one target board to the local hw_server. No board run performed here.
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
con
puts "Application started; open UART1 at 115200 8N1 and enter quick"
