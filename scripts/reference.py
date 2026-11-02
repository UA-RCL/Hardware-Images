"""Compare bundled C++ simulator results with the shipped exact-spike references."""
import argparse
import json
from pathlib import Path
import struct
import subprocess

ROOT=Path(__file__).resolve().parents[1]


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--full',action='store_true',help='all 10000 images instead of the 16-image smoke test')
    args=parser.parse_args();data=ROOT/'data';out=ROOT/'build/reference'
    out.mkdir(parents=True,exist_ok=True)
    model_path=data/'input.json';ticks=20;images=16
    if args.full:
        images=10000;ticks=10004;model=json.loads(model_path.read_text());model.pop('packets')
        counts=[v[0] for v in struct.iter_unpack('<I',(data/'count.bin').read_bytes())]
        words=(data/'data.bin').read_bytes();offset=0;model_path=out/'full_input.json'
        # Stream image-sized chunks to avoid a second full packet graph in RAM.
        with model_path.open('w') as f:
            f.write(json.dumps(model)[:-1]+',"packets":[')
            for i,count in enumerate(counts):
                packets=[]
                for word, in struct.iter_unpack('<I',words[offset:offset+count*4]):
                    packets.append(dict(destination_core=[word>>21,0],destination_axon=(word>>4)&255,destination_tick=word&15))
                offset+=count*4
                f.write((',' if i else '')+json.dumps(packets,separators=(',',':')))
            f.write(']}')
    trace=out/('full_spikes.txt' if args.full else 'cpu_spikes.txt')
    with (out/'simulator.log').open('w') as log:
        subprocess.run([str(ROOT/'simulator/build/ranc_sim'),'--input',str(model_path),
            '--config',str(data/'config.json'),'--output',str(trace),'--ticks',str(ticks),
            '--report_freq','1000'],check=True,stdout=log,stderr=subprocess.STDOUT)
    rows=[[int(v) for v in row.split()] for row in trace.read_text(encoding='utf-8-sig').splitlines()]
    assert len(rows)==ticks and all(len(row)==256 and set(row)<={0,1} for row in rows)
    packed=b''.join(struct.pack('<8I',*[sum(bit<<i for i,bit in enumerate(row[j:j+32])) for j in range(0,256,32)]) for row in rows)
    if args.full:expected=(data/'reference_full.bin').read_bytes()
    else:
        masks=[int(v,16) for v in (data/'expected_spikes.mem').read_text().split()]
        expected=b''.join(mask.to_bytes(32,'little') for mask in masks)
    assert packed==expected,'Simulator output differs from shipped spike reference'
    predictions=[]
    for row in rows[2:images+2]:
        votes=[sum(row[c:250:10]) for c in range(10)]
        predictions.append(max(range(10),key=lambda c:votes[c]))
    labels=(data/'labels.bin').read_bytes()[:images]
    assert bytes(predictions)==(data/'predictions.bin').read_bytes()[:images]
    correct=sum(a==b for a,b in zip(labels,predictions))
    report=dict(simulator='PASS',images=images,ticks=ticks,channels=256,
        all_reference_masks_match=True,output_spikes=sum(map(sum,rows)),correct=correct,accuracy=correct/images)
    (out/('full_verification.json' if args.full else 'verification.json')).write_text(json.dumps(report,indent=2)+'\n')
    if args.full:model_path.unlink()
    print(json.dumps(report,indent=2))


if __name__=='__main__':main()
