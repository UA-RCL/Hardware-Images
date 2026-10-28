#!/usr/bin/env python3
"""Check a board/DMA dump of little-endian tick-tagged words against a reference."""
import argparse
from pathlib import Path
import struct

HW=Path(__file__).resolve().parents[1]
ap=argparse.ArgumentParser(description=__doc__)
ap.add_argument("capture",type=Path)
ap.add_argument("--expected-dir",type=Path,default=HW/"data",
                help="directory containing expected_rx.bin (default: original VMM)")
args=ap.parse_args()
raw=args.capture.read_bytes()
expected=(args.expected_dir/"expected_rx.bin").read_bytes()
if len(raw)!=len(expected):
    raise SystemExit(f"FAIL: expected {len(expected)} bytes ({len(expected)//4} words), got {len(raw)}")
words=[word[0] for word in struct.iter_unpack("<I",raw)]
gold=[word[0] for word in struct.iter_unpack("<I",expected)]
diff=[i+1 for i,(a,b) in enumerate(zip(words,gold)) if a!=b]
counts=[sum((word>>a)&1 for word in words) for a in range(16)]
y=[counts[2*i]-counts[2*i+1] for i in range(8)]
print("Signed VMM:",y)
print("Tick word mismatches:",len(diff))
if diff:
    raise SystemExit(f"FAIL: first differing row {diff[0]}")
print(f"PASS: {len(words)} DMA tick words exactly match the reference")
