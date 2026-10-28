#!/usr/bin/env bash
set -euo pipefail
TASK_HW_DIR=$(cd "$(dirname "$0")/.." && pwd)
set +u
source /tools/Xilinx/Vivado/2024.2/settings64.sh
set -u
cd "$TASK_HW_DIR"
# RTL and initialized memories are shipped; no upstream workspace is required.
mkdir -p build/vmm
cd build/vmm
xvlog ../../rtl/*.v > compile.log 2>&1
xvlog --sv ../../tb/tb_vmm.sv >> compile.log 2>&1
xelab tb_vmm -s vmm_sim -O2 > elaborate.log 2>&1
xsim vmm_sim -runall > simulate.log 2>&1
cat simulate.log
grep -q 'PASS: signed VMM RTL, 615 ticks, 2632 packets' simulate.log
