#ifndef DISPATCH_HPP
#define DISPATCH_HPP

#include <cstdint>
#include "layout_config.h"

// Everything the manager needs to decide, for one PE, whether a command applies
// to it. This used to live inside the PE, which meant every PE carried a copy of
// the whole grid's layout.

enum Mode : uint8_t {
    MODE_IDLE  = 0,
    MODE_RUN   = 1,
    MODE_RESET = 2
};

enum TargetMode : uint8_t {
    TARGET_SINGLE_PE = 0,
    TARGET_CLUSTER   = 1
};

// Address decode. Cluster membership is a pure function of position, so this is
// combinational and no per-PE copy of the layout is stored anywhere.
inline bool pe_selected(uint8_t tmode, uint8_t tx, uint8_t ty,
                        uint8_t cluster, int x, int y) {
    #pragma HLS INLINE
    return (tmode == TARGET_SINGLE_PE)
             ? (x == (int)tx && y == (int)ty)
             : (grid_layout_entry(x, y).cluster_id == cluster);
}

// A RUN is legal only if the requested bank is resident in this PE's cluster.
// Previously this read num_loaded out of PE state, which powers up as zero, so
// every RUN was silently dropped until a RESET had copied the layout in. Deriving
// it from the compiled-in layout removes that dependency.
inline bool bank_legal(uint8_t bank, int x, int y) {
    #pragma HLS INLINE
    return bank < grid_layout_entry(x, y).num_loaded;
}

#endif
