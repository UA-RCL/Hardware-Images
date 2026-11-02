#!/usr/bin/env python3
"""Export an offline explainer and a printable figure from actual MNIST runs.

Hidden states are reconstructed from deployed connections/weights. Output masks
must match both saved FPGA measurements and the RANC simulator before export.
No board access, network access or replacement model is used.
"""
import base64
from datetime import datetime
import hashlib
import io
import json
import os
from pathlib import Path

os.environ.setdefault('MPLCONFIGDIR','/tmp/ranc-mnist-matplotlib')
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from matplotlib.colors import ListedColormap
from matplotlib.patches import Rectangle
import numpy as np
from PIL import Image

HERE=Path(__file__).resolve().parent
ROOT=HERE.parents[1]
MNIST=ROOT
STARTS=[(0,0),(0,12),(12,0),(12,12)]
COLORS=['#287fbb','#ac5496','#d18a26','#168b81']


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def neuron_layer(core,source,used):
    """Each axon's instruction selects one of its neuron's four signed weights."""
    neurons=core['neurons']
    weights=np.asarray([n['weights'] for n in neurons],dtype=int)[:,core['axons']]
    weights=weights*np.asarray(core['connections'],dtype=int)
    leak=np.asarray([n['leak'] for n in neurons],dtype=int)
    thresholds=np.asarray([n['positive_threshold'] for n in neurons],dtype=int)
    net=weights@source+leak
    spikes=(net>=thresholds).astype(int)
    assert all(n['reset_mode']==0 and n['current_potential']==0 and n['reset_potential']==0 and n['negative_threshold']==0
               for n in neurons[:used]), 'Reconstruction requires the audited absolute-reset mapping'
    assert np.all(thresholds[:used]==0)
    # Bound every scan prefix and the leak update at the deployed 9-bit width.
    low=np.minimum(weights,0)@source
    high=np.maximum(weights,0)@source
    assert min(low.min(),(low+leak).min())>=-256
    assert max(high.max(),(high+leak).max())<=255
    assert not spikes[used:].any(), 'Padded neurons must remain silent'
    return dict(weights=weights[:used].tolist(),bias=leak[:used].tolist(),
                threshold=thresholds[:used].tolist(),score=net[:used].tolist(),
                spikes=spikes[:used].tolist())


def sample(stem,model):
    candidates=list((MNIST/'results/evidence/custom_images'/stem).glob('*/verification.json'))
    report_path=max(candidates,key=lambda p:json.loads(p.read_text())['timestamp'])
    report=json.loads(report_path.read_text())
    path=MNIST/report['image']
    assert sha(path)==report['image_sha256'], 'Source PNG changed since physical test'
    with Image.open(path) as im:
        rgba=im.convert('RGBA')
        gray=np.asarray(Image.alpha_composite(Image.new('RGBA',im.size,(0,0,0,255)),rgba).convert('L'))
    assert gray.shape==(28,28)
    if report['inverted']:
        gray=255-gray
    binary=(gray>=report['pixel_threshold']).astype(int)
    bitmap=np.packbits(binary.reshape(-1),bitorder='little').tobytes()
    assert hashlib.sha256(bitmap).hexdigest()==report['bitmap_sha256']
    patches=[binary[y:y+16,x:x+16].reshape(-1).tolist() for y,x in STARTS]
    layers=[neuron_layer(core,np.asarray(patch),64) for core,patch in zip(model['cores'][:4],patches)]
    hidden=np.concatenate([layer['spikes'] for layer in layers])
    output=neuron_layer(model['cores'][4],hidden,250)
    spikes=np.asarray(output['spikes'])
    padded=np.pad(spikes,(0,6))
    masks=[sum(int(bit)<<i for i,bit in enumerate(padded[start:start+32])) for start in range(0,256,32)]
    votes=spikes.reshape(25,10).sum(axis=0).tolist()
    prediction=int(np.argmax(votes))
    assert report['physical_board_verified'] and report['all_output_spikes_match']
    assert masks==report['physical_mask']==report['simulator_mask']
    assert votes==report['physical_votes']==report['simulator_votes']
    assert prediction==report['physical_prediction']==report['simulator_prediction']
    assert sum(map(sum,patches))==report['packets']
    # Re-read simulator rows rather than relying solely on summary metadata.
    trace=np.loadtxt(report_path.parent/'simulator_spikes.txt',dtype=int,encoding='utf-8-sig')
    assert np.array_equal(trace[2],padded)
    png=io.BytesIO()
    Image.fromarray(gray.astype('uint8')).save(png,format='PNG')
    sorted_votes=sorted(votes,reverse=True)
    return dict(name=stem,digit=prediction,gray=gray.reshape(-1).tolist(),binary=binary.reshape(-1).tolist(),
        png='data:image/png;base64,'+base64.b64encode(png.getvalue()).decode(),patches=patches,
        hidden=layers,output=output,votes=votes,active_pixels=int(binary.sum()),packets=report['packets'],
        hidden_count=int(hidden.sum()),output_count=int(spikes.sum()),vote_margin=sorted_votes[0]-sorted_votes[1],
        inference_us=report['inference_us'],job_us=report['job_us'],
        evidence=dict(report=str(report_path.relative_to(ROOT)),board_log=report['board_log'],
                      timestamp=report['timestamp'],image_sha256=report['image_sha256'],
                      bitmap_sha256=report['bitmap_sha256'],elf_sha256=report['elf_sha256'],
                      xsa_sha256=report['xsa_sha256'],output_masks=masks))


