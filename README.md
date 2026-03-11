# Instructions

## HLS Files

Build the all HLS IPs by running `build_IPs.sh` in `vitis_hls` and following the prompts (Yes to all to extract exported files directly)

## VIVADO Project

1. Generate the Vivado project by running `generate_vivado_prj.sh` in `vivado`
2. Make changes as necessary, export your hardware file.

### Address Map

| Accelerator | Interface | Address |
|:---:|:---:|:---:|
| conv2d_0 | s_axi_control | 0x8000_0000 |
| conv2d_0 | reset_gpio | 0x8001_0000 |
| conv2d_0_dma | S_AXI_LITE | 0x8006_0000 |
| conv2d_0_dma1 | S_AXI_LITE | 0x800C_0000 |
| ReLU_0 | s_axi_control | 0x8002_0000 |
| ReLU_0 | reset_gpio | 0x8008_0000 |
| relu_0_dma | S_AXI_LITE | 0x8007_0000 |
| ZIP_HLS_accel_0 | s_axi_ctrl | 0x8004_0000 |
| ZIP_HLS_accel_0 | reset_gpio | 0x8005_0000 |
| zip_0_dma | S_AXI_LITE | 0x8003_0000 |
| MaxPoolingKernel_0 | s_axi_config | 0x8009_0000 |
| MaxPoolingKernel_0 | reset_gpio | 0x800B_0000 |
| maxpool2d_0_dma | S_AXI_LITE | 0x800A_0000 |

## Petalinux

1. Update the hardware configuration for petalinux with `petalinux-config --get-hw-description=../vivado/your_new_hw.xsa`
2. Run `petalinux-build`
3. Run `petalinux-package --boot --u-boot --fpga` use `--force` to overwrite files that already exists
4. Copy `boot.scr`, `BOOT.BIN`, and `image.ub` under `petalinux/images/linux` to the `BOOT` partition of the SD card.

## SD Card Image

Pre-built petalinux image files `boot.scr`, `BOOT.BIN`, and `image.ub` are available under `SDCard`. 
