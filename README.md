# AUP-ZCU3-1vmm-ranc

This branch provides one RANC spiking vector–matrix multiplication accelerator
connected to the Zynq UltraScale+ processing system on the AUP board
(`xczu3eg-sfvc784-2-e`). It includes the hardware sources, self-checking RTL
tests, routed hardware handoff, bare-metal verifier, PetaLinux configuration,
Linux verifier and prebuilt SD-card boot files.

The processing-system preset comes from the UA-RCL Hardware-Images AUP baseline.
Its FFT/ZIP accelerator path is replaced by the RANC network and a DMA/GPIO
interface. Use **Vivado/XSim 2024.2, Vitis 2024.2 and PetaLinux 2024.2**.
Package prepared 2026-10-28.

## What operation does it perform?

The application supplies a signed five-element integer row vector `x`. The
accelerator computes eight signed outputs, `y = x A`, using this fixed matrix:

```text
A = [ 234  -83 -242 -162  225   44  -10   85
     -216   21   11 -134  -93 -141   30 -186
     -149  196 -139 -160   52   13   44 -101
      144 -163  215 -118   56  -31 -106  189
      -16  246   65  196  159  -87  250  246 ]
```

For example:

```text
x = [0, 2, -3, 0, 2]
y = [-17, -54, 569, 604, -24, -495, 428, 423]
```

Each output is `y[j] = sum(x[i] * A[i][j], i=0..4)`.
The supplied applications accept each input in **[-15,15]**. The vector can change
at runtime. Changing the matrix requires regenerating the neural mapping,
initialization memories and software reference, then rebuilding the hardware.

RANC executes a spiking neural mapping of this operation rather than a direct
array of integer multipliers. This package uses the **Fair VMM mapping**:
splitter, representation/feedback, bit-weighting and summation/feedback stages.
The reference C code independently models the spikes, while direct CPU matrix
multiplication checks the final arithmetic answer.

## Repository organization

| Path | Purpose |
| --- | --- |
| `rtl/` | Verilog used by this design, including RANC and AXI integration |
| `tb/` | Self-checking SystemVerilog testbenches |
| `data/` | Core initialization memories, input packets and expected spike traces |
| `vivado/` | AUP PS preset, project creation and implementation Tcl |
| `build/ranc_aup.xsa` | Shipped routed XSA, including the FPGA bitstream |
| `scripts/` | RTL test runners, hardware-handoff checks and capture checker |
| `vitis/src/` | Bare-metal UART verifier and portable C reference/mapping |
| `vitis/handoff/ranc_aup.xsa` | Hardware input to the Vitis platform builder |
| `vitis/prebuilt/ranc_uart_verify.elf` | Prebuilt A53 bare-metal executable |
| `vitis/scripts/`, `vitis/build.sh`, `vitis/run_jtag.sh` | Platform/application build and JTAG launch |
| `petalinux/project-spec/` | OS configuration, hardware description, device tree and recipes |
| `petalinux/handoff/` | Matching XSA for Linux hardware import |
| `linux/` | Linux userspace verifier source and cross-build helper |
| `SDCard/BOOT/` | Prebuilt `BOOT.BIN`, `boot.scr` and `image.ub` |
| `SDCard/rootfs/rootfs.tar.gz` | Prebuilt Linux root filesystem for partition 2 |
| `SHA256SUMS` | Checksums of packaged hardware, sources, reference data and SD artifacts |
| `AUP-ZCU3-1vmm-ranc.bsp` | PetaLinux project archive with configuration, recipes and XSA |

`linux/` and `vitis/` are two alternative software environments for the same
accelerator. The Linux verifier accesses UIO and u-dma-buf devices. The Vitis
application uses a standalone BSP and bare-metal drivers. `petalinux/` builds
the operating system that runs the Linux verifier; it is not another verifier.
Both applications share the portable reference files under `vitis/src/`.

Generated workspaces, logs and caches are excluded. `data/unit_vector_x4/` is a
passing regression for `[0,0,0,0,1]`, retained to detect the earlier short-burst
input bug. `data/vector_a/` and `data/vector_b/` support consecutive-vector tests.

## Hardware and connections

### Address map

These are PS physical addresses assigned by `vivado/create_project.tcl`, shared
by the bare-metal and Linux drivers. Both peripherals have a 64 KiB aperture.

