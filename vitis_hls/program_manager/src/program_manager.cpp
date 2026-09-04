#include <hls_stream.h>
#include <ap_axi_sdata.h>

#include "pe_unit.hpp"
#include "dispatch.hpp"
#include "grid_utils.hpp"

// We use pointers for inputs/outputs so they map to AXI Lite registers
// 'volatile' is critical: tells HLS "this value can change externally, read it every loop"
//
// Two layers, deliberately: this file owns the AXI-Lite registers, the command
// bus and all targeting decisions; pe_unit.hpp owns a single PE's timing and
// nothing else. A PE never sees the grid it belongs to.
//
// Instructions do NOT come in over AXI-Lite. They arrive as 64-bit VLIW words on
// instr_in, an AXI4-Stream slave fed by an AXI DMA, and are consumed with a
// non-blocking read of one beat per loop iteration. That is the whole reason for
// the stream: this is a free-running ap_ctrl_none kernel whose loop ticks all 64
// PE countdowns every iteration, so it must never block on a transfer. A burst
// read from an AXI master would stall every PE for the length of the load.
//
// The PEs have no instruction memory yet, so beats are counted and discarded.
// When banks become real memories the change is to route beat.data into the
// addressed bank; the framing, the residency table and the DMA path stay as-is.

typedef ap_axiu<64, 0, 0, 0> instr_beat_t;

