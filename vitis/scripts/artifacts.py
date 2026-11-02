"""Select locally rebuilt artifacts, or the byte-identical shipped board release."""
import hashlib
import json
import os
from pathlib import Path
import shutil

ROOT=Path(__file__).resolve().parents[2]


def load():
    built=ROOT/'vitis/build/latest_full.json'
    path=built if built.exists() else ROOT/'vitis/prebuilt/artifacts.json'
    meta=json.loads(path.read_text())
    for field in ['xsa','workspace']:
        if field in meta:meta[field]=str((ROOT/meta[field]).resolve())
    meta['elf']=[str((ROOT/p).resolve()) for p in meta['elf']]
    if hashlib.sha256(Path(meta['xsa']).read_bytes()).hexdigest()!=meta['xsa_sha256']:
        raise RuntimeError('Selected XSA changed; rebuild Vitis against it first')
    if len(meta['elf'])!=1 or not Path(meta['elf'][0]).is_file():
        raise RuntimeError('Missing selected application ELF')
    if 'elf_sha256' in meta and hashlib.sha256(Path(meta['elf'][0]).read_bytes()).hexdigest()!=meta['elf_sha256']:
        raise RuntimeError('Shipped ELF checksum mismatch')
    return meta


def nm_tool():
    if os.environ.get('AARCH64_NM'):return os.environ['AARCH64_NM']
    found=shutil.which('aarch64-none-elf-nm') or shutil.which('aarch64-linux-gnu-nm')
    if found:return found
    default=Path(os.environ.get('XILINX_VITIS','/tools/Xilinx/Vitis/2024.2'))
    candidate=default/'gnu/aarch64/lin/aarch64-none/bin/aarch64-none-elf-nm'
    if candidate.is_file():return str(candidate)
    raise RuntimeError('Source Vitis 2024.2 or set AARCH64_NM to an AArch64 nm executable')
