#!/usr/bin/env bash
# Package the Linux project with AMD's packager; avoid its 2024.2 /build path bug.
set -eo pipefail
TASK_BRANCH_DIR=$(cd "$(dirname "$0")/.." && pwd)
if ! command -v petalinux-package >/dev/null; then
  echo 'Source PetaLinux 2024.2 settings.sh first.' >&2
  exit 1
fi
cd "$TASK_BRANCH_DIR/petalinux"
python3 - "$TASK_BRANCH_DIR" "$(command -v petalinux-package)" <<'PY'
import sys, runpy
from pathlib import Path
root = Path(sys.argv[1])
packager = Path(sys.argv[2]).resolve()
sys.path.insert(0, str(packager.parent / 'libs'))
import plnx_vars
# The BSP command passes an empty project root into BuildDir.format(), producing
# /build in this tool release. Change only this process's logging/build location.
plnx_vars.BuildDir = str(root / 'petalinux/build')
sys.argv = [str(packager), 'bsp', '-p', str(root / 'petalinux'),
            '--hwsource', str(root / 'vitis/handoff/ranc_aup.xsa'),
            '--exclude-workspace', '-o', str(root / 'AUP-ZCU3-1vmm-ranc.bsp')]
runpy.run_path(str(packager), run_name='__main__')
PY
