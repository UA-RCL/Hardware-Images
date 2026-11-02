# ABI v3: optional mailbox submission; never reset or reprogram hardware here.
if {$argc ni {1 4}} {error "Usage: check_board.tcl <result address> ?<config address> <images> <flags>?"}
proc u64 {words index} {
    expr {wide([lindex $words $index]) | (wide([lindex $words [expr {$index+1}]])<<32)}
}
connect -url tcp:127.0.0.1:3121
targets -set -filter {name == "PSU"}
set address [lindex $argv 0]
set initial [mrd -value $address 64]
if {[lindex $initial 0]!=0x4d4e4953 || [lindex $initial 7]!=3 || [lindex $initial 33]!=1} {
    error "No matching resident app; run verify_board.sh --setup-only"
}
if {[lindex $initial 1]==0 || [lindex $initial 1]==2} {error "App busy or failed; wait or explicitly --reload"}
# A moving heartbeat distinguishes a running server from stale DDR contents.
after 100
set live [mrd -value $address 64]
if {[lindex $initial 35]==[lindex $live 35]} {error "Resident app not responding; --reload required"}
set request -1
if {$argc==4} {
    lassign $argv address cfg images flags
    set request [expr {([mrd -value [expr {$cfg+8}]]+1)&0xffffffff}]
    # XSDB mwr values are explicitly hexadecimal; mrd -value returns decimal.
    mwr $cfg [format 0x%x $images]
    mwr [expr {$cfg+4}] [format 0x%x $flags]
    mwr [expr {$cfg+8}] [format 0x%x $request]
}
set passed 0
for {set attempt 0} {$attempt<120} {incr attempt} {
    set values [mrd -value $address 64]
    puts "BOARD_RESULT $values"
    lassign $values magic state tick correct classified images status version
    if {$magic!=0x4d4e4953 || $version!=3} {error "Wrong result-block ABI"}
    if {$request>=0 && [lindex $values 32]!=$request} {after 100; continue}
    if {$state==2} {error "Physical MNIST failed at tick $tick, status=$status"}
    if {$state==1} {
        if {$images<1 || $images>10000 || $tick!=$images+1 || $classified!=$images ||
            $correct>$images || ($status&0xff)!=0 || ($status>>16)!=[lindex $values 31]} {
            error "Invalid final physical result"
        }
        set application [u64 $values 8]; set test [u64 $values 10]
        set latency [u64 $values 12]; set amortized [u64 $values 14]
        set setup [u64 $values 16]
        if {$test<=0 || $application<$test || $latency<=0 || $amortized!=$test/$images} {
            error "Invalid application timer readings"
        }
        puts "PHYSICAL_MNIST PASS correct=$correct total=$classified ticks=$tick"
        puts "PHYSICAL_MNIST_TIMING application_us=$application test_us=$test setup_us=$setup avg_image_latency_us=$latency amortized_us_per_image=$amortized first_image_latency_us=[u64 $values 24] min_image_latency_us=[u64 $values 26] max_image_latency_us=[u64 $values 28] counter_hz=[u64 $values 22]"
        set passed 1
        break
    }
    after 200
}
disconnect
if {!$passed} {error "Physical verification timeout (or waiting for UART selection)"}
