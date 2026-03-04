set_property BITSTREAM.GENERAL.COMPRESS TRUE [current_design]

# PCIe reference clock - MGTREFCLK0 on BANK 128 (100 MHz from Orin NX)
set_property PACKAGE_PIN AB34 [get_ports pcie_ref_clk_p]
create_clock -period 10.000 [get_ports pcie_ref_clk_p]

# PCIe reset - active low
set_property PACKAGE_PIN K14 [get_ports pcie_rstn]
set_property IOSTANDARD LVCMOS12 [get_ports pcie_rstn]

# PCIe clock request - active low output
set_property PACKAGE_PIN H14 [get_ports pcie_clkreq]
set_property IOSTANDARD LVCMOS33 [get_ports pcie_clkreq]

set_property C_CLK_INPUT_FREQ_HZ 250000000 [get_debug_cores dbg_hub]
set_property C_ENABLE_CLK_DIVIDER false [get_debug_cores dbg_hub]
set_property C_USER_SCAN_CHAIN 1 [get_debug_cores dbg_hub]
connect_debug_port dbg_hub/clk [get_nets fft_xdma_block_design_i/xdma_0_axi_aclk]


#