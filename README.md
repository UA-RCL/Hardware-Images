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
| `include/dispatch.hpp` | address decode: is this command for this PE, and is the bank legal |
| `include/layout_config.h` | the compiled-in cluster/bank layout — manager-side only |
| `include/pe_unit.hpp` | one PE: a countdown timer, with no knowledge of the grid |
| `include/grid_utils.hpp` | busy-bitmap packing |

A PE receives `start` / `clear` / `cycles` and nothing else, so it can be
synthesized and tested on its own. It is a real RTL submodule (`INLINE off`);
build with `PE_FLAT=1` to inline it back into the manager.

Grid size is a build knob rather than a source edit:

```
make csim GRID_DIM=16      # functional check at 16x16
make ip   GRID_DIM=16      # synthesize it
make layout-header GRID_DIM=16   # regenerate pm-app's pm_layout.h to match
```

Changing it re-runs HLS and needs a new bitstream. `pm_layout.h` is generated and
committed because bitbake builds `pm-app` out of its own `files/` directory and
cannot reach across the tree.

## HLS Files

Fast functional check first (plain g++, no Xilinx tools, seconds):

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

Other peripherals: `axi_gpio_0` (LEDs) 0x4000_0000, `axi_uartlite_0` 0x4060_0000,
`axi_iic_0` 0x4080_0000, `axi_intc` 0x4120_0000, DDR4 ctrl 0x4130_0000,
`axi_timer_0` 0x41C0_0000, DDR4 0x8000_0000.

## Petalinux

1. Update the hardware configuration for petalinux with `petalinux-config --get-hw-description=../vivado/your_new_hw.xsa`
2. Run `petalinux-build`
3. Run `petalinux-boot --qemu --kernel` to test with Qemu before programming the board
4. Run `petalinux-boot --jtag --fpga --kernel` to program the board over JTAG.

The `pm-app` userspace application (`project-spec/meta-user/recipes-apps/pm-app/`)
drives the accelerator. Enable it with `petalinux-config -c rootfs` under
"user packages" before building; the rootfs is INITRD-based with no persistent
storage, so it has to be baked into the image.
