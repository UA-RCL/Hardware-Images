# FFT XDMA PL Only Vivado Project

## Overview
This repository contains a portable Vivado project for a Zynq UltraScale+ FPGA design featuring an FFT (Fast Fourier Transform) accelerator with XDMA (PCIe Direct Memory Access) and integrated ILA (Integrated Logic Analyzer) debug capabilities. The project is organized for easy recreation, build, and deployment, and includes all necessary source files, constraints, block design, and pre-built bitstreams.

## Directory Structure
- `vivado_project/` — Main Vivado project directory
  - `load_project.tcl` — TCL script to recreate the Vivado project
  - `sources_1/` — HDL sources, block design, and IP cores
  - `constrs_1/` — XDC constraint files
  - `utils_1/` — Synthesized DCP netlists
- `bitstream/` — Pre-built bitstream and debug files
  - `fft_xdma_block_design_wrapper.bit` — FPGA bitstream
  - `fft_xdma_block_design_wrapper.ltx` — ILA debug probes
  - `debug_nets.ltx` — Additional debug netlist
- `fft_xdma_block_design.pdf` — Block design documentation

## How to Load and Recreate the Project
1. **Requirements:**
   - Vivado 2023.2 (or compatible version)
   - Zynq UltraScale+ target board (xczu19eg-ffvc1760-2-i)

2. **Recreate the Project:**
   - Open a terminal and navigate to the `vivado_project` directory:
     ```bash
     cd vivado_project
     vivado -mode batch -source load_project.tcl
     ```
   - This will automatically create the Vivado project, import all sources, constraints, and block design files.

3. **Open the Project in Vivado (Optional):**
   - After running the TCL script, you can open the generated `.xpr` file in Vivado GUI for further exploration or modification.

4. **Build and Generate Bitstream:**
   - In Vivado, run synthesis, implementation, and bitstream generation as needed.
   - Pre-built bitstreams are available in the `bitstream/` folder for immediate use.

## What the Project Includes
- HDL source files for the FFT accelerator and XDMA interface
- Block design (`fft_xdma_block_design.bd`) and all associated IP cores
- XDC constraint files for pinout and timing
- Pre-synthesized DCP netlist for rapid project recreation
- ILA debug configuration and probe files
- Pre-built bitstream and debug netlists for direct FPGA programming
- Block design PDF documentation

## Bitstream Folder
The `bitstream/` directory contains:
- `fft_xdma_block_design_wrapper.bit`: The main FPGA configuration bitstream
- `fft_xdma_block_design_wrapper.ltx`: ILA debug probe file for use with Vivado Hardware Manager
- `debug_nets.ltx`: Additional debug netlist for advanced debugging

You can use these files to program your FPGA directly without rebuilding the project, or use them with Vivado's Hardware Manager for debugging and validation.

## Hardware Design Configuration

This project implements an FFT streaming accelerator with PCIe XDMA and integrated debug. Below are the main hardware configuration details:

- **Target Device:** Zynq UltraScale+ XCZU19EG-FFVC1760-2-I
- **Top Block Design:** `fft_xdma_block_design`
- **Main IP Blocks:**
  - **XDMA (PCIe DMA, v4.1):**
    - 2-lane PCIe Gen3 (5.0 GT/s)
    - AXI Stream data interface
    - 3 read and 2 write DMA channels
    - Reference clock: 100 MHz
    - Advanced mode, GTY_Quad_128 location
  - **XFFT (v9.1):**
    - 2048-point, single-channel, floating-point FFT
    - Pipelined streaming I/O, block RAM for 4 stages
    - Natural output order, runtime configurable length
    - Target clock: 250 MHz
  - **AXI Stream Vector Multiplier:**
    - Custom HDL module for element-wise vector multiplication
    - 64-bit AXI Stream interface, 250 MHz
  - **System ILA (v1.1):**
    - 6 monitor slots, 7 probes
    - Monitors all major AXI Stream interfaces and debug signals
  - **Other IP:**
    - `util_ds_buf` for differential clock input
    - `c_counter_binary` for test/debug
    - `xlconstant` for static signals
- **Key Connections:**
  - PCIe reference clock and reset via dedicated ports
  - XDMA S_AXIS_C2H_0 connected to FFT output
  - XDMA S_AXIS_C2H_1 connected to vector multiplier output
  - XDMA M_AXIS_H2C_0 to FFT input, M_AXIS_H2C_2 to vector multiplier input
  - All major data and debug nets monitored by ILA
- **Constraints:**
  - Pinout and timing constraints in `constrs_1/new/z19_fft_pcie.xdc` and `implementation_only.xdc`
- **Clocking:**
  - Main user logic at 250 MHz (AXI Stream domain)
  - PCIe reference at 100 MHz

## Support
For questions or issues, please open an issue in your repository or contact the project maintainer.
