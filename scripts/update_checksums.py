#!/usr/bin/env python3
"""Hash publication files while excluding ignored build products and Git internals."""
import hashlib
from pathlib import Path
import subprocess

ROOT=Path(__file__).resolve().parents[1]
names=subprocess.check_output(['git','ls-files','-co','--exclude-standard','-z'],cwd=ROOT).decode().split('\0')
paths=sorted({name for name in names if name and name!='SHA256SUMS' and (ROOT/name).is_file()})
with (ROOT/'SHA256SUMS').open('w') as f:
    for name in paths:
        f.write(hashlib.sha256((ROOT/name).read_bytes()).hexdigest()+'  '+name+'\n')
print(f'Wrote SHA256SUMS for {len(paths)} publication files; ignored build/cache files excluded')