| Peripheral | Base | End | Purpose |
| --- | --- | --- | --- |
| RANC AXI DMA | `0x80000000` | `0x8000FFFF` | DDR-to-RANC MM2S and RANC-to-DDR S2MM |
| RANC AXI GPIO | `0x80010000` | `0x8001FFFF` | Run/reset/tick limit output and hardware status input |

| Register | Absolute address | Access / use |
| --- | --- | --- |
| MM2S_DMACR | `0x80000000` | DMA transmit control |
| MM2S_DMASR | `0x80000004` | DMA transmit status/errors |
| MM2S_SA / SA_MSB | `0x80000018` / `0x8000001C` | Input buffer physical address |
| MM2S_LENGTH | `0x80000028` | Input bytes; writing starts the transfer |
| S2MM_DMACR | `0x80000030` | DMA receive control |
| S2MM_DMASR | `0x80000034` | DMA receive status/errors |
| S2MM_DA / DA_MSB | `0x80000048` / `0x8000004C` | Output buffer physical address |
| S2MM_LENGTH | `0x80000058` | Receive buffer bytes; writing arms reception |
| GPIO channel 1 DATA | `0x80010000` | Write RANC control word |
| GPIO channel 2 DATA | `0x80010008` | Read RANC status word |

The RANC RTL itself has no separate AXI-Lite address aperture. Its control/status
signals connect through GPIO, and its spike packets connect through DMA streams.
DDR buffer addresses are allocated separately; Linux discovers them from
u-dma-buf sysfs and must not substitute a userspace virtual address into DMA.

```text
                  register reads/writes
Zynq A53 ── HPM0_LPD ── control SmartConnect ── AXI DMA registers
                                          └── AXI GPIO control/status

DDR ── HP0_FPD ── memory SmartConnect ── AXI DMA MM2S ── RANC S_AXIS
                                    └── AXI DMA S2MM ◀── RANC M_AXIS
```

The control SmartConnect routes CPU accesses to the DMA and GPIO registers.
The memory SmartConnect joins the DMA read/write masters to the PS DDR port.
The GPIO drives RANC run/reset/tick-limit signals and reads status. It does not
carry the spike data. The DMA carries spikes between DDR and the AXI streams.

| Setting | Value |
| --- | --- |
| Network | 3 × 3 core grid, output at (2,2) |
| Capacity per core | 256 neurons, 256 axons |
| Neuron weight choices | Four |
| Potential storage | 16 bits |
| Weight/leak/threshold widths | 9 bits |
| Scheduler slots | 16 |
| PL clock | Approximately 100 MHz |
| Tick interval | 100,000 PL clocks, approximately 1 ms |
| DMA | Simple mode, 32-bit streams; no scatter-gather |
| DMA register base | `0x80000000` |
| GPIO register base | `0x80010000` |

PS `pl_clk0` clocks RANC, DMA, GPIO and both interconnects. A Processor System
Reset block supplies synchronized active-low peripheral reset. DMA MM2S/S2MM
interrupts and the RANC tick interrupt connect through a concatenator to PS IRQ0;
the provided verification applications poll the DMA.

GPIO channel 1, offset `0x00`, is the control output:

| Bits | Meaning |
| --- | --- |
| 0 | Enable automatic ticks |
| 1 | Reset wrapper controllers/FIFO |
| 31:16 | Tick limit; zero means unlimited |

GPIO channel 2, offset `0x08`, is the status input:

| Bits | Meaning |
| --- | --- |
| 0 | Tick pulse |
| 1 | Running |
| 5:2 | Sticky input/fetch/scheduler/core-controller errors |
| 6 | Capture FIFO overflow |
| 7 | Duplicate output |
| 31:16 | Tick count |

The original RTL has local compatibility/reset fixes and the input-burst
completion correction. Integration adds `ranc_vmm_axis.v` and the tick-mask
stream/FIFO. Potential width is increased to 16 bits because this mapping exceeds
the original 9-bit potential range. The shipped initialized memories are part of
the design and must accompany the RTL.

## How an operation travels through the system

1. Software computes a direct integer VMM answer and runs the C event reference
   to determine the expected output masks and a sufficient tick limit.
2. Software encodes `2 * sum(abs(x))` input spike packets. The example above
   needs 14 packets, or 56 bytes. Both input signs use the mapping's paired
   representation; a negative value does not add an extra copy beyond this rule.
3. With ticks disabled, DMA MM2S sends all packets in one input burst. TLAST marks
   the final packet. Each word contains the original 30-bit RANC packet:
   signed dx[29:21], signed dy[20:12], axon[11:4], delay[3:0]; top two bits are zero.
