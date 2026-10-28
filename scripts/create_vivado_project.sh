#!/usr/bin/env bash
set -eo pipefail
TASK_HW_DIR=$(cd "$(dirname "$0")/.." && pwd)
source /tools/Xilinx/Vivado/2024.2/settings64.sh
cd "$TASK_HW_DIR/build"
vivado -mode batch -source ../vivado/create_project.tcl -log create_project.log -journal create_project.jou
