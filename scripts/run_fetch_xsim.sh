#!/usr/bin/env bash
set -eo pipefail
TASK_HW_DIR=$(cd "$(dirname "$0")/.." && pwd)
source /tools/Xilinx/Vivado/2024.2/settings64.sh
mkdir -p "$TASK_HW_DIR/build/fetch"
cd "$TASK_HW_DIR/build/fetch"
xvlog ../../rtl/PacketFetch.v > compile.log 2>&1
xvlog --sv ../../tb/tb_fetch.sv >> compile.log 2>&1
xelab tb_fetch -s fetch_sim > elaborate.log 2>&1
xsim fetch_sim -runall > simulate.log 2>&1
cat simulate.log
grep -q 'PASS:' simulate.log