4. Software starts automatic ticking once, using `(tick_limit << 16) | 1`.
   It does not start a separate operation for every tick. A tick is a neural
   simulation time step, with enough physical cycles for scanning and routing.
5. Hardware records one 32-bit frame per tick, including silent ticks:
   `word = (tick_id << 16) | spike_mask`. Each frame asserts TLAST. Simple-mode
   S2MM must be rearmed per frame while the FIFO absorbs temporary delays.
6. Software compares every tick tag/mask with the event reference. Output `j`
   is the number of spikes on channel `2*j` minus those on channel `2*j+1`.
   The eight decoded values must also equal direct integer VMM.

For the example, verification uses 616 frames/ticks. Its hardware execution takes
roughly 616 ms plus transfer/drain overhead at the current tick interval.
`packets` describes input encoding, `ticks` describes the simulation duration,
and `peak` describes the reference model's maximum absolute membrane potential.
They are not interchangeable spike counts or CPU clock cycles.

The wrapper stops at the programmed tick limit. It does not independently know
when the mathematical VMM is complete. The software reference waits for pending
events and neuron potentials to drain before choosing the limit.
Soft reset resets controllers and capture state, **not initialized neuron RAM**.
Start testing with freshly loaded matching PL; after a failed/aborted hardware
run, reload PL or reboot before retrying. Fully drained successful vectors can
run consecutively.

## Choose a starting point

All host commands below start at the checkout root unless a command changes
directory. Tool scripts assume installations under `/tools/Xilinx`; adjust their
settings paths if your tools are elsewhere. Install the host prerequisites for
PetaLinux 2024.2 and use a supported build environment.

- To test shipped hardware on Linux, deploy the SD files and run the Linux verifier.
- To test bare-metal UART commands, build the Vitis platform and launch through JTAG.
- To reproduce or modify PL, run RTL tests, create the Vivado project and implement it.

Check the package before deployment:

```bash
sha256sum -c SHA256SUMS
```

Every entry should report `OK`. After editing/rebuilding files, checksums describe
the original package and should be regenerated when preparing a new release.

## RTL simulation: verify correctness before implementation

```bash
bash scripts/run_units_xsim.sh
bash scripts/run_fetch_xsim.sh
bash scripts/run_stream_xsim.sh
bash scripts/run_input_bursts_xsim.sh
bash scripts/run_smoke_xsim.sh
bash scripts/run_board_vector_xsim.sh
bash scripts/run_runtime_xsim.sh
bash scripts/run_vmm_xsim.sh
```

| Test | What it checks |
| --- | --- |
| units | Signed neuron threshold/reset behavior |
| fetch | Fetch reset, error reporting and recovery |
| stream | Tick framing and AXI backpressure |
| input_bursts | Short input bursts and accepted burst completion |
| smoke | First 25 ticks of the mapped VMM |
| board_vector | Wrapper run of `[0,0,0,0,1]` against exact expected frames/results |
| runtime | Two opposite vectors on the same initialized network |
| vmm | Full original vector: 615 computation ticks and 2,632 output spikes |

The full VMM test compares hardware computation with reference rows 2–616;
the board wrapper also captures the initial empty interval, hence 616 frames.
Each runner requires its specific PASS marker and returns nonzero on failure.
Logs are generated under `build/<test>/`. Full RTL runs can take many minutes.
These tests exercise PL with stream stimulus; they do not execute the Zynq CPU.
The board-reset test additionally requires generated Vivado simulation products.

## Create and implement the Vivado project

```bash
bash scripts/create_vivado_project.sh
source /tools/Xilinx/Vivado/2024.2/settings64.sh
cd build
vivado -mode batch -source ../vivado/build_bitstream.tcl
cd ..
```

Open `build/vivado/ranc_aup.xpr` in Vivado 2024.2. Open block design `ranc_system`
to inspect PS, DMA, GPIO and RANC connections. Project creation validates the
block design; implementation is a separate step and must finish successfully.

Implementation writes utilization/timing reports and exports `build/ranc_aup.xsa`
with bitstream after timing checks. It also stages matching XSAs under
`vitis/handoff/` and `petalinux/handoff/`.

```bash
python3 scripts/share_hardware.py
```

This checks identical hardware bytes and the embedded bitstream/register map.
After changing PL, rebuild the Vitis platform or reimport the PetaLinux handoff
before deploying the new design. The XSA is the shared contract for both flows.

