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
    volatile uint32_t busy_map_out_bits[PE_Map::NUM_WORDS],
    volatile uint32_t *cfg_status       // readback of the cluster table, see below
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
    // Declared last so it lands after busy_map_out_bits in the register map and
    // does not push the map off 0x60. Re-check xprogram_manager_top_hw.h anyway.
    #pragma HLS INTERFACE s_axilite port=cfg_status

    // Persistent Hardware State. Zero-initialized at configuration, which is now
    // a valid idle PE -- nothing in here has to be seeded by a RESET any more.
    static PE_State pe_states[DIM_X][DIM_Y];
    #pragma HLS ARRAY_PARTITION variable=pe_states complete dim=0

    // Runtime cluster table, uploaded by the host one PE at a time with
    // MODE_CONFIG and then published with a single commit command. Until the
    // commit lands, eff_cluster()/eff_banks() fall back to the compiled-in layout,
    // so the grid is usable straight out of configuration.
    static uint8_t cfg_cluster[DIM_X][DIM_Y];
    static uint8_t cfg_banks  [DIM_X][DIM_Y];
    static bool    cfg_loaded = false;
    #pragma HLS ARRAY_PARTITION variable=cfg_cluster complete dim=0
    #pragma HLS ARRAY_PARTITION variable=cfg_banks   complete dim=0

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

        // A config command addressed to a real PE writes the table; the one
        // addressed to CFG_COMMIT publishes it. Splitting the two means the grid
        // never dispatches against a half-uploaded table.
        bool cfg_cmd    = trigger_pulse && (c_mode == MODE_CONFIG);
        bool cfg_commit = cfg_cmd && (c_px == CFG_COMMIT);
        if (cfg_commit) {
            cfg_loaded = true;
        }

        // --- STEP 3: DECODE, THEN TICK EVERY PE ---
        // Every PE ticks on every cycle. The old code returned early on a RESET
        // command, which stalled the countdown of PEs the reset was not even
        // addressed to; separating decode from timing removes that.
        MANAGER_LOOP_X: for (int i = 0; i < DIM_X; i++) {
            #pragma HLS UNROLL
            MANAGER_LOOP_Y: for (int j = 0; j < DIM_Y; j++) {
                #pragma HLS UNROLL

                // Config write. MODE_CONFIG never reaches start/clear below, so
                // uploading a table cannot disturb a countdown already in flight.
                if (cfg_cmd && !cfg_commit &&
                    i == (int)c_px && j == (int)c_py) {
                    cfg_cluster[i][j] = c_clus;
                    cfg_banks[i][j]   = c_bank;
                }

                uint8_t ecl = eff_cluster(cfg_loaded, cfg_cluster[i][j], i, j);
                uint8_t ebk = eff_banks  (cfg_loaded, cfg_banks[i][j],   i, j);

                bool selected = trigger_pulse &&
                                pe_selected(c_tmode, c_px, c_py, c_clus, i, j, ecl);
                bool start = selected && (c_mode == MODE_RUN) &&
                             bank_legal(c_bank, ebk);
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

        // Cluster-table readback for whichever PE pe_x/pe_y currently address, so
        // the host can prove an upload landed instead of inferring it from
        // dispatch behaviour. Reports the raw table plus the commit bit, so the
        // host can tell "written but not published" from "in force".
        //   bit 31    cfg_loaded
        //   bits 15:8 cluster id      bits 7:0 bank count
        uint8_t rb_x = (*pe_x < DIM_X) ? (uint8_t)*pe_x : (uint8_t)0;
        uint8_t rb_y = (*pe_y < DIM_Y) ? (uint8_t)*pe_y : (uint8_t)0;
        *cfg_status = ((uint32_t)(cfg_loaded ? 1u : 0u) << 31) |
                      ((uint32_t)cfg_cluster[rb_x][rb_y] << 8) |
                      ((uint32_t)cfg_banks[rb_x][rb_y]);
    }
#ifndef __SYNTHESIS__
    while (0);
#endif
}
