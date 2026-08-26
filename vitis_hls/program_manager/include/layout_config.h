#ifndef LAYOUT_CONFIG_H
#define LAYOUT_CONFIG_H

#include <stdint.h>

// =========================================================
// STATIC GRID / CLUSTER / KERNEL LAYOUT
// =========================================================
// This is baked into the bitstream at HLS compile time. Editing the
// layout means re-running synthesis, in exchange for a program manager
// with zero host-side "upload" traffic: the grid initializes itself
// from GRID_LAYOUT on every RESET.
//
// Manager-side only. PEs (pe_unit.hpp) do not include this header -- the
// manager decodes targets and bank validity on their behalf.

// Grid geometry. Overridable from the build so the size can be swept without
// editing sources: `make csim GRID_DIM=16`, `make ip GRID_DIM=16`. After a
// change, run `make layout-header` so pm-app picks up the new dimensions, and
// re-check the register map in the regenerated xprogram_manager_top_hw.h.
#ifndef DIM_X
#define DIM_X 8
#endif
#ifndef DIM_Y
#define DIM_Y 8
#endif
#ifndef MAX_BANKS
#define MAX_BANKS 4
#endif

#define NUM_CLUSTERS 2

struct PEInit {
    uint8_t  cluster_id;
    uint16_t bank_kernel_id[MAX_BANKS]; // kernel ID tag resident in each bank (0 = unused slot)
    uint8_t  num_loaded;                // number of valid entries in bank_kernel_id
};

// Two example clusters:
//  - Cluster 0: left half of the grid (x < DIM_X/2), kernels {101, 102}
//  - Cluster 1: right half of the grid (x >= DIM_X/2), kernels {201, 202, 203}
inline const PEInit &grid_layout_entry(int x, int y) {
    #pragma HLS INLINE
    static const PEInit CLUSTER0 = {0, {101, 102, 0, 0}, 2};
    static const PEInit CLUSTER1 = {1, {201, 202, 203, 0}, 3};
    return (x < DIM_X / 2) ? CLUSTER0 : CLUSTER1;
}

#endif