## Build and run the bare-metal Vitis verifier

```bash
bash vitis/build.sh
cat vitis/build/latest.json
```

The builder uses the shipped handoff, creates an A53 core 0 standalone platform,
builds its BSP/boot components and builds the UART verifier. It uses the supplied
`fixed_mapping.h`; no external mapping generator is needed. Output metadata
records the XSA hash, workspace, platform and application ELF paths.

For GUI use, open the recorded workspace in Vitis Unified IDE 2024.2. Components
are `ranc_uart_platform` and `ranc_uart_verify`. Edit `vitis/src/` and rerun the
builder to synchronize imported sources. The prebuilt ELF is supplied for artifact
inspection/manual use; the scripted launch uses metadata from the local build.

To run on the board:

1. Power off and select the board's labeled JTAG boot position; power on.
2. Connect the PROG/UART port with a USB data cable. Open the appropriate UART
   device at **115200 baud, 8N1, no flow control**; the tested host used ttyUSB1,
   but device numbering can differ. Bare-metal stdout uses PS UART1, MIO32–33.
3. Start `hw_server` from a Vitis-configured terminal, with one target board connected.
4. From the checkout root, run `bash vitis/run_jtag.sh`.

The launch resets the system, programs the matching bitstream, applies PS/DDR
initialization, downloads the A53 ELF and starts it. At the UART prompt:

```text
info
cpu 0 2 -3 0 2
v 0 2 -3 0 2
v 0 0 0 0 1
quick
stress 10 12345
```

`cpu` runs the software reference only. `v` runs the physical accelerator and
checks it. `quick` runs a finite verification suite; `stress` runs seeded random
vectors. Require PASS for every hardware case. For the unit vector above,
expect `[-16,246,65,196,159,-87,250,246]`.

## Build PetaLinux from source

### PetaLinux BSP archive

`AUP-ZCU3-1vmm-ranc.bsp` is the PetaLinux board-support project archive, matching
the packaging style of the FFT/ZIP branches. It contains the project configuration,
recipes and hardware handoff. It is distinct from the **Vitis standalone BSP**,
which the Vitis platform builder generates for the bare-metal application.
The `.bsp` archive is not an SD boot image; use `SDCard/` for prebuilt deployment.

To create a fresh project from the archive, source PetaLinux 2024.2 settings and
run from a directory outside this checkout:

```bash
petalinux-create project -s /path/to/AUP-ZCU3-1vmm-ranc.bsp -n ranc_linux
cd ranc_linux
umask 022
petalinux-config --get-hw-description=hardware
petalinux-build
petalinux-package --boot --u-boot --fpga --force
```

Alternatively, build the checked-in `petalinux/` directory as described below.
To regenerate the archive after updating that project, source PetaLinux settings
and run from the checkout root:

```bash
bash scripts/package_bsp.sh
```

This invokes AMD's BSP packager with a local workaround for its 2024.2 attempt
to create `/build` when packaging a BSP. It changes the path only within the
packaging process, without modifying the installed tools. An existing output
archive may require moving aside before packaging again.

Source your installed PetaLinux 2024.2 `settings.sh`, then:

```bash
umask 022
cd petalinux
petalinux-config --get-hw-description=handoff
petalinux-build
petalinux-package --boot --u-boot --fpga --force
cd ..
```

Keep the imported hardware and the application address map consistent.
The configuration selects an ext4 root filesystem on `/dev/mmcblk0p2`, UART
console at 115200, generic UIO and u-dma-buf support. OpenSSH is selected, but
network access still requires working Ethernet hardware/device-tree configuration;
UART plus SD transfer is sufficient for all tests here.

Build outputs are under `petalinux/images/linux/`. Deploy `BOOT.BIN`, `boot.scr`,
`image.ub` and the matching `rootfs.tar.gz` together. Rebuilding the OS does not
automatically install `linux/ranc_linux_verify`; build and copy it separately.

## Deploy and boot the SD card

Use an SD card with a FAT boot partition and an ext4 Linux root partition.
Back up existing contents before replacing them. The prebuilt files are:

| SD destination | Package source |
| --- | --- |
| FAT partition root: `BOOT.BIN` | `SDCard/BOOT/BOOT.BIN` |
| FAT partition root: `boot.scr` | `SDCard/BOOT/boot.scr` |
| FAT partition root: `image.ub` | `SDCard/BOOT/image.ub` |
| ext4 partition root | Extract `SDCard/rootfs/rootfs.tar.gz` |

