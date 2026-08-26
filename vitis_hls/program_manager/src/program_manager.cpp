#include "pe_unit.hpp"
#include "dispatch.hpp"
#include "grid_utils.hpp"

// We use pointers for inputs/outputs so they map to AXI Lite registers
// 'volatile' is critical: tells HLS "this value can change externally, read it every loop"
//
// Two layers, deliberately: this file owns the AXI-Lite registers, the command
// bus and all targeting decisions; pe_unit.hpp owns a single PE's timing and
// nothing else. A PE never sees the grid it belongs to.

void program_manager_top(
    volatile uint8_t  *mode,           // 0 = IDLE, 1 = RUN, 2 = RESET
    volatile uint8_t  *target_mode,    // 0 = SINGLE_PE (use pe_x/pe_y), 1 = CLUSTER (use cluster_id)
    volatile uint8_t  *pe_x,
    volatile uint8_t  *pe_y,
    volatile uint8_t  *cluster_id,
    volatile uint8_t  *bank_id,
    volatile uint32_t *cycles,
    volatile bool     *valid_signal,
    volatile uint32_t *busy_mask_out,
    volatile uint32_t busy_map_out_bits[PE_Map::NUM_WORDS]
) {
    // 1. Control Protocol: None (Free Running)
    #pragma HLS INTERFACE ap_ctrl_none port=return

    // 2. Map Ports to AXI Lite (S_AXILITE)
    #pragma HLS INTERFACE s_axilite port=mode
    #pragma HLS INTERFACE s_axilite port=target_mode
    #pragma HLS INTERFACE s_axilite port=pe_x
    #pragma HLS INTERFACE s_axilite port=pe_y
    #pragma HLS INTERFACE s_axilite port=cluster_id
    #pragma HLS INTERFACE s_axilite port=bank_id
    #pragma HLS INTERFACE s_axilite port=cycles
    #pragma HLS INTERFACE s_axilite port=valid_signal
    #pragma HLS INTERFACE s_axilite port=busy_mask_out
    #pragma HLS INTERFACE s_axilite port=busy_map_out_bits

    // Persistent Hardware State. Zero-initialized at configuration, which is now
    // a valid idle PE -- nothing in here has to be seeded by a RESET any more.
    static PE_State pe_states[DIM_X][DIM_Y];
    #pragma HLS ARRAY_PARTITION variable=pe_states complete dim=0

    // Edge Detection State
    static bool last_valid = false;

    // --- INFINITE HARDWARE LOOP ---
    // In hardware this is a truly free-running while(1). In C-sim, the
    // testbench drives one "cycle" per call to this function, so the body
    // must run exactly once per call rather than looping forever or never
    // (a plain while(0) here would skip the body entirely and make csim vacuous).
#ifndef __SYNTHESIS__
        do
#else
        while (1)
#endif
        {
        #pragma HLS PIPELINE II=1

        // --- STEP 1: READ INPUTS (POLLING) ---
        bool current_valid = *valid_signal;

        // Edge Detection: Only trigger if signal went from 0 to 1
        bool trigger_pulse = (current_valid && !last_valid);
        last_valid = current_valid;

        uint8_t  c_mode   = trigger_pulse ? *mode        : (uint8_t)0;
        uint8_t  c_tmode  = trigger_pulse ? *target_mode : (uint8_t)0;
        uint8_t  c_px     = trigger_pulse ? *pe_x        : (uint8_t)0;
        uint8_t  c_py     = trigger_pulse ? *pe_y        : (uint8_t)0;
        uint8_t  c_clus   = trigger_pulse ? *cluster_id  : (uint8_t)0;
        uint8_t  c_bank   = trigger_pulse ? *bank_id     : (uint8_t)0;
        uint32_t c_cycles = trigger_pulse ? *cycles      : (uint32_t)0;

        // --- STEP 2: PREPARE ACCUMULATORS ---
        PE_Map local_map;
        clear_map(local_map);

        // --- STEP 3: DECODE, THEN TICK EVERY PE ---
        // Every PE ticks on every cycle. The old code returned early on a RESET
        // command, which stalled the countdown of PEs the reset was not even
        // addressed to; separating decode from timing removes that.
        MANAGER_LOOP_X: for (int i = 0; i < DIM_X; i++) {
            #pragma HLS UNROLL
            MANAGER_LOOP_Y: for (int j = 0; j < DIM_Y; j++) {
                #pragma HLS UNROLL

                bool selected = trigger_pulse &&
                                pe_selected(c_tmode, c_px, c_py, c_clus, i, j);
                bool start = selected && (c_mode == MODE_RUN) &&
                             bank_legal(c_bank, i, j);
                bool clear = selected && (c_mode == MODE_RESET);

                pe_states[i][j] = pe_tick(pe_states[i][j], start, clear, c_cycles);

                if (pe_states[i][j].busy) {
                    set_pe_busy_bit(local_map, i, j);
                }
            }
        }

        // --- STEP 4: WRITE OUTPUTS ---
        for (int k = 0; k < PE_Map::NUM_WORDS; k++) {
            #pragma HLS UNROLL
            busy_map_out_bits[k] = local_map.bits[k];
        }
        // Legacy single-word view: only covers the first 32 PEs. Kept so the
        // register map does not move; busy_map_out_bits is the real output.
        *busy_mask_out = local_map.bits[0];
    }
#ifndef __SYNTHESIS__
    while (0);
#endif
}
