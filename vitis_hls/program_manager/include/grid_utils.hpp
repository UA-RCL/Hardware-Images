#ifndef GRID_UTILS_HPP
#define GRID_UTILS_HPP

#include <cstdint>
#include "layout_config.h"

// Status Map Structure
// We use a struct to group the bits, easy to map to AXI registers
struct PE_Map {
    static const int NUM_WORDS = (DIM_X * DIM_Y + 31) / 32;
    uint32_t bits[NUM_WORDS];
};

// Helper: Set a specific bit in the map
inline void set_pe_busy_bit(PE_Map &map, int x, int y) {
    #pragma HLS INLINE
    int flat_index = y * DIM_X + x;
    int word_idx   = flat_index / 32;
    int bit_idx    = flat_index % 32;

    map.bits[word_idx] |= (1 << bit_idx);
}

// Helper: Clear the map for the new cycle
inline void clear_map(PE_Map &map) {
    #pragma HLS INLINE
    for (int k = 0; k < PE_Map::NUM_WORDS; k++) {
        #pragma HLS UNROLL
        map.bits[k] = 0;
    }
}

#endif
