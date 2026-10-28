#!/usr/bin/env bash
# Explicit board action. Start hw_server separately and select JTAG boot first.
set -eo pipefail
TASK_VITIS_DIR=$(cd "$(dirname "$0")" && pwd)
if [ "$#" -gt 1 ] || { [ "$#" -eq 1 ] && [ "$1" != '--prepare-only' ]; }; then
  echo 'Usage: run_jtag.sh [--prepare-only]' >&2; exit 1
fi
source /tools/Xilinx/Vitis/2024.2/settings64.sh
mapfile -t TASK_JTAG_FILES < <(python3 - "$TASK_VITIS_DIR" <<'PY'
from pathlib import Path
import hashlib,json,sys,zipfile
v=Path(sys.argv[1]);meta=json.loads((v/'build/latest.json').read_text())
xsa=Path(meta['xsa'])
blocked=v/'build/known_bad_xsa.json'
if blocked.exists() and meta['xsa_sha256'] in json.loads(blocked.read_text()):
    raise SystemExit('This platform has the known short-burst packet-loss bug; wait for the corrected platform build.')
if hashlib.sha256(xsa.read_bytes()).hexdigest()!=meta['xsa_sha256']:
    raise SystemExit('XSA changed since platform build; rebuild Vitis application first')
out=v/'build/jtag'/meta['xsa_sha256'][:12];out.mkdir(parents=True,exist_ok=True)
with zipfile.ZipFile(xsa) as archive:
    bits=[n for n in archive.namelist() if n.endswith('.bit')]
    inits=[n for n in archive.namelist() if n.endswith('psu_init.tcl')]
    if len(bits)!=1 or len(inits)!=1:
        raise SystemExit('Build Vitis against the final XSA containing one bitstream and psu_init.tcl first')
    for source in [bits[0],inits[0]]:
        target=out/Path(source).name;target.write_bytes(archive.read(source));print(target)
elfs=[Path(p) for p in meta['application_elf']]
if len(elfs)!=1 or not elfs[0].is_file(): raise SystemExit('Cannot select application ELF')
print(elfs[0])
PY
)
if [ "${#TASK_JTAG_FILES[@]}" -ne 3 ]; then
  echo 'JTAG preparation failed; no board programming attempted.' >&2; exit 1
fi
if [ "${1:-}" = '--prepare-only' ]; then
  printf '%s\n' "${TASK_JTAG_FILES[@]}"
  echo 'PASS: matching JTAG artifacts prepared; no board connection attempted.'
  exit 0
fi
xsdb "$TASK_VITIS_DIR/scripts/run_jtag.tcl" "${TASK_JTAG_FILES[@]}"
