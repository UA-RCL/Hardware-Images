#ifndef LAYOUT_CONFIG_H
#define LAYOUT_CONFIG_H

#include <stdint.h>

// =========================================================
// POWER-UP DEFAULT GRID / CLUSTER / KERNEL LAYOUT
// =========================================================
// This is no longer "the" layout: the manager holds a runtime cluster table that
// the host uploads with MODE_CONFIG commands from /etc/pm-layout.conf, so
// re-clustering the grid does not need HLS. What is baked in here is only the
// default in force from configuration until the host commits a table of its own,
// which keeps a freshly programmed bitstream usable with no host at all.
//
// Grid *dimensions* remain compile-time: DIM_X/DIM_Y size the register arrays and
// the unrolled dispatch loop. A layout file partitions the compiled grid; it
// cannot grow it.
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

// Two default clusters, mirrored by the shipped pm-layout.conf so that the first
// boot with a layout file behaves identically to one without:
//  - Cluster 0: left half of the grid (x < DIM_X/2), kernels {101, 102}
//  - Cluster 1: right half of the grid (x >= DIM_X/2), kernels {201, 202, 203}
inline const PEInit &grid_layout_entry(int x, int y) {
    #pragma HLS INLINE
    static const PEInit CLUSTER0 = {0, {101, 102, 0, 0}, 2};
    static const PEInit CLUSTER1 = {1, {201, 202, 203, 0}, 3};
    return (x < DIM_X / 2) ? CLUSTER0 : CLUSTER1;
}

#endif
