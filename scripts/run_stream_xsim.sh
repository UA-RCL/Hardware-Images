#!/usr/bin/env bash
set -eo pipefail
TASK_HW_DIR=$(cd "$(dirname "$0")/.." && pwd)
source /tools/Xilinx/Vivado/2024.2/settings64.sh
mkdir -p "$TASK_HW_DIR/build/stream"
cd "$TASK_HW_DIR/build/stream"
xvlog ../../rtl/ranc_tick_stream.v > compile.log 2>&1
xvlog --sv ../../tb/tb_stream.sv >> compile.log 2>&1
xelab tb_stream -s stream_sim > elaborate.log 2>&1
xsim stream_sim -runall > simulate.log 2>&1
cat simulate.log
grep -q 'PASS:' simulate.log
