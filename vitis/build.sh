#!/usr/bin/env bash
set -eo pipefail
TASK_VITIS_DIR=$(cd "$(dirname "$0")" && pwd)
source /tools/Xilinx/Vitis/2024.2/settings64.sh
export XILINX_VITIS_DATA_DIR="$TASK_VITIS_DIR/build/tool_data"
mkdir -p "$XILINX_VITIS_DATA_DIR"
# Use the shipped, verified src/fixed_mapping.h without external mapping dependencies.
cd "$TASK_VITIS_DIR"
vitis -s "$TASK_VITIS_DIR/scripts/build.py"
