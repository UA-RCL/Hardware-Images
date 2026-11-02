#!/usr/bin/env bash
set -eo pipefail
task_dir=$(cd "$(dirname "$0")/.." && pwd)
source "$task_dir/scripts/xilinx_env.sh" vivado
mkdir -p "$task_dir/build/xsim"
cd "$task_dir/build/xsim"
xvlog ../../rtl/*.v > compile.log 2>&1
xvlog --sv ../../tb/tb_mnist.sv >> compile.log 2>&1
xelab tb_mnist -s mnist_sim -O2 > elaborate.log 2>&1
xsim mnist_sim -runall > simulate.log 2>&1
cat simulate.log
grep -q 'PASS: tutorial MNIST RTL' simulate.log
