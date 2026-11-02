#!/usr/bin/env bash
# Resident replay; only --setup-only/--reload resets and programs the AUP.
set -eo pipefail
task_dir=$(cd "$(dirname "$0")" && pwd)
source "$task_dir/../scripts/xilinx_env.sh" vitis
python3 "$task_dir/scripts/verify_board.py" "$@"
