#!/usr/bin/env bash
set -eo pipefail
TASK_HW_DIR=$(cd "$(dirname "$0")/.." && pwd)
source /tools/Xilinx/Vivado/2024.2/settings64.sh
mkdir -p "$TASK_HW_DIR/build/units"
cd "$TASK_HW_DIR/build/units"
xvlog ../../rtl/*.v > compile.log 2>&1
xvlog --sv ../../tb/tb_units.sv >> compile.log 2>&1
xelab tb_units -s units_sim > elaborate.log 2>&1
xsim units_sim -runall > simulate.log 2>&1
cat simulate.log
grep -q 'PASS:' simulate.log
