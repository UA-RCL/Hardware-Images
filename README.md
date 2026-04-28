# Instructions

## HLS Files

Build the all HLS IPs by running `build_IPs.sh` in `vitis_hls` and following the prompts (Yes to all to extract exported files directly)

## VIVADO Project

1. Open a block design, enable GEM and MDIO from I/O configuration for Zynq IP to enable ethernet.

2. Add Accelerator IP's enable M-AXIs and S-AXIs from Zynq IP

## Petalinux

0. Create project with `petalinux-create project -n <myproj> --template zynqMP`
1. Update the hardware configuration for petalinux with `petalinux-config --get-hw-description=../vivado/your_new_hw.xsa`
2. Enable C++ runtime libraries from Yocto interface with `petalinux-config -c rootfs` or `petalinux-config -c kernel`
3. Add u-dma kernel module sources to `my_proj/project-spec/meta-user/recipes-modules` and add udmabuf's to device tree under `my_proj/project-spec/meta-user/recipes-bsp/device-tree/files/system-user.dtsi`
4. Run `petalinux-build`
5. Run `petalinux-package --boot --u-boot --fpga` use `--force` to overwrite files that already exists
6. Copy `boot.scr`, `BOOT.BIN`, and `image.ub` under `petalinux/images/linux` to the `BOOT` partition of the SD card.
7. For cross-compilation for the ARM cores, Petalinux SDK can be used. Run following commands to use SDK: `petalinux-build --sdk` and `petalinux-package sysroot -s|--sdk <custom sdk path> -d|--dir <custom directory path>`, cross compilation can be done afterwards by sourcing the environment under created SDK folder.

## SD Card Image

Pre-built petalinux image files `boot.scr`, `BOOT.BIN`, and `image.ub` are available under `SDCard`. 
