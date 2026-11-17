# Instructions

This repository is a base image for the VCU128 board. It provides a MicroBlaze-based design with Ethernet support and the matching PetaLinux project. Add your own designs to the Vivado project and build on top of it.

## HLS Files

Place your HLS files in `vitis_hls`, then build all HLS IPs by running `build_IPs.sh` in that directory and following the prompts (answer Yes to all to extract the exported files directly).

## Vivado Project

1. Generate the Vivado project by running `generate_vivado_prj.sh` in `vivado`.
2. Make changes as necessary, then export your hardware file (`.xsa`).

## PetaLinux

Run the following from the `petalinux` directory.

1. Update the hardware configuration with `petalinux-config --get-hw-description=../vivado/your_new_hw.xsa`.
2. (Optional) Set your board's Ethernet MAC address. The project ships with the placeholder `00:0a:35:00:00:00`.
    1. Run `petalinux-config`.
    2. Go to `Subsystem AUTO Hardware Settings` -> `Ethernet Settings`.
    3. Set `Primary Ethernet` to `axi_ethernet_0`.
    4. Disable `Randomise MAC address`.
    5. Select `Ethernet MAC address` and enter your MAC address.
    6. Save and exit. The address is stored in `project-spec/configs/config`. It is specific to one board, so do not commit that change.
3. Run `petalinux-build`.
4. Run `petalinux-boot --qemu --kernel` to test with QEMU before programming the board.
5. Run `petalinux-boot --jtag --fpga --kernel` to program the board over JTAG.
