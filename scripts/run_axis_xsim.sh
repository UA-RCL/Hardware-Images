#!/usr/bin/env bash
set -eo pipefail
task_dir=$(cd "$(dirname "$0")/.." && pwd)
source "$task_dir/scripts/xilinx_env.sh" vivado
mkdir -p "$task_dir/build/axis_xsim"
cd "$task_dir/build/axis_xsim"
xvlog ../../rtl/*.v > compile.log 2>&1
xvlog --sv ../../tb/tb_mnist_axis.sv >> compile.log 2>&1
xelab tb_mnist_axis -s axis_sim -O2 > elaborate.log 2>&1
xsim axis_sim -runall > simulate.log 2>&1
cat simulate.log
grep -q 'PASS: MNIST board wrapper' simulate.log
