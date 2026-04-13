# Instructions

## HLS Files

Build the all HLS IPs by running `build_IPs.sh` in `vitis_hls` and following the prompts (Yes to all to extract exported files directly)

## VIVADO Project

1. Generate the Vivado project by running `generate_vivado_prj.sh` in `vivado`
2. Make changes as necessary, export your hardware file.

### Address Map

| Accelerator | Interface | Address |
|:---:|:---:|:---:|
| fft_0 | s_axi_control | 0x4001_0000 |
| fft_0 | reset_gpio | 0x4003_0000 |
| fft_0_dma | S_AXI_LITE | 0x4002_0000 |

## Petalinux

1. Update the hardware configuration for petalinux with `petalinux-config --get-hw-description=../vivado/your_new_hw.xsa`
2. Run `petalinux-build`
3. Run `petalinux-boot --qemu --kernel` to test with Qemu before programming the board
4. Run `petalinux-boot --jtag --fpga --kernel` to program the board over JTAG. 
