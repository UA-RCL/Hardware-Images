# Setup acknowledgement only: no replay, reset, or UART access.
connect -url tcp:127.0.0.1:3121
targets -set -filter {name == "PSU"}
set ready 0
for {set n 0} {$n<50} {incr n} {
    set words [mrd -value [lindex $argv 0] 64]
    if {[lindex $words 0]==0x4d4e4953 && [lindex $words 7]==3 &&
        [lindex $words 1]==3 && [lindex $words 33]==1} {set ready 1; break}
    after 100
}
disconnect
if {!$ready} {error "Resident application did not become ready"}
puts "MNIST resident READY; UART: run 100; no further FPGA download needed"
