# Instructions

The accelerator in this design is `program_manager_top`: a PE-array program
manager that dispatches RUN/RESET commands across an 8x8 grid of processing
elements and reports occupancy as a busy bitmap. It is an AXI-Lite-only,
free-running (`ap_ctrl_none`) HLS kernel — no DMA, no stream ports, no DDR
access — driven from Linux userspace through a /dev/mem mapping.

The design sources live in `vitis_hls/program_manager/` — this project is
self-contained and needs nothing outside the repo.

## Design structure

Two layers, with the boundary at the AXI-Lite command bus:

| File | Owns |
| --- | --- |
| `src/program_manager.cpp` | AXI-Lite registers, the command bus, edge detection, and all targeting |
| `include/dispatch.hpp` | address decode: is this command for this PE, and does the named bank hold the kernel |
| `include/layout_config.h` | the power-up default cluster/residency layout — manager-side only |
| `include/pe_unit.hpp` | one PE: a countdown timer, with no knowledge of the grid |
| `include/grid_utils.hpp` | busy-bitmap packing |

A PE receives `start` / `clear` / `cycles` and nothing else, so it can be
synthesized and tested on its own. It is a real RTL submodule (`INLINE off`);
build with `PE_FLAT=1` to inline it back into the manager.

## Cluster layout and kernel residency

Neither which PEs belong to which cluster nor which kernel sits in which bank is
compiled into the bitstream. The manager holds both as runtime tables that
`pm-app` uploads from `/etc/pm-layout.conf` at startup, so re-clustering the grid
and moving kernels around it are a text edit and a restart — no Vitis HLS, no
Vivado, no new bitstream.

```
grid 8 8

cluster 0  at 0 0  size 4 8  kernels 101 102
cluster 1  at 4 0  size 4 8  kernels 201 202 203

kernel 101 instr 256
kernel 102 instr 256 image /lib/firmware/k102.bin
```

A cluster is one or more rectangles given as origin + size; repeat a line with the
same id to build a non-rectangular cluster. `kernels` is **positional and
enforced**: the first id goes in bank 0 of every PE in the rectangle, the second
in bank 1, and so on, with `0` leaving a slot empty. A `RUN` naming an empty bank
is refused. Ids run 1..255 — the residency table is one byte per bank.

`kernel <id> instr <n> [image <path>]` says what to actually stream into the array
for that kernel at startup: `n` generated dummy 64-bit VLIW words, or the first
`n` words of a file. A kernel with no such line is declared in the table but
nothing is transferred for it.

Any PE that no rectangle covers is a **spare**: it belongs to no cluster, matches
no cluster dispatch, and refuses every `RUN`. Overlapping rectangles from
different clusters are a parse error rather than last-one-wins.

The file above reproduces the layout that used to be compiled in, which is also
what `layout_config.h` still provides as the power-up default — so the hardware is
usable before any host has talked to it, and a first boot with the shipped file
behaves exactly as it did before the tables existed.

### Running a kernel

Two cases, and the hardware knows which is which:

```
pm> where 202              # which PEs hold kernel 202, straight from the table
pm> run k 202 1 10         # resident: every PE in cluster 1 starts from its own slot
pm> load 204 instr 512 c 1 bank 3 run 10    # not resident: stream it in, then start
pm> banks 0 0              # what one PE holds, read back from hardware
pm> status                 # the whole array: clusters, residency, what is running
```

`run k` names a kernel rather than a slot, so PEs in one cluster need not agree on
where they keep it — each finds its own. `load` reads the array status first
(occupancy map plus the residency query), picks a free slot, streams the
instructions, and with `run <sec>` the manager commits the tag and starts the PEs
in the same cycle. Placement and eviction policy live in `pm-app`; the hardware
only enforces what it was told.

`status` is the one that answers "what is the array doing right now": it sweeps
all 256 table entries and prints the cluster map, the running kernel per PE, and
residency grouped by identical bank contents — all read from hardware, so it owes
nothing to the host's mirror. The manager latches the kernel id when a PE accepts
work, which is what makes the running column possible at all: the countdown alone
says a PE is busy, not with what.

Also in the shell: `layout` prints the parsed *file* (including preload sources,
which hardware cannot know — so `layout` vs `status` is intent vs truth),
`load-layout <file>` re-reads and re-uploads without restarting, `abort` tears
down a transfer whose stream never ended. `pm-app -c <file>` overrides the default
path.

### The instruction path

