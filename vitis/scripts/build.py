"""Run with Vitis 2024.2: vitis -s ranc_hw/vitis/scripts/build.py.

Uses a hardware-hash-specific workspace; never modifies the PetaLinux tree.
RANC_XSA optionally selects the final XSA containing the routed bitstream.
"""
from pathlib import Path
import hashlib
import json
import os
import zipfile
import vitis

ROOT = Path(__file__).resolve().parents[1]
HW = ROOT.parent
default_xsa = HW/'build/ranc_aup.xsa'
if not default_xsa.exists(): default_xsa=HW/'build/ranc_aup_hw.xsa'
handoff=ROOT/'handoff/ranc_aup.xsa'
if 'RANC_XSA' not in os.environ and handoff.exists():
    if hashlib.sha256(handoff.read_bytes()).digest()!=hashlib.sha256(default_xsa.read_bytes()).digest():
        raise RuntimeError('Shared Vitis handoff is stale; run scripts/share_hardware.py --stage after export.')
    default_xsa=handoff
xsa = Path(os.environ.get('RANC_XSA', str(default_xsa))).resolve()
if not xsa.is_file():
    raise RuntimeError(f'Missing hardware handoff: {xsa}. Run the Vivado platform export first.')
digest = hashlib.sha256(xsa.read_bytes()).hexdigest()
blocked=ROOT/'build/known_bad_xsa.json'
if blocked.exists() and digest in json.loads(blocked.read_text()):
    raise RuntimeError('This XSA contains the known short-burst loader bug; wait for the corrected bitstream export.')
workspace = ROOT/'build'/('ws_'+digest[:12])
client = vitis.create_client()
try:
    client.set_workspace(str(workspace))
    if (workspace/'ranc_uart_platform').exists():
        platform = client.get_component(name='ranc_uart_platform')
    else:
        platform = client.create_platform_component(
            name='ranc_uart_platform', hw_design=str(xsa),
            cpu='psu_cortexa53_0', os='standalone', domain_name='standalone_a53',
            is_pmufw_req=True)
    xpfm = workspace/'ranc_uart_platform/export/ranc_uart_platform/ranc_uart_platform.xpfm'
    # Vitis creates a placeholder XPFM during component creation, before its
    # BSP/boot artifacts have been built. Only our successful-build stamp is
    # sufficient to reuse a platform on subsequent application builds.
    platform_stamp = workspace/'.ranc_platform_built'
    if not platform_stamp.exists() or not xpfm.exists() or os.environ.get('RANC_REBUILD_PLATFORM')=='1':
        domain = platform.get_domain(name='standalone_a53')
        domain.set_config(option='os', param='standalone_stdin', value='psu_uart_1')
        domain.set_config(option='os', param='standalone_stdout', value='psu_uart_1')
        if platform.build() != 0:
            raise RuntimeError('Vitis platform build did not return SUCCESS')
        platform_stamp.write_text(digest+'\n')
    if not xpfm.exists():
        matches = list((workspace/'ranc_uart_platform').rglob('*.xpfm'))
        if len(matches)!=1: raise RuntimeError(f'Cannot select platform: {matches}')
        xpfm = matches[0]
    if (workspace/'ranc_uart_verify').exists():
        app = client.get_component(name='ranc_uart_verify')
    else:
        app = client.create_app_component(name='ranc_uart_verify',platform=str(xpfm),
                                         domain='standalone_a53',template='empty_application')
    app.import_files(from_loc=str(ROOT/'src'),
                     files=['main.c','ranc_reference.c','ranc_reference.h','fixed_mapping.h'],
                     dest_dir_in_cmp='src')
    app.set_app_config(key='USER_COMPILE_OPTIMIZATION_LEVEL',values='-O2')
    if app.build() != 0:
        raise RuntimeError('Vitis application build did not return SUCCESS')
    elfs = list((workspace/'ranc_uart_verify').rglob('*.elf'))
    if not elfs: raise RuntimeError('Application build produced no ELF')
    with zipfile.ZipFile(xsa) as archive:
        includes_bit=any(n.endswith('.bit') for n in archive.namelist())
    result=dict(xsa=str(xsa),xsa_sha256=digest,workspace=str(workspace),platform=str(xpfm),
                application_elf=[str(p) for p in elfs],uart='UART1 MIO32..33 115200 8N1',
                includes_bitstream=includes_bit,board_tested=False)
    (ROOT/'build/latest.json').write_text(json.dumps(result,indent=2)+'\n')
    print('PASS: Vitis standalone UART verification platform and application built')
    print(json.dumps(result,indent=2))
finally:
    vitis.dispose()
