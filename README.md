# Instructions

## HLS Files

Build the all HLS IPs by running `build_IPs.sh` in `vitis_hls` and following the prompts (Yes to all to extract exported files directly)

## VIVADO Project

1. Generate the Vivado project by running `generate_vivado_prj.sh` in `vivado`
2. Make changes as necessary, export your hardware file.

### Address Map

| Accelerator | Interface | Address |
|:---:|:---:|:---:|
| fft_0_dma    | AXI_LITE    | 0x8000_0000 |
| fft_0        | config_gpio | 0x8002_0000 |
| fft_0        | reset_gpio  | 0x8001_0000 |
| ZIP_0_dma    | AXI_LITE    | 0x8003_0000 |
| ZIP_0        | axi_control | 0x8004_0000 |
| ZIP_0        | reset_gpio  | 0x8005_0000 |

## Petalinux

1. Update the hardware configuration for petalinux with `petalinux-config --get-hw-description=../vivado/your_new_hw.xsa`
2. Run `petalinux-build`
3. Run `petalinux-package --boot --u-boot --fpga` use `--force` to overwrite files that already exists
4. Copy `boot.scr`, `BOOT.BIN`, and `image.ub` under `petalinux/images/linux` to the `BOOT` partition of the SD card.

## SD Card Image

Pre-built petalinux image files `boot.scr`, `BOOT.BIN`, and `image.ub` are available under `SDCard`. 