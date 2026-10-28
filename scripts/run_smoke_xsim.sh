#!/usr/bin/env bash
set -eo pipefail
TASK_HW_DIR=$(cd "$(dirname "$0")/.." && pwd)
source /tools/Xilinx/Vivado/2024.2/settings64.sh
mkdir -p "$TASK_HW_DIR/build/smoke"
cd "$TASK_HW_DIR/build/smoke"
xvlog ../../rtl/*.v > compile.log 2>&1
xvlog --sv ../../tb/tb_vmm.sv >> compile.log 2>&1
xelab tb_vmm -generic_top RUN_TICKS=25 -s smoke_sim -O2 > elaborate.log 2>&1
xsim smoke_sim -runall > simulate.log 2>&1
cat simulate.log
grep -q 'PASS: signed VMM RTL, 25 ticks' simulate.log