def figure(samples):
    """Two aligned columns preserve the actual 2-D pixel and voter arrangements."""
    plt.rcParams.update({'font.family':'DejaVu Sans','font.size':10,'axes.titleweight':'bold',
                         'axes.spines.top':False,'axes.spines.right':False,'svg.fonttype':'none'})
    fig=plt.figure(figsize=(14,15.7),facecolor='#f6f8fb')
    grid=fig.add_gridspec(4,2,left=.07,right=.97,top=.91,bottom=.13,
                         height_ratios=[1.1,.86,1.72,1.05],hspace=.55,wspace=.25)
    for column,s in enumerate(samples):
        ax=fig.add_subplot(grid[0,column]);ax.set_facecolor('white')
        ax.imshow(np.asarray(s['gray']).reshape(28,28),cmap='gray',vmin=0,vmax=255,interpolation='nearest')
        for core,((y,x),color) in enumerate(zip(STARTS,COLORS)):
            ax.add_patch(Rectangle((x-.5,y-.5),16,16,fill=False,ec=color,lw=2.2))
            ax.text(x+.2,y+.1,str(core),color='white',va='top',weight='bold',fontsize=9,
                    bbox=dict(facecolor=color,edgecolor='none',pad=1.5))
        ax.set_title(f'1  Input {s["digit"]}: four overlapping 16×16 patches',pad=12)
        ax.set_xlabel(f'{s["active_pixels"]} active pixels → {s["packets"]} input packets (threshold ≥128)')
        ax.set_xticks([]);ax.set_yticks([])
        sub=grid[1,column].subgridspec(1,4,wspace=.12)
        for core,layer in enumerate(s['hidden']):
            ax=fig.add_subplot(sub[0,core]);color=COLORS[core]
            ax.imshow(np.asarray(layer['spikes']).reshape(8,8),cmap=ListedColormap(['#e5eaf0',color]),vmin=0,vmax=1)
            ax.set_title(f'Core {core}\n{sum(layer["spikes"])}/64 spikes',color=color,fontsize=10)
            ax.set_xticks([]);ax.set_yticks([])
            if core==0: ax.set_ylabel('2  Hidden layer',fontsize=11,weight='bold')
            ax.set_xlabel('Neuron IDs 0–63',fontsize=8)
        ax=fig.add_subplot(grid[2,column]);ax.set_facecolor('white')
        ax.imshow(spikes:=np.asarray(s['output']['spikes']).reshape(25,10),
                  cmap=ListedColormap(['#e5eaf0','#168b81']),vmin=0,vmax=1,aspect='auto',interpolation='nearest')
        ax.add_patch(Rectangle((s['digit']-.5,-.5),1,25,fill=False,ec='#152b43',lw=2))
        ax.set_title(f'3  Output layer: {s["output_count"]} recorded spikes / 250 neurons',pad=12)
        ax.set_xticks(range(10));ax.set_xlabel('Class column (digit)')
        ax.set_yticks([0,4,9,14,19,24],['0','4','9','14','19','24'])
        ax.set_ylabel('Voter group (25 groups of 10 neurons)')
        ax.set_xticks(np.arange(-.5,10,1),minor=True);ax.set_yticks(np.arange(-.5,25,1),minor=True)
        ax.grid(which='minor',color='white',lw=1.5);ax.tick_params(which='minor',length=0)
        ax=fig.add_subplot(grid[3,column]);ax.set_facecolor('white')
        bars=ax.bar(range(10),s['votes'],color=['#168b81' if c==s['digit'] else '#a9bacb' for c in range(10)])
        ax.bar_label(bars,padding=4,fontsize=11)
        ax.set_xticks(range(10));ax.set_ylim(0,19);ax.set_yticks([0,5,10,15])
        ax.set_ylabel('Output spikes = votes');ax.set_xlabel('Digit class')
        ax.set_title(f'4  argmax(votes) = {s["digit"]} · {max(s["votes"])} votes · lead {s["vote_margin"]}',pad=12)
        ax.text(.5,-.31,f'FPGA = RANC simulator · measured inference {s["inference_us"]/1000:.3f} ms',
                transform=ax.transAxes,ha='center',color='#536579',fontsize=10)
    fig.suptitle('From MNIST pixels to RANC spike votes',x=.07,ha='left',y=.975,fontsize=23,weight='bold',color='#152b43')
    fig.text(.07,.942,'Two real PNG submissions · unchanged deployed weights · exact FPGA / simulator output agreement',fontsize=12,color='#536579')
    fig.text(.07,.028,'Read top to bottom. Hidden 8×8 tiles arrange neuron IDs; they are not spatial feature maps. Hidden spikes are model reconstructions.\n'
             'Output tiles are saved FPGA measurements. Each lit cell contributes one vote to its class column; a row may vote for several classes.\n'
             'Votes are spike counts, not probabilities. Inference timing excludes host preprocessing/upload and the two blank flushing steps.',
             fontsize=9,color='#536579',linespacing=1.6)
    fig.savefig(HERE/'mnist_spike_flow.png',dpi=180,facecolor=fig.get_facecolor())
    fig.savefig(HERE/'mnist_spike_flow.svg',facecolor=fig.get_facecolor())
    plt.close(fig)


