#ifndef DISPATCH_HPP
#define DISPATCH_HPP

#include <cstdint>
#include "layout_config.h"

// Everything the manager needs to decide, for one PE, whether a command applies
// to it. This used to live inside the PE, which meant every PE carried a copy of
// the whole grid's layout.
//
// Cluster membership is no longer a compiled-in function of position: the manager
// holds a runtime table (see program_manager.cpp) that the host uploads with
// MODE_CONFIG commands at startup, so re-clustering the grid needs no rebuild.
// layout_config.h survives as the power-up default, used until the host commits a
// table of its own.

enum Mode : uint8_t {
    MODE_IDLE   = 0,
    MODE_RUN    = 1,
    MODE_RESET  = 2,
    MODE_CONFIG = 3
};

enum TargetMode : uint8_t {
    TARGET_SINGLE_PE = 0,
    TARGET_CLUSTER   = 1
};

// Reserved cluster ids. Both sit above any id a layout file may use.
static const uint8_t CLUSTER_SPARE = 255;  // in no cluster: matches nothing, runs nothing
static const uint8_t CLUSTER_ALL   = 254;  // targets every PE, spares included

// Under MODE_CONFIG, pe_x == CFG_COMMIT means "no PE: publish the table that has
// been written so far" rather than "configure PE 255".
static const uint8_t CFG_COMMIT = 255;

// The layout in force for one PE. Until the host commits a table, this is the
// compiled-in default, so a freshly configured bitstream behaves exactly as it did
// before this mechanism existed. Note the fallback is gated on a flag we set
// ourselves rather than on static array initializers surviving into RTL, which
// depends on the config_rtl reset mode.
inline uint8_t eff_cluster(bool loaded, uint8_t cfg, int x, int y) {
    #pragma HLS INLINE
    return loaded ? cfg : grid_layout_entry(x, y).cluster_id;
}

inline uint8_t eff_banks(bool loaded, uint8_t cfg, int x, int y) {
    #pragma HLS INLINE
    return loaded ? cfg : grid_layout_entry(x, y).num_loaded;
}

// Address decode, against the effective cluster id of this PE.
inline bool pe_selected(uint8_t tmode, uint8_t tx, uint8_t ty,
                        uint8_t cluster, int x, int y, uint8_t ecl) {
    #pragma HLS INLINE
    if (tmode == TARGET_SINGLE_PE) {
        return (x == (int)tx && y == (int)ty);
    }
    if (cluster == CLUSTER_ALL) {
        return true;
    }
    // A spare belongs to no cluster, so it can only ever be reached by CLUSTER_ALL
    // or by being named directly.
    return (ecl != CLUSTER_SPARE) && (ecl == cluster);
}

// A RUN is legal only if the requested bank is resident on this PE. Spares are
// configured with zero banks, so this rejects them without a special case.
inline bool bank_legal(uint8_t bank, uint8_t ebk) {
    #pragma HLS INLINE
    return bank < ebk;
}

#endif
