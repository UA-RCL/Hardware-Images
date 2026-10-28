#!/usr/bin/env bash
set -eo pipefail
TASK_HW_DIR=$(cd "$(dirname "$0")/.." && pwd)
source /tools/Xilinx/Vivado/2024.2/settings64.sh
mkdir -p "$TASK_HW_DIR/build/board_reset"
cd "$TASK_HW_DIR/build/board_reset"
xvlog ../vivado/ranc_aup.gen/sources_1/bd/ranc_system/ip/ranc_system_reset_0/ranc_system_reset_0_sim_netlist.v > compile.log 2>&1
xvlog --sv ../../tb/tb_board_reset.sv >> compile.log 2>&1
xelab tb_board_reset glbl -L unisims_ver -L secureip -s board_reset_sim > elaborate.log 2>&1
xsim board_reset_sim -runall > simulate.log 2>&1
cat simulate.log
grep -q 'PASS: generated AUP reset IP' simulate.log
