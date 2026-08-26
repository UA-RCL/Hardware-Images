#ifndef PE_UNIT_HPP
#define PE_UNIT_HPP

#include <cstdint>

// A PE is a countdown timer and nothing else. It knows nothing about the grid it
// sits in: which command is addressed to it, and whether the requested bank is
// resident, are decided by the manager (dispatch.hpp) before `start` is asserted.
// Deliberately no include of layout_config.h -- that dependency is what made a PE
// impossible to synthesize or test on its own.
//
// State goes in and out by value so that with INLINE off, HLS emits a replicated
// combinational module and leaves the registers in the parent. Passing the struct
// by reference across a non-inlined boundary would synthesize memory ports.

struct PE_State {
    uint32_t count;
    uint32_t target_cycles;
    bool     busy;
};

inline PE_State pe_initial_state() {
    #pragma HLS INLINE
    PE_State s;
    s.count = 0;
    s.target_cycles = 0;
    s.busy = false;
    return s;
}

// `start`  the manager has decided this command is for this PE and is legal
// `clear`  RESET: abandon whatever is in flight
// `cycles` occupancy length, only sampled on the cycle `start` is high
//
// Returns the next state. The occupancy bit for this cycle is the returned
// `busy` -- it is high on the cycle work starts and low on the cycle it ends.
//
// Build with -DPE_FLAT to inline this back into the manager (the pre-split
// behaviour), which is the fallback if the module boundary costs too much.
inline PE_State pe_tick(PE_State s, bool start, bool clear, uint32_t cycles) {
#ifdef PE_FLAT
    #pragma HLS INLINE
#else
    #pragma HLS INLINE off
#endif

    if (clear) {
        return pe_initial_state();
    }

    if (!s.busy) {
        if (start) {
            s.busy = true;
            s.count = 0;
            // Latch the length now: `cycles` is only valid on this cycle. Even a
            // zero-cycle request must occupy the PE for one cycle.
            s.target_cycles = (cycles == 0) ? 1 : cycles;
        }
    } else {
        // A busy PE ignores `start`. This is the one decision that is genuinely
        // PE-local, so it stays here rather than moving to the manager.
        if (s.count < s.target_cycles - 1) {
            s.count++;
        } else {
            s.busy = false;
        }
    }

    return s;
}

#endif