Instructions are 64-bit VLIW words and do **not** travel over AXI-Lite. They reach
the manager on an AXI4-Stream slave port fed by an `axi_dma` in MM2S simple mode,
which `pm-app` drives directly over `/dev/mem` — four registers, no scatter-gather
descriptors, no kernel driver. The source buffer comes from `u-dma-buf`
(`/dev/udmabuf0`) when the module is loaded, and otherwise from the reserved
window declared in `system-user.dtsi`.

The manager counts the beats and discards them: the PEs have no instruction memory
yet. What is real is everything around that — the DMA, the stream, the declared
instruction count, and the residency commit. A transfer is opened by `MODE_LOAD`
(which is refused outright if any targeted PE is busy), ends on the DMA's `TLAST`,
and commits the tag only if the beat count matches the count declared up front. A
short transfer leaves the bank empty rather than claiming a kernel that was never
fully delivered. When a bank becomes a real memory, the change is to route the
beat into it instead of dropping it; the protocol around it stays.

Because the loop consumes at most one beat per iteration with a **non-blocking**
read, a multi-thousand-word transfer never stalls a PE countdown — which is why
this is a stream port and not an AXI master doing bursts.

Grid **size** is still a build knob, because `DIM_X`/`DIM_Y` size the hardware's
register arrays and the unrolled dispatch loop:

```
make csim GRID_DIM=16      # functional check at 16x16
make ip   GRID_DIM=16      # synthesize it
make layout-header GRID_DIM=16   # regenerate pm-app's pm_layout.h to match
```

Changing it re-runs HLS and needs a new bitstream. A layout file partitions the
compiled grid; it cannot grow it, which is why its `grid` line is checked rather
than obeyed. `pm_layout.h` is generated and committed because bitbake builds
`pm-app` out of its own `files/` directory and cannot reach across the tree.

## HLS Files

Fast functional check first (plain g++, seconds — it needs only the Vitis
`include/` directory, for `hls_stream.h` and `ap_axi_sdata.h`, which the Makefile
locates automatically or you can point at with `HLS_INC=`):

```
cd vitis_hls
make test     # single-PE unit test, then the whole-grid simulation
```

Then build all HLS IPs by running `build_IPs.sh` in `vitis_hls` and following the prompts (Yes to all to extract exported files directly) — or `make ip`, which is the same thing.

This must be done **before** generating the Vivado project: the block design
instantiates `xilinx.com:hls:program_manager_top:1.0` from `vitis_hls/IPs`, and
that directory is gitignored, so it does not exist in a fresh clone.

## VIVADO Project

1. Generate the Vivado project by running `generate_vivado_prj.sh` in `vivado`
2. Make changes as necessary, export your hardware file.

### Address Map

| Accelerator | Interface | Address |
|:---:|:---:|:---:|
| program_manager_top_0 | s_axi_control | 0x4001_0000 |
| axi_dma_0 | S_AXI_LITE | 0x4040_0000 |

`axi_dma_0` is the instruction path: its `M_AXIS_MM2S` drives
`program_manager_top_0/instr_in`, and its `M_AXI_MM2S` reads DDR through the
existing SmartConnect. Both addresses are duplicated in
`recipes-apps/pm-app/files/pm-app.c` and `dma.h`; keep them in step.

Other peripherals: `axi_gpio_0` (LEDs) 0x4000_0000, `axi_uartlite_0` 0x4060_0000,
`axi_iic_0` 0x4080_0000, `axi_intc` 0x4120_0000, DDR4 ctrl 0x4130_0000,
`axi_timer_0` 0x41C0_0000, DDR4 0x8000_0000. The top 1 MiB of DDR
(0xFF00_0000) is reserved for the instruction staging buffer.

## Petalinux

1. Update the hardware configuration for petalinux with `petalinux-config --get-hw-description=../vivado/your_new_hw.xsa`
2. Run `petalinux-build`
3. Run `petalinux-boot --qemu --kernel` to test with Qemu before programming the board
4. Run `petalinux-boot --jtag --fpga --kernel` to program the board over JTAG.
petalinux-boot --jtag --fpga --kernel   --after-connect 'targets -set -filter {name =~ "xcvu37p"}'

The `pm-app` userspace application (`project-spec/meta-user/recipes-apps/pm-app/`)
drives the accelerator. Enable it with `petalinux-config -c rootfs` under
"user packages" before building; the rootfs is INITRD-based with no persistent
storage, so it has to be baked into the image.
