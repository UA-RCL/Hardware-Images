#!/usr/bin/env bash
set -eo pipefail
TASK_HW_DIR=$(cd "$(dirname "$0")/.." && pwd)
source /tools/Xilinx/Vivado/2024.2/settings64.sh
# Use the shipped independent expected traces for the two opposite vectors.
mkdir -p "$TASK_HW_DIR/build/runtime"
cd "$TASK_HW_DIR/build/runtime"
xvlog ../../rtl/*.v > compile.log 2>&1
xvlog --sv ../../tb/tb_runtime.sv >> compile.log 2>&1
xelab tb_runtime -s runtime_sim -O2 > elaborate.log 2>&1
xsim runtime_sim -runall > simulate.log 2>&1
cat simulate.log
grep -q 'PASS: two different runtime vectors' simulate.log
