# Vivado 2024.2. Reuse the checked-in AUP-ZU3 board's PS preset and FPGA part.
set hw [file normalize [file join [file dirname [info script]] ..]]
set project_dir [file join $hw build vivado]
if {[info exists ::env(RANC_PROJECT_DIR)]} {set project_dir [file normalize $::env(RANC_PROJECT_DIR)]}
create_project ranc_aup $project_dir -part xczu3eg-sfvc784-2-e -force
set_property target_language Verilog [current_project]
add_files [glob [file join $hw rtl *.v]]
update_compile_order -fileset sources_1
create_bd_design ranc_system
set zynq_ultra_ps_e_0 [create_bd_cell -type ip -vlnv xilinx.com:ip:zynq_ultra_ps_e:3.5 ps]
source [file join $hw vivado aup_ps_preset.tcl]
set_property -dict [list CONFIG.PSU__USE__M_AXI_GP2 {1} CONFIG.PSU__USE__S_AXI_GP2 {1} \
  CONFIG.PSU__USE__S_AXI_GP3 {0} CONFIG.PSU__USE__S_AXI_GP4 {0} CONFIG.PSU__USE__S_AXI_GP5 {0} \
  CONFIG.PSU__USE__FABRIC__RST {1} CONFIG.PSU__USE__IRQ0 {1}] $zynq_ultra_ps_e_0
set reset [create_bd_cell -type ip -vlnv xilinx.com:ip:proc_sys_reset:5.0 reset]
set one [create_bd_cell -type ip -vlnv xilinx.com:ip:xlconstant:1.1 one]
set zero [create_bd_cell -type ip -vlnv xilinx.com:ip:xlconstant:1.1 zero]
set_property CONFIG.CONST_VAL 0 $zero
# Simple DMA moves spikes through DDR; no scatter-gather descriptor ring is used.
set dma [create_bd_cell -type ip -vlnv xilinx.com:ip:axi_dma:7.1 dma]
set_property -dict [list CONFIG.c_include_sg {0} CONFIG.c_include_mm2s_dre {1} CONFIG.c_include_s2mm_dre {1} \
  CONFIG.c_m_axis_mm2s_tdata_width {32} CONFIG.c_s_axis_s2mm_tdata_width {32} CONFIG.c_sg_length_width {23}] $dma
# GPIO channel 1 writes wrapper control; channel 2 reads tick count and sticky status.
set gpio [create_bd_cell -type ip -vlnv xilinx.com:ip:axi_gpio:2.0 control]
set_property -dict [list CONFIG.C_GPIO_WIDTH {32} CONFIG.C_ALL_OUTPUTS {1} CONFIG.C_IS_DUAL {1} \
  CONFIG.C_GPIO2_WIDTH {32} CONFIG.C_ALL_INPUTS_2 {1}] $gpio
