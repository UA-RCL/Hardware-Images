#!/usr/bin/env bash
# Workstation PNG preparation/JTAG submission; the board runs bare-metal C.
set -eo pipefail
task_dir=$(cd "$(dirname "$0")" && pwd)
source "$task_dir/../scripts/xilinx_env.sh" vitis
python3 "$task_dir/scripts/submit_image.py" "$@"
