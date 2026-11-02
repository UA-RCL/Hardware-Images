# Upload one bitmap to the resident server. No PS reset, FPGA load or ELF download.
if {$argc!=4} {error "Usage: image_request.tcl result_addr config_addr bitmap_addr words_file"}
lassign $argv result cfg bitmap path
connect -url tcp:127.0.0.1:3121
targets -set -filter {name == "PSU"}
set initial [mrd -value $result 64]
if {[lindex $initial 0]!=0x4d4e4953 || [lindex $initial 7]!=3 ||
    [lindex $initial 33]!=1 || [lindex $initial 61]!=0x494d4731} {
    error "Load the custom-image-capable ELF using verify_board.sh --setup-only"
}
if {[lindex $initial 1] in {0 2}} {error "Resident app busy or failed"}
after 100
set live [mrd -value $result 64]
if {[lindex $live 35]==[lindex $initial 35]} {error "No live resident app; reload required"}
set f [open $path r]; set words [split [string trim [read $f]]]; close $f
if {[llength $words]!=32} {error "Require 32 packed bitmap words"}
set index 0
foreach word $words {mwr [expr {$bitmap+4*$index}] $word; incr index}
set request [expr {([mrd -value [expr {$cfg+8}]]+1)&0xffffffff}]
mwr $cfg 1
mwr [expr {$cfg+4}] 4
# Publish the request last, after all payload writes have completed.
mwr [expr {$cfg+8}] [format 0x%x $request]
set passed 0
for {set n 0} {$n<50} {incr n} {
    set values [mrd -value $result 64]
    if {[lindex $values 32]==$request} {
        set state [lindex $values 1]
        if {$state in {2 4}} {error "Custom-image request failed/rejected: $values"}
        if {$state==1} {
            if {[lindex $values 36]!=1 || ([lindex $values 6]&0xff)!=0 ||
                ([lindex $values 6]>>16)!=[lindex $values 31]} {error "Invalid custom-image completion"}
            puts "CUSTOM_RESULT $values"
            set passed 1; break
        }
    }
    after 100
}
disconnect
if {!$passed} {error "Custom-image request timeout"}