`BOOT.BIN` supplies boot firmware and PL configuration. `boot.scr` directs
U-Boot's boot sequence. `image.ub` supplies the kernel/device-tree image.
The separate ext4 rootfs supplies Linux programs, libraries and configuration.
Copying only the three boot files does not update the ext4 rootfs.

For a newly prepared or emptied ext4 partition, extract the archive as root
with ownership and permissions preserved, for example:

```bash
# Replace this mount path with the verified SD partition mount, not the host root.
sudo tar --numeric-owner -xzpf SDCard/rootfs/rootfs.tar.gz -C /path/to/SD_ROOTFS
```

Extracting over an old filesystem leaves obsolete files; use a clean destination
for reproducibility. Confirm the selected mount belongs to the intended SD card.
Copy the three boot files into the FAT partition, run `sync`, leave both mount
directories and unmount them safely. A busy mount usually means a terminal or
file manager still has it open.

With board power off, insert the card and select SD boot. Power on and use UART.
Log in as `petalinux`; the default initial password is normally `petalinux`, with
a forced password change. A previously deployed card may retain a changed
password; the password used in an earlier session is not a general board default.

## Build and run the Linux verifier

On the workstation:

```bash
bash linux/build.sh
```

This produces `linux/build/ranc_linux_verify`, a static **aarch64 Linux** executable.
The default compiler is the Vitis 2024.2 Linux cross compiler; override `CC` if needed.
It is not an x86 program or a bare-metal ELF. No Python/compiler is needed on the board.

Copy this executable onto the SD FAT partition alongside the boot files.
After booting Linux, transfer it to the user's Linux home directory:

```bash
sudo mkdir -p /mnt/sdboot
sudo mount /dev/mmcblk0p1 /mnt/sdboot
sudo cp /mnt/sdboot/ranc_linux_verify /home/petalinux/
sudo chown petalinux:petalinux /home/petalinux/ranc_linux_verify
chmod +x /home/petalinux/ranc_linux_verify
cd /home/petalinux
sudo ./ranc_linux_verify info
sudo ./ranc_linux_verify quick
sudo ./ranc_linux_verify v 1 2 3 4 4
```

Skip mounting if the partition is already mounted there. FAT access may require
sudo. The program discovers the UIO devices by address, so uio0/uio1 numbering
need not be fixed. It uses u-dma-buf contiguous memory for DMA.

`info` should locate `0x80000000` and `0x80010000`; register access alone does not
prove arithmetic correctness. `quick` must show equal `cpu_y` and `ranc_y`, a
PASS for every case, and `SUITE quick PASS cases=7` for this Linux verifier.
If UIO or buffer devices are missing, inspect `ls /dev/uio* /dev/udmabuf*` and
`dmesg`; confirm that the matching PetaLinux image/device tree is booted.

Only one application may own the accelerator/DMA at a time. Do not run the
standalone verifier concurrently with a runtime that uses those devices.

## Correctness, timing and reproducibility limits

The supplied board setup previously passed seven Linux hardware cases, including
zero, mixed signs and the short two-packet unit vector. Packaging validation
checked matching XSAs, artifact hashes and Linux cross compilation. Cleanup did
not rerun the full RTL, implementation or physical board tests; reproduce them
with the commands above before claiming a new board/build result.

Current latency is dominated by approximately 1 ms per neural tick. Direct CPU
matrix multiplication and a CPU spiking simulator perform different amounts of
work; label comparisons accordingly. For faster design exploration, use the
RTL testbenches to evaluate tick spacing and packet correctness before rebuilding
the bitstream. A timing report establishes achievable clock timing, not the full
input-dependent operation latency.

This package contains standalone verification software, not a SEQUOIA runtime
distribution. SEQUOIA's SPIKEVMM backend can target the same DMA/GPIO protocol,
but its runtime, application shared object and scheduler configuration are built
in the SEQUOIA repository. Keep the hardware protocol and mapping synchronized.

The source repository for the RANC architecture is
[UA-RCL/RANC](https://github.com/UA-RCL/RANC). The Hardware-Images baseline layout
is [AUP-ZU3-1fft1zip](https://github.com/UA-RCL/Hardware-Images/tree/AUP-ZU3-1fft1zip).
This branch is local until committed and published; no GitHub push is implied.
