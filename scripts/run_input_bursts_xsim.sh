#!/usr/bin/env bash
set -eo pipefail
TASK_HW_DIR=$(cd "$(dirname "$0")/.." && pwd)
source /tools/Xilinx/Vivado/2024.2/settings64.sh
mkdir -p "$TASK_HW_DIR/build/input_bursts"
cd "$TASK_HW_DIR/build/input_bursts"
xvlog ../../rtl/RANCNetwork_S00_AXIS.v ../../rtl/PacketFetch.v > compile.log 2>&1
xvlog --sv ../../tb/tb_input_bursts.sv >> compile.log 2>&1
xelab tb_input_bursts -s input_bursts_sim -O2 > elaborate.log 2>&1
xsim input_bursts_sim -runall > simulate.log 2>&1
cat simulate.log
grep -q 'PASS: complete AXIS bursts' simulate.log
