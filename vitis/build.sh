#!/usr/bin/env bash
set -eo pipefail
task_dir=$(cd "$(dirname "$0")" && pwd)
source "$task_dir/../scripts/xilinx_env.sh" vitis
export XILINX_VITIS_DATA_DIR="$task_dir/build/tool_data"
mkdir -p "$XILINX_VITIS_DATA_DIR"
vitis -s "$task_dir/build.py"
