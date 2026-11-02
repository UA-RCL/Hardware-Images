"""Vitis Unified 2024.2 A53 standalone build, no Vivado SDK dependency."""
import hashlib
import json
import os
import sys
import zipfile
from pathlib import Path
import vitis

ROOT = Path(__file__).resolve().parent
HW = ROOT.parent
default = HW/'vitis/hardware/ranc_aup.xsa'
xsa = Path(os.environ.get('MNIST_XSA', default)).resolve()
if not xsa.is_file():
    raise RuntimeError('Create the MNIST Vivado project/XSA first; do not use the VMM XSA')
digest = hashlib.sha256(xsa.read_bytes()).hexdigest()
full = True
sys.path.insert(0,str(HW/'scripts'))
from generate_fixture import generate
fixture_dir=generate()
workspace = ROOT/'build'/('ws_'+digest[:12]+('_full' if full else ''))
client = vitis.create_client()
try:
    client.set_workspace(str(workspace))
    if (workspace/'mnist_platform').exists():
        platform = client.get_component(name='mnist_platform')
    else:
        platform = client.create_platform_component(name='mnist_platform',
            hw_design=str(xsa), cpu='psu_cortexa53_0', os='standalone',
            domain_name='standalone_a53', is_pmufw_req=True)
    stamp = workspace/'.platform_built'
    if not stamp.exists():
        domain = platform.get_domain(name='standalone_a53')
        domain.set_config(option='os', param='standalone_stdin', value='psu_uart_1')
        domain.set_config(option='os', param='standalone_stdout', value='psu_uart_1')
        if platform.build() != 0:
            raise RuntimeError('Platform build failed')
        stamp.write_text(digest+'\n')
    xpfm = workspace/'mnist_platform/export/mnist_platform/mnist_platform.xpfm'
    if (workspace/'mnist_verify').exists():
        app = client.get_component(name='mnist_verify')
    else:
        app = client.create_app_component(name='mnist_verify', platform=str(xpfm),
                                         domain='standalone_a53', template='empty_application')
    app.import_files(from_loc=str(ROOT/'src'), files=['main.c'], dest_dir_in_cmp='src')
    if full:
        app.import_files(from_loc=str(fixture_dir),files=['fixture.h'],dest_dir_in_cmp='src')
    imported=(workspace/'mnist_verify/src/fixture.h').read_text()
    if f'#define MNIST_IMAGES {10000 if full else 16}\n' not in imported:
        raise RuntimeError('Imported fixture does not match requested verification mode')
    app.set_app_config(key='USER_COMPILE_OPTIMIZATION_LEVEL', values='-O2')
    if app.build() != 0:
        raise RuntimeError('Application build failed')
    result = dict(xsa=str(xsa), xsa_sha256=digest, workspace=str(workspace),
                  elf=[str(p) for p in (workspace/'mnist_verify').rglob('*.elf')],
                  board_tested=False)
    result['full_dataset']=full
    with zipfile.ZipFile(xsa) as archive:
        result['includes_bitstream']=any(n.endswith('.bit') for n in archive.namelist())
    (ROOT/'build'/('latest_full.json' if full else 'latest.json')).write_text(json.dumps(result, indent=2)+'\n')
    print('PASS: MNIST Vitis standalone platform/application built')
finally:
    vitis.dispose()
