# KPACT prototype — capabilities

What the PE-array program manager can do as of this build. An 8×8 grid whose
clustering, kernel residency and instruction delivery are all runtime concerns,
with nothing about the layout compiled into the bitstream.

| | |
| --- | --- |
| Grid | 64 PEs, 8×8, fixed at synthesis |
| Residency table | 256 kernel tags — 4 banks × 64 PEs, one byte each |
| Instruction bus | 64-bit AXI4-Stream, one VLIW word per beat |
| Dispatch tick | 33.3 MHz (100 MHz fabric at II=3) |
| Max transfer | 131,072 instructions (1 MiB staging buffer) |

**Evidence column.** Every capability carries the strongest evidence that
actually exists for it:

- **board** — observed running on hardware
- **csim** — passes in `make csim`, not yet run on the board
- **untested** — implemented, never exercised

Most of the system is verified in simulation only. The board has so far
confirmed the register map, dispatch, and one real DMA transfer.

---

## Data paths

```
MicroBlaze ──── AXI-Lite commands + readback (0x4001_0000) ────┐
  pm-app                                                       │
                                                    ┌──────────▼─────────────┐
DDR buffer ──► AXI DMA ──── 64b AXIS + TLAST ──────►│ program_manager_top    │
 1 MiB          MM2S                                │  free-running          │
 contiguous     0x4040_0000                         │                        │
                                                    │ cfg_cluster[8][8]      │
                                                    │ cfg_kernel[8][8][4]    │
                                                    │                        │
                                                    │ beats counted, then    │
                                                    │ discarded (no IMEM)    │
                                                    └──────────┬─────────────┘
                                                    start/clear │
                                                    ┌──────────▼─────────────┐
                                                    │ 64 PE countdown timers │
                                                    └────────────────────────┘
```

Commands and readback go over AXI-Lite; instructions over a separate stream.
The stream is consumed one beat per loop iteration with a **non-blocking** read,
which is why a multi-thousand-word transfer never stalls a PE countdown.

---

## Dispatch — hardware

Every PE is a countdown timer that knows nothing about the grid. The manager
decides, per PE per cycle, whether a command applies and whether it is legal.

| Capability | What it does | Evidence |
| --- | --- | --- |
| Run one PE | `run p x y bank sec` — targets by position, ignoring cluster membership entirely | board |
| Run a cluster | `run c cluster bank sec` — one command fans out to every PE in the cluster | board |
| Run by kernel id | `run k kernel cluster sec` — each PE searches its own four tags and starts from whichever slot holds the kernel. PEs in one cluster need not agree on the slot | csim |
| Reset everything | `reset` issues one `CLUSTER_ALL` command reaching every PE, spares included, whatever the cluster ids are | csim |
| Occupancy map | 64-bit busy bitmap in two words; `map` prints it, and accept/refuse is inferred by sampling either side of the strobe | board |
| Refuse an empty bank | A RUN naming a bank holding no kernel is silently refused — the enforcement behind the whole residency table | board |
| Spare PEs | A PE no rectangle covers belongs to nothing: matches no cluster dispatch, refuses every RUN, until something is loaded onto it directly | board |
| Independent countdowns | A busy PE ignores new starts, and no command — reset, config, or a load elsewhere — perturbs a countdown it wasn't addressed to | csim |

## Runtime tables — hardware

Neither cluster membership nor kernel residency is compiled into the bitstream.
Both are register tables the host uploads at startup, so re-clustering the grid
is a text edit and a restart.

| Capability | What it does | Evidence |
| --- | --- | --- |
| Cluster membership | One byte per PE. Up to 64 cluster ids, plus reserved `CLUSTER_SPARE` (255) and `CLUSTER_ALL` (254) | board |
| Kernel residency | 256 one-byte tags — a kernel id per bank per PE, 0 meaning empty. Ids run 1–255 | board |
| Atomic publish | Entries are written one at a time, then a single commit swings the whole grid over. The array never dispatches against a half-written table | csim |
| Usable with no host | Until the first commit both tables fall back to the layout compiled into `layout_config.h`, so a freshly configured bitstream works before anything talks to it | board |
| Per-entry readback | `banks x y` reads a PE's four tags, cluster, resident count, busy bit and in-load flag from hardware — how to tell a host-mirror bug from a real one | csim |
| Kernel→PE query | `where kernel` returns a bitmap of every PE holding that kernel in any bank, in one read instead of walking 256 tags | csim |
| Running kernel per PE | The manager latches the kernel id when a PE accepts work — from the bank's tag, the named kernel, or an auto-start — so "what is executing right now" is answerable from hardware and survives a host restart | csim |
| Whole-array sweep | `status` reads all 256 entries and prints clusters, running kernels and residency grouped by identical bank contents — 640 transactions, a few ms | csim |
| Live reconfiguration | Membership and residency can be rewritten while other PEs are mid-countdown; a config write cannot disturb work in flight | csim |

## Instruction delivery — AXI-Stream

Instructions are 64-bit VLIW words on a dedicated stream port fed by an AXI DMA,
not AXI-Lite writes. The manager counts beats and discards them — the PEs have
no instruction memory yet — but the transport, framing and residency commit
around that are real.

