#!/usr/bin/env python3
"""Stage/verify identical routed hardware for Vitis and PetaLinux."""
import argparse
from datetime import datetime
import hashlib
import json
from pathlib import Path
import shutil
import xml.etree.ElementTree as ET
import zipfile
from zoneinfo import ZoneInfo

HW=Path(__file__).resolve().parents[1]
SOURCE=HW/'build/ranc_aup.xsa'
TARGETS={flow:HW/flow/'handoff/ranc_aup.xsa' for flow in ['vitis','petalinux']}
OUT=HW/'build/shared_hardware'
# Identify exact hardware bytes, not file timestamps or filenames.
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
# Check routed bitstream presence, expected register addresses, and blocked faulty handoffs.
def inspect():
    if not SOURCE.is_file():raise RuntimeError('Final routed XSA is not available')
    digest=sha(SOURCE)
    blocked=HW/'vitis/build/known_bad_xsa.json'
    if blocked.exists() and digest in json.loads(blocked.read_text()):
        raise RuntimeError('The old packet-loss XSA is blocked; wait for the corrected build')
    with zipfile.ZipFile(SOURCE) as archive:
        bits=[n for n in archive.namelist() if n.endswith('.bit')]
        if len(bits)!=1:raise RuntimeError('Routed XSA must contain exactly one bitstream')
        root=ET.fromstring(archive.read('ranc_system.hwh'))
        ranges={n.attrib['INSTANCE']:int(n.attrib['BASEVALUE'],16)
                for n in root.iter('MEMRANGE') if n.attrib.get('INSTANCE') in ['dma','control']}
        if ranges!={'dma':0x80000000,'control':0x80010000}:
            raise RuntimeError(f'Unexpected register map: {ranges}')
        part=next(root.iter('SYSTEMINFO')).attrib
        if (part['DEVICE'],part['PACKAGE'],part['SPEEDGRADE'])!=('xczu3eg','sfvc784','-2'):
            raise RuntimeError(f'Unexpected device: {part}')
        bit=archive.read(bits[0]);init=archive.read('psu_init.tcl')
    return digest,bit,init,ranges
# Copy one canonical routed XSA into both software handoff directories.
def stage():
    digest,bit,init,ranges=inspect()
    for path in TARGETS.values():
        path.parent.mkdir(parents=True,exist_ok=True)
        temp=path.with_suffix('.xsa.tmp');shutil.copyfile(SOURCE,temp);temp.replace(path)
    OUT.mkdir(parents=True,exist_ok=True)
    (OUT/'ranc_aup.bit').write_bytes(bit);(OUT/'psu_init.tcl').write_bytes(init)
    report=dict(date=datetime.now(ZoneInfo('America/Phoenix')).isoformat(),
                source=str(SOURCE),xsa_sha256=digest,bitstream_sha256=hashlib.sha256(bit).hexdigest(),
                device='xczu3eg-sfvc784-2-e',tool_version='2024.2',
                handoffs={k:str(v) for k,v in TARGETS.items()},
                register_map={k:hex(v) for k,v in ranges.items()},
                petalinux_config_imported=False,petalinux_image_built=False)
    (OUT/'manifest.json').write_text(json.dumps(report,indent=2)+'\n')
    print('PASS: identical routed XSA staged for Vitis and PetaLinux:',digest)
# Compare staged and imported hardware so Vitis and PetaLinux target the same design.
def verify(require_vitis):
    digest,bit,_,_=inspect()
    for flow,path in TARGETS.items():
        if not path.is_file() or sha(path)!=digest:raise RuntimeError(f'{flow} handoff is missing or stale: {path}')
    manifest=json.loads((OUT/'manifest.json').read_text())
    if manifest['xsa_sha256']!=digest or manifest['bitstream_sha256']!=hashlib.sha256(bit).hexdigest():
        raise RuntimeError('Shared hardware manifest is stale')
    if require_vitis:
        meta=json.loads((HW/'vitis/build/latest.json').read_text())
        if meta['xsa_sha256']!=digest or not meta['includes_bitstream']:
            raise RuntimeError('Vitis platform has not been built against the shared final XSA')
        copies=list((Path(meta['workspace'])/'ranc_uart_platform/hw').glob('*.xsa'))
        if len(copies)!=1 or sha(copies[0])!=digest:
            raise RuntimeError(f'Vitis actual imported hardware differs: {copies}')
        print('PASS: actual imported Vitis XSA matches both hardware handoffs')
    print('PASS: Vitis/PetaLinux XSA bytes and embedded bitstream match:',digest)
    return digest
if __name__=='__main__':
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--stage',action='store_true')
    ap.add_argument('--require-vitis',action='store_true',help='also check the built platform actual imported XSA')
    args=ap.parse_args()
    if args.stage:stage()
    verify(args.require_vitis)
