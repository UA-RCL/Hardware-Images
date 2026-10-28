#!/usr/bin/env bash
set -euo pipefail
task_dir=$(cd "$(dirname "$0")" && pwd)
task_cc=${CC:-/tools/Xilinx/Vitis/2024.2/gnu/aarch64/lin/aarch64-linux/bin/aarch64-linux-gnu-gcc}
mkdir -p "$task_dir/build"
"$task_cc" -O2 -Wall -Wextra -Werror -static -I "$task_dir/../vitis/src" \
    "$task_dir/ranc_linux_verify.c" "$task_dir/../vitis/src/ranc_reference.c" \
    -o "$task_dir/build/ranc_linux_verify"
file "$task_dir/build/ranc_linux_verify"
