#!/usr/bin/env python3
"""Run verifier commands over a Linux serial port; Python standard library only."""
import argparse
import os
from pathlib import Path
import select
import sys
import termios
import time

ap=argparse.ArgumentParser(description=__doc__)
ap.add_argument('--port',required=True,help='board USB UART, e.g. /dev/ttyUSB1')
ap.add_argument('--command',action='append',help='repeat for multiple commands (default: quick)')
ap.add_argument('--timeout',type=float,default=120,help='seconds per command')
ap.add_argument('--log',type=Path,default=Path('ranc_uart_capture.txt'))
args=ap.parse_args()
commands=args.command or ['quick']
if args.timeout<=0 or any('\n' in cmd or '\r' in cmd for cmd in commands):
    ap.error('positive timeout and single-line commands required')
fd=os.open(args.port,os.O_RDWR|os.O_NOCTTY|os.O_NONBLOCK)
old=termios.tcgetattr(fd)
transcript=bytearray()
def send(data):
    view=memoryview(data)
    while view:
        _,ready,_=select.select([], [fd], [], args.timeout)
        if not ready: raise TimeoutError('serial write timed out')
        view=view[os.write(fd,view):]
def read_prompt(timeout,required=None):
    result=bytearray();deadline=time.monotonic()+timeout
    while time.monotonic()<deadline:
        ready,_,_=select.select([fd],[],[],min(0.25,max(0,deadline-time.monotonic())))
        if ready:
            data=os.read(fd,4096)
            if not data: raise RuntimeError('serial port disconnected')
            transcript.extend(data);result.extend(data)
            sys.stdout.buffer.write(data);sys.stdout.buffer.flush()
            if result.endswith(b'> ') and (required is None or required in result):
                return result.decode('ascii',errors='replace')
    raise TimeoutError('UART prompt timeout; check UART port, loaded ELF and board initialization')
try:
    attrs=termios.tcgetattr(fd)
    attrs[0]=0;attrs[1]=0;attrs[2]=termios.CS8|termios.CREAD|termios.CLOCAL;attrs[3]=0
    attrs[4]=attrs[5]=termios.B115200
    attrs[6][termios.VMIN]=0;attrs[6][termios.VTIME]=0
    termios.tcsetattr(fd,termios.TCSANOW,attrs)
    termios.tcflush(fd,termios.TCIFLUSH)
    send(b'\rhelp\r');read_prompt(10,required=b'Commands:')
    for command in commands:
        print(f'\nSending: {command}',flush=True)
        send(command.encode('ascii')+b'\r')
        reply=read_prompt(args.timeout)
        if 'FAIL' in reply or 'ERROR:' in reply: raise RuntimeError('board verifier reported failure')
        if command.split()[0] in ('v','cpu','quick','stress') and ' PASS ' not in reply and ' PASS\r' not in reply:
            raise RuntimeError('command returned without a PASS marker')
    print('\nPASS: requested UART commands completed')
finally:
    args.log.parent.mkdir(parents=True,exist_ok=True)
    args.log.write_bytes(transcript)
    termios.tcsetattr(fd,termios.TCSANOW,old);os.close(fd)
