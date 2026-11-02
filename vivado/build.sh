#!/usr/bin/env bash
# Recreate the GUI project; --bitstream additionally synthesizes/routes/exports.
set -eo pipefail
task_dir=$(cd "$(dirname "$0")/.." && pwd)
if [ "$#" -gt 1 ] || { [ "$#" -eq 1 ] && [ "$1" != '--bitstream' ]; }; then
  echo 'Usage: bash vivado/build.sh [--bitstream]' >&2; exit 1
fi
source "$task_dir/scripts/xilinx_env.sh" vivado
mkdir -p "$task_dir/build"
cd "$task_dir/build"
vivado -mode batch -source ../vivado/create_project.tcl -log create_project.log -journal create_project.jou
if [ "${1:-}" = '--bitstream' ]; then
  vivado -mode batch -source ../vivado/build_bitstream.tcl -log build_bitstream.log -journal build_bitstream.jou
fi
