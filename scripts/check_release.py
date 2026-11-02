#!/usr/bin/env python3
"""Read-only release checks; no JTAG connection, programming or fixture overwrite."""
import hashlib
import json
from pathlib import Path
import struct
import subprocess
import sys
import zipfile
import xml.etree.ElementTree as ET

ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'vitis/scripts'))
from artifacts import load,nm_tool


def main():
    meta=load();elf=Path(meta['elf'][0])
    with zipfile.ZipFile(meta['xsa']) as archive:
        assert sum(n.endswith('.bit') for n in archive.namelist())==1
        assert sum(n.endswith('psu_init.tcl') for n in archive.namelist())==1
        hwh=ET.fromstring(archive.read('ranc_system.hwh'))
        assert any(m.get('INSTANCE')=='ranc' and m.get('MODTYPE')=='ranc_mnist_axis' for m in hwh.iter('MODULE'))
        ranges={m.get('INSTANCE'):int(m.get('BASEVALUE'),16) for m in hwh.iter('MEMRANGE') if m.get('INSTANCE') in ['dma','control']}
        assert ranges==dict(dma=0x80000000,control=0x80010000)
    symbols=subprocess.check_output([nm_tool(),'-n','-S',str(elf)],text=True)
    checked={}
    for line in symbols.splitlines():
        f=line.split()
        if len(f)==4 and f[3] in ['tx','rx','mnist_packets','mnist_image_words','mnist_board_result','mnist_run_config']:
            address,size=int(f[0],16),int(f[1],16)
            assert address+size<0x80000000
            if f[3]!='mnist_packets':assert address%64==0
            else:assert size==1706025*4
            checked[f[3]]=dict(address=hex(address),bytes=size)
    assert len(checked)==6
    counts=[v[0] for v in struct.iter_unpack('<I',(ROOT/'data/count.bin').read_bytes())]
    assert len(counts)==10000 and sum(counts)==1706025 and max(counts)==389
    assert (ROOT/'data/reference_full.bin').stat().st_size==10004*32
    labels=(ROOT/'data/labels.bin').read_bytes();pred=(ROOT/'data/predictions.bin').read_bytes()
    assert len(labels)==len(pred)==10000 and sum(a==b for a,b in zip(labels,pred))==9526
    provenance=json.loads((ROOT/'results/evidence/provenance.json').read_text())
    elf_sha=hashlib.sha256(elf.read_bytes()).hexdigest()
    report=dict(host_release_checks='PASS',symbols=checked,xsa_sha256=meta['xsa_sha256'],elf_sha256=elf_sha,
                matches_original_board_tested_pair=elf_sha==provenance['elf_sha256'] and meta['xsa_sha256']==provenance['xsa_sha256'],
                fixture_capacity_images=10000,full_dataset_expected_accuracy=0.9526,
                note='Host checks only; saved physical measurements are under results/evidence/')
    (ROOT/'build').mkdir(exist_ok=True)
    (ROOT/'build/release_verification.json').write_text(json.dumps(report,indent=2)+'\n')
    print(json.dumps(report,indent=2))


if __name__=='__main__':main()