void program_manager_top(
    volatile uint8_t  *mode,           // see enum Mode in dispatch.hpp
    volatile uint8_t  *target_mode,    // 0 = SINGLE_PE (use pe_x/pe_y), 1 = CLUSTER (use cluster_id)
    volatile uint8_t  *pe_x,
    volatile uint8_t  *pe_y,
    volatile uint8_t  *cluster_id,
    volatile uint8_t  *bank_id,        // a slot index, 0..MAX_BANKS-1
    volatile uint8_t  *kernel_id,      // LOAD payload id, RUN_KERNEL target, and residency query
    volatile uint32_t *instr_count,    // instructions the next LOAD will deliver
    volatile uint32_t *cycles,
    volatile bool     *valid_signal,
    hls::stream<instr_beat_t> &instr_in,
    volatile uint32_t *busy_mask_out,
    volatile uint32_t busy_map_out_bits[PE_Map::NUM_WORDS],
    volatile uint32_t kernel_map_out_bits[PE_Map::NUM_WORDS],
    volatile uint32_t *cfg_status,
    volatile uint32_t *load_status,
    volatile uint32_t *load_last
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
    #pragma HLS INTERFACE s_axilite port=kernel_id
    #pragma HLS INTERFACE s_axilite port=instr_count
    #pragma HLS INTERFACE s_axilite port=cycles
    #pragma HLS INTERFACE s_axilite port=valid_signal
    #pragma HLS INTERFACE s_axilite port=busy_mask_out
    #pragma HLS INTERFACE s_axilite port=busy_map_out_bits
    #pragma HLS INTERFACE s_axilite port=kernel_map_out_bits
    #pragma HLS INTERFACE s_axilite port=cfg_status
    #pragma HLS INTERFACE s_axilite port=load_status
    #pragma HLS INTERFACE s_axilite port=load_last

    // 3. The instruction path: a plain AXI4-Stream slave (TDATA/TVALID/TREADY/TLAST).
    #pragma HLS INTERFACE mode=axis port=instr_in

    // Persistent Hardware State. Zero-initialized at configuration, which is now
    // a valid idle PE -- nothing in here has to be seeded by a RESET any more.
    static PE_State pe_states[DIM_X][DIM_Y];
    #pragma HLS ARRAY_PARTITION variable=pe_states complete dim=0

    // Runtime tables, uploaded by the host one entry at a time with MODE_CONFIG
    // and then published with a single commit command. Until the commit lands,
    // eff_cluster()/eff_kernel() fall back to the compiled-in layout, so the grid
    // is usable straight out of configuration.
    static uint8_t cfg_cluster[DIM_X][DIM_Y];
    static uint8_t cfg_kernel [DIM_X][DIM_Y][MAX_BANKS];
    static bool    cfg_loaded = false;
    #pragma HLS ARRAY_PARTITION variable=cfg_cluster complete dim=0
    #pragma HLS ARRAY_PARTITION variable=cfg_kernel  complete dim=0

    // What each PE is currently executing. The countdown in PE_State says a PE is
    // busy but not with what, and after dispatch the identity is otherwise gone:
    // MODE_RUN names a bank, MODE_RUN_KERNEL names a kernel, and an auto-start
    // names neither after the fact. Latching it here is what makes "what is
    // running right now" answerable from hardware rather than from host
    // bookkeeping, which does not survive a restart.
    static uint8_t pe_kernel[DIM_X][DIM_Y];
    #pragma HLS ARRAY_PARTITION variable=pe_kernel complete dim=0

    // In-flight instruction transfer.
    static bool     ld_active  = false;
    static bool     ld_refused = false;   // a target was busy: nothing was opened
    static bool     ld_short   = false;   // stream ended before instr_count beats
    static bool     ld_over    = false;   // more beats arrived than declared
    static uint8_t  ld_kernel  = 0;
    static uint8_t  ld_bank    = 0;
    static uint32_t ld_expect  = 0;
    static uint32_t ld_beats   = 0;
    static uint32_t ld_run     = 0;       // auto-start length, 0 = commit only
    static uint32_t ld_last    = 0;
    static bool     ld_sel[DIM_X][DIM_Y];
    #pragma HLS ARRAY_PARTITION variable=ld_sel complete dim=0

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

        uint8_t  c_mode   = trigger_pulse ? *mode        : (uint8_t)MODE_IDLE;
        uint8_t  c_tmode  = trigger_pulse ? *target_mode : (uint8_t)0;
        uint8_t  c_px     = trigger_pulse ? *pe_x        : (uint8_t)0;
        uint8_t  c_py     = trigger_pulse ? *pe_y        : (uint8_t)0;
        uint8_t  c_clus   = trigger_pulse ? *cluster_id  : (uint8_t)0;
        uint8_t  c_bank   = trigger_pulse ? *bank_id     : (uint8_t)0;
        uint8_t  c_kern   = trigger_pulse ? *kernel_id   : (uint8_t)0;
        uint32_t c_instr  = trigger_pulse ? *instr_count : (uint32_t)0;
        uint32_t c_cycles = trigger_pulse ? *cycles      : (uint32_t)0;

        // bank_id is a whole byte off the bus, so it can name a slot that does not
        // exist. Clamp before it is ever used as an index -- unclamped it is an
        // out-of-bounds read in C-sim and a wider mux than intended in RTL.
        bool    bank_ok = (c_bank < MAX_BANKS);
        uint8_t b_idx   = bank_ok ? c_bank : (uint8_t)0;

        // The residency query is a live readback, not a command: whatever id sits
        // in kernel_id right now is the one kernel_map_out_bits answers for.
        uint8_t q_kern = *kernel_id;

        // --- STEP 2: PREPARE ACCUMULATORS ---
        PE_Map local_map, kernel_map;
        clear_map(local_map);
        clear_map(kernel_map);

        // A config command addressed to a real PE writes one table entry; the one
        // addressed to CFG_COMMIT publishes both tables. Splitting the two means
        // the grid never dispatches against a half-uploaded table.
        bool cfg_cmd    = trigger_pulse && (c_mode == MODE_CONFIG);
        bool cfg_commit = cfg_cmd && (c_px == CFG_COMMIT);
        bool cfg_write  = cfg_cmd && !cfg_commit;
        if (cfg_commit) {
            cfg_loaded = true;
        }

        // --- STEP 3: DECIDE WHETHER A LOAD MAY OPEN ---
        // A load is refused outright if any PE it targets is busy, or if another
        // transfer is still in flight. All-or-nothing: a broadcast that took on
        // some PEs and not others would leave the table describing a delivery that
        // never happened. The host picks its target by reading the array status
        // first, so this is the backstop for that race, not the mechanism.
        bool load_cmd  = trigger_pulse && (c_mode == MODE_LOAD);
        bool abort_cmd = trigger_pulse && (c_mode == MODE_LOAD_ABORT);

        // Everything each PE needs, derived from the registered tables exactly
        // once. Sharing matters here: computed twice, the tag comparators alone
        // cost more than the tables they read.
        //
        // In particular there is ONE slot lookup per PE, not two. c_kern is
        // q_kern gated by trigger_pulse -- the same register -- so the result
        // serves both the RUN_KERNEL decode and the residency query bitmap.
        uint8_t ecl_of[DIM_X][DIM_Y];
        uint8_t tags  [DIM_X][DIM_Y][MAX_BANKS];
        bool    sel   [DIM_X][DIM_Y];
        bool    hit   [DIM_X][DIM_Y];   // this PE holds q_kern in some bank
        #pragma HLS ARRAY_PARTITION variable=ecl_of complete dim=0
        #pragma HLS ARRAY_PARTITION variable=tags   complete dim=0
        #pragma HLS ARRAY_PARTITION variable=sel    complete dim=0
        #pragma HLS ARRAY_PARTITION variable=hit    complete dim=0

        bool any_busy_sel = false;

        SCAN_X: for (int i = 0; i < DIM_X; i++) {
            #pragma HLS UNROLL
            SCAN_Y: for (int j = 0; j < DIM_Y; j++) {
                #pragma HLS UNROLL
                ecl_of[i][j] = eff_cluster(cfg_loaded, cfg_cluster[i][j], i, j);
                SCAN_TAGS: for (int b = 0; b < MAX_BANKS; b++) {
                    #pragma HLS UNROLL
                    tags[i][j][b] = eff_kernel(cfg_loaded, cfg_kernel[i][j][b], i, j, b);
                }

                sel[i][j] = pe_selected(c_tmode, c_px, c_py, c_clus, i, j, ecl_of[i][j]);

                // Which slot holds it does not matter yet -- starting a PE does
                // not name a bank. When banks become real memories this is where
                // the index comes from.
                hit[i][j] = (kernel_slot(tags[i][j], q_kern) >= 0);
                if (hit[i][j]) {
                    set_pe_busy_bit(kernel_map, i, j);
                }

                if (load_cmd && sel[i][j] && pe_states[i][j].busy) {
                    any_busy_sel = true;
                }
            }
        }

        bool load_open = load_cmd && !any_busy_sel && !ld_active && bank_ok;

        if (load_cmd) {
            // Status bits describe the most recent load attempt only.
            ld_refused = !load_open;
            ld_short   = false;
            ld_over    = false;
        }
        if (load_open) {
            ld_kernel = c_kern;
            ld_bank   = b_idx;
            ld_expect = c_instr;
            ld_run    = c_cycles;
            ld_beats  = 0;
            ld_active = true;
        }
        if (abort_cmd) {
            ld_active = false;
        }

        // --- STEP 4: CONSUME ONE BEAT ---
        // Non-blocking, and outside the PE loop: on an iteration with no beat
        // waiting the manager does its normal work and every countdown still
        // advances. That is what keeps a multi-thousand-word transfer invisible to
        // the PEs.
        instr_beat_t beat;
        bool beat_ok = false;
        if (ld_active) {
            beat_ok = instr_in.read_nb(beat);
        }

        uint32_t beats_next = ld_beats + (beat_ok ? 1u : 0u);
        bool     beat_last  = beat_ok && (beat.last != 0);
        // The declared count and TLAST must agree. A stream that stops early
        // commits nothing, so the bank stays empty and the failure surfaces as
        // "kernel not resident" rather than as a PE running half a program.
        bool ld_commit = beat_last && (beats_next == ld_expect);

        if (beat_ok) {
            ld_last  = (uint32_t)beat.data;
            ld_beats = beats_next;
            if (!beat_last && beats_next >= ld_expect) {
                ld_over = true;
            }
        }
        if (beat_last) {
            if (!ld_commit) {
                ld_short = true;
            }
            ld_active = false;
        }

        // --- STEP 5: DECODE, THEN TICK EVERY PE ---
        // Every PE ticks on every cycle. The old code returned early on a RESET
        // command, which stalled the countdown of PEs the reset was not even
        // addressed to; separating decode from timing removes that.
        MANAGER_LOOP_X: for (int i = 0; i < DIM_X; i++) {
            #pragma HLS UNROLL
            MANAGER_LOOP_Y: for (int j = 0; j < DIM_Y; j++) {
                #pragma HLS UNROLL

                // Config write. MODE_CONFIG never reaches start/clear below, so
                // uploading a table cannot disturb a countdown already in flight.
                if (cfg_write && i == (int)c_px && j == (int)c_py) {
                    if (c_tmode == CFG_KERNEL) {
                        if (bank_ok) cfg_kernel[i][j][b_idx] = c_kern;
                    } else {
                        cfg_cluster[i][j] = c_clus;
                    }
                }

                // Opening a transfer empties the destination bank on every PE it
                // targets, so a bank being written reads as empty for the whole
                // transfer and cannot be dispatched to. Note tags[] above was read
                // from the registers before any of these writes, deliberately:
                // feeding a same-cycle write back into the dispatch decode would
                // put a table write and a full slot lookup on one combinational
                // path, and a config write and a RUN are never the same command
                // anyway.
                if (load_open) {
                    ld_sel[i][j] = sel[i][j];
                    if (sel[i][j]) {
                        cfg_kernel[i][j][b_idx] = KERNEL_NONE;
                    }
                }

                bool in_load = ld_active && ld_sel[i][j];
                bool mine    = ld_commit && ld_sel[i][j];

                // Completing it publishes the tag.
                if (mine) {
                    cfg_kernel[i][j][ld_bank] = ld_kernel;
                }

                bool selected = trigger_pulse && sel[i][j];

                // A PE in the middle of a transfer takes no dispatch at all, so it
                // cannot be started through one of its other banks either.
                bool run_bank = selected && (c_mode == MODE_RUN) &&
                                bank_ok && bank_resident(tags[i][j][b_idx]);
                bool run_kern = selected && (c_mode == MODE_RUN_KERNEL) && hit[i][j];

                // Auto-start: committing the tag and starting the PE happen in the
                // same cycle, so "not resident -> stream -> start" leaves no window
                // for another command to claim the PE.
                bool auto_run = mine && (ld_run != 0);

                bool start = (!in_load && (run_bank || run_kern)) || auto_run;
                bool clear = selected && (c_mode == MODE_RESET);
                uint32_t len = auto_run ? ld_run : c_cycles;

                // Record what this PE is about to execute. Sampled BEFORE the
                // tick because pe_tick ignores start on a busy PE -- latching
                // unconditionally would record a dispatch the PE actually
                // refused. The id is already in hand at all three start paths:
                // RUN carries it in the addressed bank's tag, RUN_KERNEL names it
                // outright, and an auto-start uses the kernel just delivered.
                bool busy_before = pe_states[i][j].busy;
                if (clear) {
                    pe_kernel[i][j] = KERNEL_NONE;
                } else if (start && !busy_before) {
                    pe_kernel[i][j] = auto_run ? ld_kernel
                                    : (run_kern ? c_kern : tags[i][j][b_idx]);
                }

                pe_states[i][j] = pe_tick(pe_states[i][j], start, clear, len);

                if (pe_states[i][j].busy) {
                    set_pe_busy_bit(local_map, i, j);
                }
            }
        }

        // --- STEP 6: WRITE OUTPUTS ---
        for (int k = 0; k < PE_Map::NUM_WORDS; k++) {
            #pragma HLS UNROLL
            busy_map_out_bits[k]   = local_map.bits[k];
            // "Which PEs hold kernel_id, in any bank" -- one read instead of
            // walking all 256 tags through cfg_status.
            kernel_map_out_bits[k] = kernel_map.bits[k];
        }
        // Legacy single-word view: only covers the first 32 PEs. Kept so the
        // register map does not move; busy_map_out_bits is the real output.
        *busy_mask_out = local_map.bits[0];

        // Table readback for whichever PE pe_x/pe_y address and whichever slot
        // bank_id names, so the host can prove an upload landed instead of
        // inferring it from dispatch behaviour. Reports the raw tables plus the
        // commit bit, so "written but not published" is distinguishable from
        // "in force".
        // resident_count only ever reaches MAX_BANKS, so it does not need a whole
        // byte; narrowing it to 3 bits freed 23:16 for the running kernel without
        // adding a port or moving anything in the register map.
        //   bit 31     cfg_loaded            bit 30     PE is busy
        //   bit 29     PE is in the load set bits 26:24 resident bank count
        //   bits 23:16 running kernel, 0 = idle
        //   bits 15:8  cluster id            bits 7:0   kernel id in bank_id
        uint8_t rb_x = (*pe_x    < DIM_X)     ? (uint8_t)*pe_x    : (uint8_t)0;
        uint8_t rb_y = (*pe_y    < DIM_Y)     ? (uint8_t)*pe_y    : (uint8_t)0;
        uint8_t rb_b = (*bank_id < MAX_BANKS) ? (uint8_t)*bank_id : (uint8_t)0;

        // Gated on busy: the latch keeps its last value after a run ends, and a
        // stale id must never read as live work.
        bool     rb_busy = pe_states[rb_x][rb_y].busy;
        uint32_t rb_run  = rb_busy ? (uint32_t)pe_kernel[rb_x][rb_y]
                                   : (uint32_t)KERNEL_NONE;

        *cfg_status = ((uint32_t)(cfg_loaded ? 1u : 0u) << 31) |
                      ((uint32_t)(rb_busy ? 1u : 0u) << 30) |
                      ((uint32_t)((ld_active && ld_sel[rb_x][rb_y]) ? 1u : 0u) << 29) |
                      (((uint32_t)resident_count(tags[rb_x][rb_y]) & 0x7u) << 24) |
                      (rb_run << 16) |
                      ((uint32_t)ecl_of[rb_x][rb_y] << 8) |
                      ((uint32_t)tags[rb_x][rb_y][rb_b]);

        //   bit 31 active   bit 30 refused   bit 29 short   bit 28 overrun
        //   bits 23:0 beats received by the transfer being or last reported
        *load_status = ((uint32_t)(ld_active  ? 1u : 0u) << 31) |
                       ((uint32_t)(ld_refused ? 1u : 0u) << 30) |
                       ((uint32_t)(ld_short   ? 1u : 0u) << 29) |
                       ((uint32_t)(ld_over    ? 1u : 0u) << 28) |
                       (ld_beats & 0x00ffffffu);
        // Low half of the most recent beat: with a known pattern this catches a
        // dropped or duplicated word, which the beat count alone would not.
        *load_last = ld_last;
    }
#ifndef __SYNTHESIS__
    while (0);
#endif
}