set ctrl [create_bd_cell -type ip -vlnv xilinx.com:ip:smartconnect:1.0 ctrl_axi]
set_property -dict [list CONFIG.NUM_SI {1} CONFIG.NUM_MI {2}] $ctrl
set mem [create_bd_cell -type ip -vlnv xilinx.com:ip:smartconnect:1.0 ddr_axi]
set_property -dict [list CONFIG.NUM_SI {2} CONFIG.NUM_MI {1}] $mem
set ranc [create_bd_cell -type module -reference ranc_mnist_axis ranc]
# Tutorial MNIST mapping; CPU issues one bounded hardware step per image.
set_property -dict [list CONFIG.MEMORY_FILEPATH "[file join $hw data]/" CONFIG.TICK_CYCLES {100000}] $ranc
set irq [create_bd_cell -type ip -vlnv xilinx.com:ip:xlconcat:2.1 irq]
set_property CONFIG.NUM_PORTS 8 $irq
connect_bd_intf_net [get_bd_intf_pins ps/M_AXI_HPM0_LPD] [get_bd_intf_pins ctrl_axi/S00_AXI]
connect_bd_intf_net [get_bd_intf_pins ctrl_axi/M00_AXI] [get_bd_intf_pins dma/S_AXI_LITE]
connect_bd_intf_net [get_bd_intf_pins ctrl_axi/M01_AXI] [get_bd_intf_pins control/S_AXI]
connect_bd_intf_net [get_bd_intf_pins dma/M_AXI_MM2S] [get_bd_intf_pins ddr_axi/S00_AXI]
connect_bd_intf_net [get_bd_intf_pins dma/M_AXI_S2MM] [get_bd_intf_pins ddr_axi/S01_AXI]
connect_bd_intf_net [get_bd_intf_pins ddr_axi/M00_AXI] [get_bd_intf_pins ps/S_AXI_HP0_FPD]
connect_bd_intf_net [get_bd_intf_pins dma/M_AXIS_MM2S] [get_bd_intf_pins ranc/S_AXIS]
connect_bd_intf_net [get_bd_intf_pins ranc/M_AXIS] [get_bd_intf_pins dma/S_AXIS_S2MM]
connect_bd_net [get_bd_pins control/gpio_io_o] [get_bd_pins ranc/control]
connect_bd_net [get_bd_pins ranc/status] [get_bd_pins control/gpio2_io_i]
connect_bd_net [get_bd_pins ps/pl_clk0] [get_bd_pins reset/slowest_sync_clk] \
  [get_bd_pins ps/maxihpm0_lpd_aclk] [get_bd_pins ps/saxihp0_fpd_aclk] \
  [get_bd_pins ctrl_axi/aclk] [get_bd_pins ddr_axi/aclk] [get_bd_pins control/s_axi_aclk] \
  [get_bd_pins dma/s_axi_lite_aclk] [get_bd_pins dma/m_axi_mm2s_aclk] [get_bd_pins dma/m_axi_s2mm_aclk] [get_bd_pins ranc/clk]
connect_bd_net [get_bd_pins ps/pl_resetn0] [get_bd_pins reset/ext_reset_in] [get_bd_pins reset/aux_reset_in]
connect_bd_net [get_bd_pins one/dout] [get_bd_pins reset/dcm_locked]
connect_bd_net [get_bd_pins zero/dout] [get_bd_pins reset/mb_debug_sys_rst]
connect_bd_net [get_bd_pins reset/peripheral_aresetn] [get_bd_pins ctrl_axi/aresetn] \
  [get_bd_pins ddr_axi/aresetn] [get_bd_pins control/s_axi_aresetn] [get_bd_pins dma/axi_resetn] [get_bd_pins ranc/resetn]
connect_bd_net [get_bd_pins dma/mm2s_introut] [get_bd_pins irq/In0]
connect_bd_net [get_bd_pins dma/s2mm_introut] [get_bd_pins irq/In1]
connect_bd_net [get_bd_pins ranc/tick_irq] [get_bd_pins irq/In2]
for {set i 3} {$i<8} {incr i} {connect_bd_net [get_bd_pins zero/dout] [get_bd_pins irq/In$i]}
connect_bd_net [get_bd_pins irq/dout] [get_bd_pins ps/pl_ps_irq0]
# This bare-metal application uses these register addresses.
assign_bd_address -offset 0x80000000 -range 64K -target_address_space [get_bd_addr_spaces ps/Data] [get_bd_addr_segs dma/S_AXI_LITE/Reg] -force
assign_bd_address -offset 0x80010000 -range 64K -target_address_space [get_bd_addr_spaces ps/Data] [get_bd_addr_segs control/S_AXI/Reg] -force
assign_bd_address
validate_bd_design
save_bd_design
generate_target all [get_files ranc_system.bd]
make_wrapper -files [get_files ranc_system.bd] -top
add_files [file join $project_dir ranc_aup.gen sources_1 bd ranc_system hdl ranc_system_wrapper.v]
set_property top ranc_system_wrapper [current_fileset]
update_compile_order -fileset sources_1
write_bd_tcl -force [file join $hw build ranc_system_bd.tcl]
report_property [get_bd_cells ps] -file [file join $hw build ps_properties.txt]
write_hw_platform -fixed -force -file [file join $hw build ranc_aup_hw.xsa]
puts "PASS: AUP Zynq + DMA + RANC block design validated and generated"
close_project
