"""Generate the full C fixture from compact shipped data; standard library only."""
from pathlib import Path
import struct

ROOT=Path(__file__).resolve().parents[1]


def generate():
    data=ROOT/'data';out=ROOT/'build/fixture'
    out.mkdir(parents=True,exist_ok=True)
    counts=[v[0] for v in struct.iter_unpack('<I',(data/'count.bin').read_bytes())]
    packets=(data/'data.bin').read_bytes()
    labels=(data/'labels.bin').read_bytes();predictions=(data/'predictions.bin').read_bytes()
    masks=(data/'reference_full.bin').read_bytes()
    assert len(counts)==len(labels)==len(predictions)==10000
    assert sum(counts)*4==len(packets) and max(counts)<=511
    assert len(masks)==10004*32
    with (out/'fixture.h').open('w') as f:
        f.write('#pragma once\n#include <stdint.h>\n#define MNIST_IMAGES 10000\n#define MNIST_ROWS 10004\n#define MNIST_HAS_LABELS 1\n')
        f.write(f'#define MNIST_EXPECTED_CORRECT {sum(a==b for a,b in zip(labels,predictions))}\n')
        for name,values,width in [('mnist_counts',counts,32),
            ('mnist_packets',(v[0] for v in struct.iter_unpack('<I',packets)),32),
            ('mnist_labels',labels,8),('mnist_predictions',predictions,8)]:
            f.write(f'static const uint{width}_t {name}[]={{\n')
            for i,value in enumerate(values):
                f.write(f'0x{value:x},')
                if i%16==15:f.write('\n')
            f.write('\n};\n')
        f.write('static const uint32_t mnist_expected[MNIST_ROWS][8]={\n')
        for row in struct.iter_unpack('<8I',masks):
            f.write('{'+','.join(f'0x{v:08x}U' for v in row)+'},\n')
        f.write('};\n')
    print('PASS: generated full 10000-image C fixture: '+str(out/'fixture.h'))
    return out


if __name__=='__main__':generate()
