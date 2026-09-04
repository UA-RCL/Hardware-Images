#ifndef DISPATCH_HPP
#define DISPATCH_HPP

#include <cstdint>
#include "layout_config.h"

// Everything the manager needs to decide, for one PE, whether a command applies
// to it. This used to live inside the PE, which meant every PE carried a copy of
// the whole grid's layout.
//
// Two runtime tables live in the manager (see program_manager.cpp) and the host
// uploads both with MODE_CONFIG at startup:
//   cfg_cluster[x][y]          which cluster a PE belongs to
//   cfg_kernel [x][y][bank]    which kernel is resident in each of its banks
// so re-clustering the grid, and moving kernels around it, need no rebuild.
// layout_config.h survives as the power-up default for both, used until the host
// commits a table of its own.
//
// Residency is a fact about a slot, not a count: a bank holds a kernel id or 0.
// That is what lets a RUN name a kernel rather than a bank -- each PE finds the
// slot holding it, and PEs in one cluster need not agree on which slot that is.

enum Mode : uint8_t {
    MODE_IDLE       = 0,
    MODE_RUN        = 1,  // bank_id names a slot
    MODE_RESET      = 2,
    MODE_CONFIG     = 3,  // target_mode selects the sub-op below
    MODE_LOAD       = 4,  // open an AXI-Stream instruction transfer
    MODE_RUN_KERNEL = 5,  // kernel_id names a kernel, each PE finds its own slot
    MODE_LOAD_ABORT = 6   // tear down a transfer whose stream never ended
};

enum TargetMode : uint8_t {
    TARGET_SINGLE_PE = 0,
    TARGET_CLUSTER   = 1
};

// Under MODE_CONFIG, target_mode has no targeting meaning (a config write always
// names one PE) and carries the sub-op instead.
enum ConfigOp : uint8_t {
    CFG_CLUSTER = 0,  // cluster_id -> cfg_cluster[pe_x][pe_y]
    CFG_KERNEL  = 1   // kernel_id  -> cfg_kernel [pe_x][pe_y][bank_id]
};

// Reserved cluster ids. Both sit above any id a layout file may use.
static const uint8_t CLUSTER_SPARE = 255;  // in no cluster: matches nothing, runs nothing
static const uint8_t CLUSTER_ALL   = 254;  // targets every PE, spares included

// Under MODE_CONFIG, pe_x == CFG_COMMIT means "no PE: publish the table that has
// been written so far" rather than "configure PE 255".
static const uint8_t CFG_COMMIT = 255;

// An empty bank. Kernel ids are 1..255, which is why the table is a byte wide.
static const uint8_t KERNEL_NONE = 0;

// The layout in force for one PE. Until the host commits a table, this is the
// compiled-in default, so a freshly configured bitstream behaves exactly as it did
// before this mechanism existed. Note the fallback is gated on a flag we set
// ourselves rather than on static array initializers surviving into RTL, which
// depends on the config_rtl reset mode.
inline uint8_t eff_cluster(bool loaded, uint8_t cfg, int x, int y) {
    #pragma HLS INLINE
    return loaded ? cfg : grid_layout_entry(x, y).cluster_id;
}

// Same for one bank's tag. The compiled-in bank_kernel_id has always held kernel
// ids -- it simply had no reader until now -- so the fallback reproduces the old
// `bank < num_loaded` rule exactly: banks 0..1 resident on the left half, 0..2 on
// the right, nothing beyond.
inline uint8_t eff_kernel(bool loaded, uint8_t cfg, int x, int y, int b) {
    #pragma HLS INLINE
    return loaded ? cfg : (uint8_t)grid_layout_entry(x, y).bank_kernel_id[b];
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

// A RUN is legal only if the named bank actually holds something. A spare's banks
// are all empty, so this rejects it without a special case.
inline bool bank_resident(uint8_t tag) {
    #pragma HLS INLINE
    return tag != KERNEL_NONE;
}

// The lowest-numbered slot holding `kernel`, or -1 if this PE does not have it.
// Fully unrolled: MAX_BANKS comparators and a priority encode, per PE.
inline int kernel_slot(const uint8_t tags[MAX_BANKS], uint8_t kernel) {
    #pragma HLS INLINE
    int slot = -1;
    KERNEL_SLOT: for (int b = MAX_BANKS - 1; b >= 0; b--) {
        #pragma HLS UNROLL
        if (kernel != KERNEL_NONE && tags[b] == kernel) {
            slot = b;
        }
    }
    return slot;
}

// How many of a PE's banks hold something. Reported for the host's benefit only --
// nothing in dispatch uses it any more.
inline uint8_t resident_count(const uint8_t tags[MAX_BANKS]) {
    #pragma HLS INLINE
    uint8_t n = 0;
    RESIDENT_COUNT: for (int b = 0; b < MAX_BANKS; b++) {
        #pragma HLS UNROLL
        if (tags[b] != KERNEL_NONE) n++;
    }
    return n;
}

#endif