| Capability | What it does | Evidence |
| --- | --- | --- |
| DMA'd instruction stream | MM2S simple mode driven from userspace over `/dev/mem` — four registers, no scatter-gather, no kernel driver, no interrupt | board |
| Load to one PE | `load K instr N p x y` — works on any PE including a spare, which then becomes runnable by position | csim |
| Broadcast to a cluster | `load K instr N c 1` — one transfer delivers to all 32 PEs and commits 32 tags | board |
| Declared count, checked | The instruction count is sent up front and compared against TLAST. Ending early sets *short*; overrunning sets *overrun* | csim |
| All-or-nothing commit | A failed transfer commits no tag, leaving the bank empty — the failure surfaces as "kernel not resident" rather than a PE running half a program | csim |
| Busy targets refused | If any targeted PE is busy the transfer never opens and no tag is touched, so a broadcast can never half-apply | csim |
| Bank hidden while loading | Opening a transfer clears the destination tag, so a bank being written reads as empty and cannot be dispatched to mid-flight | csim |
| Commit and start together | `run <sec>` on a load makes the manager publish the tag and assert start in the same cycle — no window for another command to claim the PE | csim |
| Non-blocking consumption | One beat per loop iteration, read outside the PE loop, so a 65,536-word transfer leaves every countdown ticking normally | csim |
| Content check | Generated streams carry `(0xD0000000|k) << 32 | i` and the manager reports the last beat, so a dropped or duplicated word is visible, not just a wrong count | board |
| Abort | `abort` tears down a transfer whose stream never ended, releasing the PEs it was holding | csim |

## Layout file and shell — host

Placement and eviction policy live in `pm-app`; the hardware only enforces what
it was told. The layout file is read at startup from `/etc/pm-layout.conf`, or
from `-c <file>`.

| Capability | What it does | Evidence |
| --- | --- | --- |
| Clusters as rectangles | `cluster 0 at 0 0 size 4 8` — origin plus size. Repeat an id to build a non-rectangular cluster from several rectangles | board |
| Residency manifest | `kernels 101 102` is positional: first id to bank 0, second to bank 1, `0` leaves a slot empty | board |
| Preload sources | `kernel 101 instr 256` streams a generated pattern at boot; add `image <path>` to send a real file. Omit the line for a tag with no transfer | board |
| Validated parsing | Grid mismatch, out-of-bounds rectangles, overlapping clusters, bad kernel ids and count disagreements are each rejected with a line-numbered message before anything reaches the bus | board |
| Reload without restart | `load-layout <file>` re-parses, re-uploads and re-preloads, refusing while any PE is busy | untested |
| Automatic placement | Omit `bank` and `pm-app` probes the PE's slots in hardware, takes the first free one and says so, falling back to evicting bank 0 | csim |
| Upload verification | All 256 entries are read back after commit and mismatches reported, so a silent table failure cannot masquerade as a dispatch bug | board |
| Whole-array status | `status` prints the cluster map, the running kernel per PE, and residency grouped by identical bank contents, all read from hardware — `layout` stays as the parsed-file view, so the two are intent vs truth | csim |
| Bus self-test | A register write/read pattern plus a check that the readback register tracks `pe_x`, which catches stale register offsets after an HLS change | board |

## Build and verification — toolchain

| Capability | What it does | Evidence |
| --- | --- | --- |
| Tool-free simulation | `make csim` and `make pe-test` build the real HLS sources with plain g++ in seconds, needing only the Vitis `include/` directory | board |
| Regression gate | Phase 1 of the testbench issues no config command, so its output must stay byte-identical to the pre-table design — which it does | board |
| Resizable grid | `GRID_DIM` and `MAX_BANKS` sweep the array at build time, with `make layout-header` keeping `pm-app` in step | untested |
| PE as a real submodule | The PE synthesizes as its own RTL module and is unit-tested alone; `PE_FLAT=1` inlines it back into the manager as a fallback | board |

---

## What it cannot do yet

Named explicitly, because several of these look like capabilities from outside.

- **PEs have no instruction memory.** Delivered instructions are counted and
  dropped. A PE is still a countdown timer; residency is an access-control claim,
  not a fact about stored program bits. Routing the beat into a bank is the next
  real step.
- **Grid size is fixed at synthesis.** `DIM_X`/`DIM_Y` size the register arrays
  and the unrolled dispatch loop. A layout file partitions the compiled grid; it
  cannot grow it.
- **No cluster editing from the shell.** The hardware can set any PE's membership
  at runtime, but nothing exposes it. Changing membership means editing the file
  and reloading the whole table.
- **Cluster-wide loads exclude spares.** Broadcast targeting skips spares by
  design, and `CLUSTER_ALL` is rejected by the host before it reaches a load.
  Spares must be loaded by position.
- **Four banks, 255 kernel ids.** Both are structural: `MAX_BANKS` is a synthesis
  constant and the residency table is one byte per bank.
- **No remaining-time readback.** `status` shows *what* each PE is running but not
  how much of its countdown is left; the cycle counter is not exposed.
- **Polling only.** No interrupt anywhere — not from the manager, not from the
  DMA. Completion is discovered by reading status registers.

---

## Before trusting this on hardware

The design currently packaged in `vitis_hls/IPs/` was built **before** the timing
fix and misses the 100 MHz target at 18.8 ns. The corrected sources close at
7.205 ns against a 7.30 ns budget — roughly 95 ps of margin, which is thin — so
`make ip` must be re-run, the Vivado IP cache cleared (the VLNV is unchanged at
`program_manager_top:1.0`), and the bitstream rebuilt before any of the `csim`
rows above can be confirmed on the board.

Post-synthesis estimates for the corrected design: 46,958 LUT, 10,942 FF, no
BRAM or DSP, dispatch loop at II=3. These are HLS estimates, not
post-implementation results.
