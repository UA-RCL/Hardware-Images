#!/usr/bin/env bash
# Build and exercise the bundled original CPU simulator; no FPGA/Xilinx tools needed.
set -euo pipefail
task_dir=$(cd "$(dirname "$0")/.." && pwd)
cmake -S "$task_dir/simulator" -B "$task_dir/simulator/build" -DCMAKE_BUILD_TYPE=Release
cmake --build "$task_dir/simulator/build" -j4
python3 "$task_dir/scripts/reference.py" "$@"
