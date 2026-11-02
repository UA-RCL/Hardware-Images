# Read-only target enumeration; no reset, download or programming.
connect -url tcp:127.0.0.1:3121
puts [targets]
puts [jtag targets]
puts [targets -target-properties]
disconnect
