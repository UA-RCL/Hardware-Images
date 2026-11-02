#!/usr/bin/env bash
set -eo pipefail
task_dir=$(cd "$(dirname "$0")/.." && pwd)
source "$task_dir/scripts/xilinx_env.sh" vivado
mkdir -p "$task_dir/build/max_xsim"
cd "$task_dir/build/max_xsim"
xvlog ../../rtl/*.v > compile.log 2>&1
xvlog --sv ../../tb/tb_mnist.sv >> compile.log 2>&1
xelab tb_mnist -generic_top NUM_IMAGES=1 -generic_top NUM_PACKETS=389 \
  -generic_top NUM_ROWS=5 -generic_top FIXTURE_DIR=../../data/max_fixture/ \
  -s max_sim -O2 > elaborate.log 2>&1
xsim max_sim -runall > simulate.log 2>&1
cat simulate.log
grep -q 'PASS: tutorial MNIST RTL matches 5 CPU rows' simulate.log
