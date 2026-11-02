"""Set up once, then submit tests to the resident bare-metal application."""
import argparse
from datetime import datetime
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess
from artifacts import load,nm_tool

ROOT=Path(__file__).resolve().parents[2]
parser=argparse.ArgumentParser(description=__doc__)
mode=parser.add_mutually_exclusive_group()
mode.add_argument('--images',type=int,help='test the first N images (1..10000, default 10000)')
mode.add_argument('--interactive',action='store_true',help='select image count on UART before timing')
parser.add_argument('--verbose',action='store_true',help='include UART progress output in timed loop')
parser.add_argument('--read-existing',type=Path,help='read the last resident result associated with this launch log')
parser.add_argument('--setup-only',action='store_true',help='program and initialize once, leave app waiting for commands')
parser.add_argument('--reload',action='store_true',help='explicitly reset/program/reload before this test')
args=parser.parse_args()
requested=args.images if args.images is not None else 10000
if not 1<=requested<=10000:
    parser.error('--images must be in 1..10000')
subprocess.run(['python3',str(ROOT/'scripts/check_release.py')],check=True)
meta=load()
elf=Path(meta['elf'][0])
nm=nm_tool()
symbols=subprocess.check_output([nm,'-n',str(elf)],text=True)
addresses={line.split()[-1]:'0x'+line.split()[0] for line in symbols.splitlines()
           if line.split()[-1] in ['mnist_board_result','mnist_run_config']}
if len(addresses)!=2:
    raise RuntimeError('Rebuild the full app with runtime selection and timing first')
identity=dict(elf_sha256=hashlib.sha256(elf.read_bytes()).hexdigest(),xsa_sha256=meta['xsa_sha256'],addresses=addresses)
session=ROOT/'build/board_session.json'
if not (args.reload or args.setup_only or args.read_existing):
    if not session.exists() or json.loads(session.read_text())!=identity:
        raise RuntimeError('Set up the matching resident app first: bash verify_board.sh --setup-only')
if args.read_existing:
    out=args.read_existing.resolve()
    if not out.is_relative_to((ROOT/'build/board').resolve()) or not (out/'launch.log').is_file():
        raise RuntimeError('Existing log must be inside build/board')
    launch=json.loads((out/'launch_options.json').read_text())
    if launch['elf_sha256']!=hashlib.sha256(elf.read_bytes()).hexdigest() or launch['xsa_sha256']!=meta['xsa_sha256']:
        raise RuntimeError('Existing run uses a different ELF/XSA; do not misattribute its results')
else:
    out=ROOT/'build/board'/datetime.now().strftime('%Y%m%d_%H%M%S')
    out.mkdir(parents=True)
flags=int(args.interactive)|int(args.verbose)*2
env=dict(os.environ,MNIST_FULL='1',MNIST_RUN_CONFIG_ADDRESS=addresses['mnist_run_config'],
         MNIST_RUN_IMAGES=hex(requested),MNIST_RUN_FLAGS=hex(flags))
if not args.read_existing:
    (out/'launch_options.json').write_text(json.dumps(dict(images=requested,interactive=args.interactive,
        verbose=args.verbose,xsa_sha256=meta['xsa_sha256'],elf_sha256=hashlib.sha256(elf.read_bytes()).hexdigest()),indent=2)+'\n')
    if args.reload or args.setup_only:
        print('One-time JTAG reset, FPGA programming, PS initialization and ELF download...',flush=True)
        with (out/'launch.log').open('w') as log:
            subprocess.run(['bash',str(ROOT/'vitis/run_jtag.sh')],env=env,stdout=log,stderr=subprocess.STDOUT,check=True)
        subprocess.run(['xsdb',str(ROOT/'vitis/scripts/ready.tcl'),addresses['mnist_board_result']],check=True)
        session.write_text(json.dumps(identity,indent=2)+'\n')
    else:
        (out/'launch.log').write_text('Reusing resident app: no reset, FPGA programming or ELF download.\n')
    if args.setup_only:
        print('READY. UART: run 100. Or: bash verify_board.sh --images 100')
        raise SystemExit(0)
print('Reading existing resident result...' if args.read_existing else
      'Submitting test to resident app; only image DMA and result verification run on board...',flush=True)
result_log=out/('results_retry.log' if args.read_existing else 'results.log')
with result_log.open('w') as log:
    command=['xsdb',str(ROOT/'vitis/scripts/check_board.tcl'),addresses['mnist_board_result']]
    if not args.read_existing:
        command += [addresses['mnist_run_config'],str(requested),str(flags)]
    process=subprocess.run(command,
                           stdout=log,stderr=subprocess.STDOUT)
text=result_log.read_text()
match=re.search(r'PHYSICAL_MNIST PASS correct=(\d+) total=(\d+) ticks=(\d+)',text)
if process.returncode or not match:
    print(text)
    raise RuntimeError(f'Physical verification failed/incomplete; see {out}')
print('\n'.join(line for line in text.splitlines() if line.startswith('PHYSICAL_')))
correct,images,steps=map(int,match.groups())
if not args.interactive and not args.read_existing and images!=requested:
    raise RuntimeError('Hardware did not use the selected image count')
labels=(ROOT/'data/labels.bin').read_bytes()
predictions=(ROOT/'data/predictions.bin').read_bytes()
expected=sum(a==b for a,b in zip(labels[:images],predictions[:images]))
if correct!=expected:
    raise RuntimeError('Physical prefix accuracy differs from independent host baseline')
timing_line=re.search(r'PHYSICAL_MNIST_TIMING ([^\r\n]+)',text)
if not timing_line:
    raise RuntimeError('Missing actual timer measurements')
timing={k:int(v) for k,v in re.findall(r'(\w+)=(\d+)',timing_line.group(1))}
raw=[int(w,0) for w in re.findall(r'BOARD_RESULT ([^\r\n]+)',text)[-1].split()]
launch=json.loads((out/'launch_options.json').read_text())
if args.read_existing:
    if launch.get('request_sequence')!=raw[32]:
        raise RuntimeError('Resident result has been replaced by a different job; use saved verification.json')
else:
    launch['request_sequence']=raw[32]
    (out/'launch_options.json').write_text(json.dumps(launch,indent=2)+'\n')
sum_latency_us=raw[18]|raw[19]<<32
timing['sum_image_latency_us']=sum_latency_us
timing['mean_image_latency_us_from_sum']=sum_latency_us/images
timing['amortized_us_per_image_from_total']=timing['test_us']/images
report=dict(timestamp=datetime.now().astimezone().isoformat(),physical_board_verified=True,
            method='JTAG reads of A53-published actual DMA verification results and XTime measurements',
            images=images,correct=correct,accuracy=correct/images,verified_steps=steps,
            launch_options=launch,resident_run_number=raw[34],hardware_step_tag=raw[31],
            timing=timing,timing_notes='Application: job entry to results-ready, before final UART reporting; excludes idle command wait and one-time initialization. Setup includes two blank flush steps. Test: timed DMA/verification loop. Mean latency: input submission to verified prediction. Amortized: test time/images.',
            xsa_sha256=meta['xsa_sha256'],elf_sha256=hashlib.sha256(elf.read_bytes()).hexdigest(),
            result_address=addresses['mnist_board_result'],logs=str(out.relative_to(ROOT)))
(out/'verification.json').write_text(json.dumps(report,indent=2)+'\n')
(ROOT/'build/board_latest.json').write_text(json.dumps(report,indent=2)+'\n')
print('PASS: physical Zynq–PL MNIST verification; '+str(out))
