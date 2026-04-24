# FFT XDMA PL Only Vivado Project

## Overview

This directory contains a portable Vivado project setup. The Vivado project is not stored directly. Instead, it can be recreated using Tcl scripts and the source files in this repository.

## Directory Structure

```text
vivado_project/
├── scripts/
│   ├── create_project.tcl    # Recreates the Vivado project
│   └── create_bd.tcl         # Recreates the block design
├── src/
│   ├── axi_stream_v_mult.v   # Custom HDL source files
│   └── ...                   # Additional RTL/Tcl helper files if needed
├── constraints/
│   ├── z19_fft_pcie.xdc      # Main pinout/timing constraints
│   └── implementation_only.xdc
├── bitstream/
│   └── 2fft_block_design.bit # FPGA bitstream
└── block_design.pdf          # Block design documentation
```

## Requirements

- Vivado 2023.2, or a compatible version
- Target FPGA device:
  - `xczu19eg-ffvc1760-2-i`
  
## Recreating the Project

From inside `vivado_project/`, run:

```bash
vivado -mode batch -source scripts/create_project.tcl
```

This creates the Vivado project from the files in `scripts/`, `src/`, and `constraints/`.

After the script finishes, open the generated `.xpr` file in Vivado if needed.

## Updating the Block Design

If you make changes to the block design, update only:

```text
scripts/create_bd.tcl
```

unless you also added or removed RTL files, constraint files, IP repositories, or changed the target device/project structure.

### Exporting the Updated Block Design from Tcl

After editing and validating the block design in Vivado, run:

```tcl
validate_bd_design
save_bd_design
write_bd_tcl -force ./scripts/create_bd.tcl
```

You can also use:

```tcl
write_bd_tcl -force ./scripts/create_bd.tcl
```

### Exporting the Updated Block Design from the GUI

In Vivado, use:

```text
File → Export → Export Block Design
```

Save the exported Tcl file as:

```text
vivado_project/scripts/create_bd.tcl
```

## Version Control Notes

Commit these files/directories:

```text
vivado_project/scripts/create_project.tcl
vivado_project/scripts/create_bd.tcl
vivado_project/src/
vivado_project/constraints/
vivado_project/block_design.pdf
```

Do not commit generated Vivado output directories such as:

```text
vivado_project/build/
*.xpr
*.runs/
*.gen/
*.cache/
*.hw/
*.ip_user_files/
*.sim/
```

After changing the block design, regenerate `scripts/create_bd.tcl` and commit that updated Tcl file. This allows the project to be recreated without committing the generated Vivado project files.
