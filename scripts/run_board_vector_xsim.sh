#!/usr/bin/env bash
set -eo pipefail
TASK_HW_DIR=$(cd "$(dirname "$0")/.." && pwd)
source /tools/Xilinx/Vivado/2024.2/settings64.sh
mkdir -p "$TASK_HW_DIR/build/board_vector"
cd "$TASK_HW_DIR/build/board_vector"
xvlog ../../rtl/*.v > compile.log 2>&1
xvlog --sv ../../tb/tb_board_vector.sv >> compile.log 2>&1
xelab tb_board_vector -s board_vector_sim -O2 > elaborate.log 2>&1
xsim board_vector_sim -runall > simulate.log 2>&1
cat simulate.log
grep -q 'PASS: fresh board wrapper' simulate.log
