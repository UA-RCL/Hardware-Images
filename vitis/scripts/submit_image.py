"""Prepare an external 28x28 PNG and compare physical RANC output to its simulator.

Only workstation preprocessing uses Pillow/NumPy. The A53 encodes runtime bitmap
pixels into RANC packets; no expected masks or labels are uploaded to the board.
"""
import argparse
from datetime import datetime
import hashlib
import json
from pathlib import Path
import re
import struct
import subprocess

import numpy as np
from PIL import Image
from artifacts import load,nm_tool

ROOT=Path(__file__).resolve().parents[2]


def prepare(path, invert=False):
    """Match the authenticated original dataset's threshold and patch order."""
    with Image.open(path) as source:
        if source.size!=(28,28):
            raise ValueError('Require a 28x28 image; resize/center it explicitly before submission')
        rgba=source.convert('RGBA')
        background=Image.new('RGBA',rgba.size,(0,0,0,255))
        gray=np.asarray(Image.alpha_composite(background,rgba).convert('L'))
    if invert:
        gray=255-gray
    pixels=gray>=128
    bitmap=np.packbits(pixels.reshape(-1),bitorder='little').tobytes()
    packets=[]
    for core,(y,x) in enumerate([(0,0),(0,12),(12,0),(12,12)]):
        for axon,on in enumerate(pixels[y:y+16,x:x+16].reshape(-1)):
            if on:
                packets.append((core<<21)|(axon<<4))
    if len(packets)>511:
        raise ValueError(f'Image needs {len(packets)} packets; current staging limit is 511')
    return bitmap,packets,int(pixels.sum())


def simulate(packets,out):
    """Use the actual RANC simulator and the unchanged initialized network."""
    model=json.loads((ROOT/'data/input.json').read_text())
    model['packets']=[[dict(destination_core=[word>>21,0],
                           destination_axon=(word>>4)&255,destination_tick=0) for word in packets]]
    input_path=out/'input.json'
    input_path.write_text(json.dumps(model,separators=(',',':')))
    trace=out/'simulator_spikes.txt'
    with (out/'simulator.log').open('w') as log:
        subprocess.run([str(ROOT/'simulator/build/ranc_sim'),
            '--input',str(input_path),'--config',str(ROOT/'data/config.json'),
            '--output',str(trace),'--ticks','4','--report_freq','100'],
            check=True,stdout=log,stderr=subprocess.STDOUT)
    rows=np.loadtxt(trace,dtype=np.uint8,encoding='utf-8-sig')
    if rows.shape!=(4,256) or not np.all((rows==0)|(rows==1)):
        raise RuntimeError('Invalid simulator spike trace')
    spikes=rows[2]
    mask=[sum(int(bit)<<i for i,bit in enumerate(spikes[start:start+32])) for start in range(0,256,32)]
    votes=spikes[:250].reshape(25,10).sum(axis=0).astype(int).tolist()
    return mask,votes,int(np.argmax(votes))


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('image',type=Path,nargs='?',default=ROOT/'vitis/mnist_6.png')
    parser.add_argument('--prepare-only',action='store_true',help='write UART command/reference without board access')
    parser.add_argument('--invert',action='store_true',help='black digit on white: invert before thresholding')
    parser.add_argument('--expected-digit',type=int,choices=range(10),help='optional human label; never sent to the accelerator')
    args=parser.parse_args()
    bitmap,packets,active=prepare(args.image,args.invert)
    source_sha=hashlib.sha256(args.image.read_bytes()).hexdigest()
    key=hashlib.sha256(bitmap).hexdigest()[:12]
    out=ROOT/'build/custom_images'/args.image.stem/key
    out.mkdir(parents=True,exist_ok=True)
    (out/'uart_command.txt').write_text('image '+bitmap.hex()+'\n')
    padded=bitmap+bytes(128-len(bitmap))
    (out/'bitmap.hex').write_text('\n'.join(f'0x{word:08x}' for word in struct.unpack('<32I',padded))+'\n')
    masks,votes,prediction=simulate(packets,out)
    report=dict(timestamp=datetime.now().astimezone().isoformat(),image=str(args.image.resolve()),
        image_sha256=source_sha,bitmap_sha256=hashlib.sha256(bitmap).hexdigest(),size=[28,28],
        pixel_threshold=128,inverted=args.invert,active_pixels=active,packets=len(packets),
        simulator_prediction=prediction,simulator_votes=votes,simulator_mask=masks,
        expected_digit=args.expected_digit,physical_board_verified=False,
        uart_command=str(out/'uart_command.txt'))
    print(f'Image: {args.image}; active pixels={active}, packets={len(packets)}')
    print(f'RANC simulator prediction={prediction}, votes={votes}')
    if not args.prepare_only:
        meta=load()
        elf=Path(meta['elf'][0])
        elf_sha=hashlib.sha256(elf.read_bytes()).hexdigest()
        session_path=ROOT/'build/board_session.json'
        session=json.loads(session_path.read_text()) if session_path.exists() else {}
        if session.get('elf_sha256')!=elf_sha or session.get('xsa_sha256')!=meta['xsa_sha256']:
            raise RuntimeError('Initialize this build once: bash verify_board.sh --setup-only')
        nm=nm_tool()
        symbols=subprocess.check_output([nm,'-n',str(elf)],text=True)
        addresses={line.split()[-1]:'0x'+line.split()[0] for line in symbols.splitlines()
                   if line.split()[-1] in ['mnist_board_result','mnist_run_config','mnist_image_words']}
        if len(addresses)!=3:
            raise RuntimeError('Rebuild the custom-image-capable application first')
        board_log=out/('board_'+datetime.now().strftime('%Y%m%d_%H%M%S_%f')+'.log')
        with board_log.open('w') as log:
            result=subprocess.run(['xsdb',str(ROOT/'vitis/scripts/image_request.tcl'),
                addresses['mnist_board_result'],addresses['mnist_run_config'],addresses['mnist_image_words'],
                str(out/'bitmap.hex')],stdout=log,stderr=subprocess.STDOUT)
        text=board_log.read_text()
        match=re.search(r'CUSTOM_RESULT ([^\r\n]+)',text)
        if result.returncode or not match:
            raise RuntimeError('Physical request incomplete: '+text)
        words=[int(w,0) for w in match.group(1).split()]
        if words[38]!=len(packets) or words[49:57]!=masks or words[39:49]!=votes or words[37]!=prediction:
            raise RuntimeError(f'Physical result differs from RANC simulator; see {out}')
        report.update(physical_board_verified=True,physical_prediction=words[37],physical_votes=words[39:49],
            physical_mask=words[49:57],inference_us=words[57]|words[58]<<32,
            job_us=words[59]|words[60]<<32,request_sequence=words[32],hardware_step_tag=words[31],
            elf_sha256=elf_sha,xsa_sha256=meta['xsa_sha256'],all_output_spikes_match=True,
            board_log=str(board_log.relative_to(ROOT)))
        print(f'PHYSICAL RANC prediction={words[37]}; exact simulator spikes MATCH; inference_us={report["inference_us"]}')
    report['label_match']=None if args.expected_digit is None else prediction==args.expected_digit
    (out/'verification.json').write_text(json.dumps(report,indent=2)+'\n')
    if report['physical_board_verified']:
        board_log.with_suffix('.json').write_text(json.dumps(report,indent=2)+'\n')
    print('UART command: '+str(out/'uart_command.txt'))
    print('Report: '+str(out/'verification.json'))
    if report['label_match'] is False:
        raise SystemExit('Classifier prediction differs from the supplied label (see report); this is not a transport failure')


if __name__=='__main__':
    main()
