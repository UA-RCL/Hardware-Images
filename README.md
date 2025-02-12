# Instructions

## HLS Files

Build the all HLS IPs by running `build_IPs.sh` in `vitis_hls` and following the prompts (Yes to all to extract exported files directly)

## VIVADO Project

1. Generate the Vivado project by running `generate_vivado_prj.sh` in `vivado`
2. Make changes as necessary, export your hardware file.

### Address Map

| Accelerator | Interface | Address |
|:---:|:---:|:---:|
| fft_0_dma    | AXI_LITE    | 0x41E2_0000 |
| fft_0        | config_gpio | 0x4002_0000 |
| fft_0        | reset_gpio  | 0x4003_0000 |
| fft_1_dma    | AXI_LITE    | 0x41E3_0000 |
| fft_1        | config_gpio | 0x4004_0000 |
| fft_1        | reset_gpio  | 0x4005_0000 |
| ZIP_0_dma    | AXI_LITE    | 0x41E1_0000 |
| ZIP_0        | axi_control | 0x4000_0000 |
| ZIP_0        | reset_gpio  | 0x4006_0000 |
| ZIP_1_dma    | AXI_LITE    | 0x41E0_0000 |
| ZIP_1        | axi_control | 0x4001_0000 |
| ZIP_1        | reset_gpio  | 0x4007_0000 |

## Petalinux

1. Update the hardware configuration for petalinux with `petalinux-config --get-hw-description=../vivado/your_new_hw.xsa`
2. Update Ethernet MAC address as needed from [config](https://github.com/UA-RCL/ZCU102-2020.2/blob/2ffts_2mmults_1conv2d_1zip/petalinux/project-spec/configs/config#L91)
3. Run `petalinux-build`
4. Run `petalinux-package --boot --fsbl --u-boot --fpga` use `--force` to overwrite files that already exists
5. Copy `boot.scr`, `BOOT.BIN`, and `image.ub` under `petalinux/images/linux` to the `BOOT` partition of the SD card.

## SD Card Image

Pre-built petalinux image files `boot.scr`, `BOOT.BIN`, and `image.ub` are available under `SDCard`. 