def main():
    model_path=MNIST/'data/input.json'
    model=json.loads(model_path.read_text())
    for c,core in enumerate(model['cores'][:4]):
        assert core['coordinates']==[c,0]
        assert all(n['destination_core_offset']==[4-c,0] and n['destination_axon']==c*64+j
                   for j,n in enumerate(core['neurons'][:64]))
    assert model['cores'][4]['coordinates']==[4,0]
    assert all(n['destination_core_offset']==[1,0] and n['destination_axon']==j
               for j,n in enumerate(model['cores'][4]['neurons'][:250]))
    samples=[sample(name,model) for name in ['mnist_other_sample','mnist_6']]
    result=dict(generated_at=datetime.now().astimezone().isoformat(),
                model_path=str(model_path.relative_to(ROOT)),model_sha256=sha(model_path),
                colors=COLORS,patch_starts=STARTS,samples=samples,
                checks='Reconstructed masks = saved RANC simulator rows = recorded physical FPGA masks, all 256 bits, both images')
    encoded=json.dumps(result,separators=(',',':'))
    (HERE/'mnist_trace.json').write_text(encoded+'\n')
    template=(HERE/'explainer.template.html').read_text()
    (HERE/'mnist_spike_explainer.html').write_text(template.replace('__TRACE_DATA__',encoded.replace('</','<\\/')))
    figure(samples)
    print(result['checks'])
    for s in samples:
        print(f'digit {s["digit"]}: {s["active_pixels"]} pixels / {s["packets"]} packets / '
              f'{s["hidden_count"]} hidden spikes / {s["output_count"]} output spikes / votes {s["votes"]}')
    print('Exported offline HTML, PNG, SVG and reproducible trace to '+str(HERE))


if __name__=='__main__':
    main()
