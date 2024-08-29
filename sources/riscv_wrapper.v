//Copyright 1986-2020 Xilinx, Inc. All Rights Reserved.
//--------------------------------------------------------------------------------
//Tool Version: Vivado v.2020.2 (lin64) Build 3064766 Wed Nov 18 09:12:47 MST 2020
//Date        : Mon Jan 29 17:13:57 2024
//Host        : engr-rcl17g.catnet.arizona.edu running 64-bit Ubuntu 22.04.3 LTS
//Command     : generate_target riscv_wrapper.bd
//Design      : riscv_wrapper
//Purpose     : IP block netlist
//--------------------------------------------------------------------------------
`timescale 1 ps / 1 ps

module riscv_wrapper
   (DDR3_0_addr,
    DDR3_0_ba,
    DDR3_0_cas_n,
    DDR3_0_ck_n,
    DDR3_0_ck_p,
    DDR3_0_cke,
    DDR3_0_cs_n,
    DDR3_0_dm,
    DDR3_0_dq,
    DDR3_0_dqs_n,
    DDR3_0_dqs_p,
    DDR3_0_odt,
    DDR3_0_ras_n,
    DDR3_0_reset_n,
    DDR3_0_we_n,
    eth_mdio_clock,
    eth_mdio_data,
    eth_mdio_int,
    eth_mdio_reset,
    fan_en,
    reset,
    rs232_uart_ctsn,
    rs232_uart_rtsn,
    rs232_uart_rxd,
    rs232_uart_txd,
    sdio_cd,
    sdio_clk,
    sdio_cmd,
    sdio_dat,
    sgmii_mgt_clk_clk_n,
    sgmii_mgt_clk_clk_p,
    sgmii_rxn,
    sgmii_rxp,
    sgmii_txn,
    sgmii_txp,
    sys_diff_clock_clk_n,
    sys_diff_clock_clk_p);
  output [13:0]DDR3_0_addr;
  output [2:0]DDR3_0_ba;
  output DDR3_0_cas_n;
  output [0:0]DDR3_0_ck_n;
  output [0:0]DDR3_0_ck_p;
  output [0:0]DDR3_0_cke;
  output [0:0]DDR3_0_cs_n;
  output [7:0]DDR3_0_dm;
  inout [63:0]DDR3_0_dq;
  inout [7:0]DDR3_0_dqs_n;
  inout [7:0]DDR3_0_dqs_p;
  output [0:0]DDR3_0_odt;
  output DDR3_0_ras_n;
  output DDR3_0_reset_n;
  output DDR3_0_we_n;
  output eth_mdio_clock;
  inout eth_mdio_data;
  input eth_mdio_int;
  output eth_mdio_reset;
  output fan_en;
  input reset;
  input rs232_uart_ctsn;
  output rs232_uart_rtsn;
  input rs232_uart_rxd;
  output rs232_uart_txd;
  input sdio_cd;
  output sdio_clk;
  inout sdio_cmd;
  inout [3:0]sdio_dat;
  input sgmii_mgt_clk_clk_n;
  input sgmii_mgt_clk_clk_p;
  input sgmii_rxn;
  input sgmii_rxp;
  output sgmii_txn;
  output sgmii_txp;
  input [0:0]sys_diff_clock_clk_n;
  input [0:0]sys_diff_clock_clk_p;

  wire [13:0]DDR3_0_addr;
  wire [2:0]DDR3_0_ba;
  wire DDR3_0_cas_n;
  wire [0:0]DDR3_0_ck_n;
  wire [0:0]DDR3_0_ck_p;
  wire [0:0]DDR3_0_cke;
  wire [0:0]DDR3_0_cs_n;
  wire [7:0]DDR3_0_dm;
  wire [63:0]DDR3_0_dq;
  wire [7:0]DDR3_0_dqs_n;
  wire [7:0]DDR3_0_dqs_p;
  wire [0:0]DDR3_0_odt;
  wire DDR3_0_ras_n;
  wire DDR3_0_reset_n;
  wire DDR3_0_we_n;
  wire eth_mdio_clock;
  wire eth_mdio_data;
  wire eth_mdio_int;
  wire eth_mdio_reset;
  wire fan_en;
  wire reset;
  wire rs232_uart_ctsn;
  wire rs232_uart_rtsn;
  wire rs232_uart_rxd;
  wire rs232_uart_txd;
  wire sdio_cd;
  wire sdio_clk;
  wire sdio_cmd;
  wire [3:0]sdio_dat;
  wire sgmii_mgt_clk_clk_n;
  wire sgmii_mgt_clk_clk_p;
  wire sgmii_rxn;
  wire sgmii_rxp;
  wire sgmii_txn;
  wire sgmii_txp;
  wire [0:0]sys_diff_clock_clk_n;
  wire [0:0]sys_diff_clock_clk_p;

  riscv riscv_i
       (.DDR3_0_addr(DDR3_0_addr),
        .DDR3_0_ba(DDR3_0_ba),
        .DDR3_0_cas_n(DDR3_0_cas_n),
        .DDR3_0_ck_n(DDR3_0_ck_n),
        .DDR3_0_ck_p(DDR3_0_ck_p),
        .DDR3_0_cke(DDR3_0_cke),
        .DDR3_0_cs_n(DDR3_0_cs_n),
        .DDR3_0_dm(DDR3_0_dm),
        .DDR3_0_dq(DDR3_0_dq),
        .DDR3_0_dqs_n(DDR3_0_dqs_n),
        .DDR3_0_dqs_p(DDR3_0_dqs_p),
        .DDR3_0_odt(DDR3_0_odt),
        .DDR3_0_ras_n(DDR3_0_ras_n),
        .DDR3_0_reset_n(DDR3_0_reset_n),
        .DDR3_0_we_n(DDR3_0_we_n),
        .eth_mdio_clock(eth_mdio_clock),
        .eth_mdio_data(eth_mdio_data),
        .eth_mdio_int(eth_mdio_int),
        .eth_mdio_reset(eth_mdio_reset),
        .fan_en(fan_en),
        .reset(reset),
        .rs232_uart_ctsn(rs232_uart_ctsn),
        .rs232_uart_rtsn(rs232_uart_rtsn),
        .rs232_uart_rxd(rs232_uart_rxd),
        .rs232_uart_txd(rs232_uart_txd),
        .sdio_cd(sdio_cd),
        .sdio_clk(sdio_clk),
        .sdio_cmd(sdio_cmd),
        .sdio_dat(sdio_dat),
        .sgmii_mgt_clk_clk_n(sgmii_mgt_clk_clk_n),
        .sgmii_mgt_clk_clk_p(sgmii_mgt_clk_clk_p),
        .sgmii_rxn(sgmii_rxn),
        .sgmii_rxp(sgmii_rxp),
        .sgmii_txn(sgmii_txn),
        .sgmii_txp(sgmii_txp),
        .sys_diff_clock_clk_n(sys_diff_clock_clk_n),
        .sys_diff_clock_clk_p(sys_diff_clock_clk_p));
endmodule
