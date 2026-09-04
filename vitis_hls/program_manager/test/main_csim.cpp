#include <iostream>
#include <bitset>
#include <hls_stream.h>
#include <ap_axi_sdata.h>
#include "dispatch.hpp"
#include "grid_utils.hpp"

typedef ap_axiu<64, 0, 0, 0> instr_beat_t;

// Prototype matching the AXI-Lite pointer interface plus the AXI-Stream
// instruction port.
void program_manager_top(
    volatile uint8_t  *mode,
    volatile uint8_t  *target_mode,
    volatile uint8_t  *pe_x,
    volatile uint8_t  *pe_y,
    volatile uint8_t  *cluster_id,
    volatile uint8_t  *bank_id,
    volatile uint8_t  *kernel_id,
    volatile uint32_t *instr_count,
    volatile uint32_t *cycles,
    volatile bool     *valid_signal,
    hls::stream<instr_beat_t> &instr_in,
    volatile uint32_t *busy_mask_out,
    volatile uint32_t *busy_map_out_bits,
    volatile uint32_t *kernel_map_out_bits,
    volatile uint32_t *cfg_status,
    volatile uint32_t *load_status,
    volatile uint32_t *load_last
);

void print_pe_grid(uint32_t *map_bits) {
    PE_Map map;
    for (int i = 0; i < PE_Map::NUM_WORDS; i++) map.bits[i] = map_bits[i];

    std::cout << "   PE Occupancy Map:" << std::endl;
    for (int y = 0; y < DIM_Y; y++) {
        std::cout << "   |";
        for (int x = 0; x < DIM_X; x++) {
            int flat = y * DIM_X + x;
            bool is_busy = (map.bits[flat / 32] >> (flat % 32)) & 1;
            std::cout << (is_busy ? "[]" : " .");
        }
        std::cout << "|" << std::endl;
    }
}

int main() {
    // --- Virtual Hardware Registers ---
    volatile uint8_t  reg_mode = 0;
    volatile uint8_t  reg_tmode = 0;
    volatile uint8_t  reg_px = 0, reg_py = 0;
    volatile uint8_t  reg_cluster = 0;
    volatile uint8_t  reg_bank = 0;
    volatile uint8_t  reg_kern = 0;
    volatile uint32_t reg_instr = 0;
    volatile uint32_t reg_cycles = 0;
    volatile bool     reg_valid = false;

    volatile uint32_t reg_busy_mask = 0;
    volatile uint32_t reg_map[PE_Map::NUM_WORDS] = {0};
    volatile uint32_t reg_kmap[PE_Map::NUM_WORDS] = {0};
    volatile uint32_t reg_cfg_status = 0;
    volatile uint32_t reg_load_status = 0;
    volatile uint32_t reg_load_last = 0;

    // Stands in for the AXI DMA feeding instr_in.
    hls::stream<instr_beat_t> instr_stream;

    int errors = 0;

    std::cout << "--- Program Manager C-Sim ---" << std::endl;

    for (int cycle = 0; cycle < 60; cycle++) {
        // --- HOST LOGIC ---

        // Cycle 0: RUN before any RESET. num_loaded used to live in
        // uninitialized PE state, so this was dropped silently; the manager now
        // derives bank validity from the compiled-in layout instead.
        if (cycle == 0) {
            std::cout << "[Host] RUN single PE (7,7), bank 0, 1 cycle -- no prior RESET" << std::endl;
            reg_mode = 1; reg_tmode = TARGET_SINGLE_PE; reg_px = 7; reg_py = 7;
            reg_bank = 0; reg_cycles = 1;
            reg_valid = true;
        }
        if (cycle == 1) { reg_valid = false; }

        // Cycle 2: RESET the whole grid (single PE reset addressing (0,0) won't
        // cover everything, so we issue a per-cluster reset for each cluster id).
        if (cycle == 2) {
            std::cout << "[Host] RESET cluster 0" << std::endl;
            reg_mode = 2; reg_tmode = TARGET_CLUSTER; reg_cluster = 0;
            reg_valid = true;
        }
        if (cycle == 3) { reg_valid = false; }

        if (cycle == 4) {
            std::cout << "[Host] RESET cluster 1" << std::endl;
            reg_mode = 2; reg_tmode = TARGET_CLUSTER; reg_cluster = 1;
            reg_valid = true;
        }
        if (cycle == 5) { reg_valid = false; }

        // Cycle 10: RUN cluster 0 on bank 1 (kernel 102) for 10 cycles
        if (cycle == 10) {
            std::cout << "[Host] RUN cluster 0, bank 1, 10 cycles" << std::endl;
            reg_mode = 1; reg_tmode = TARGET_CLUSTER; reg_cluster = 0;
            reg_bank = 1; reg_cycles = 10;
            reg_valid = true;
        }
        if (cycle == 11) { reg_valid = false; }

        // Cycle 15: RUN a single PE in cluster 1 (5,5) on bank 2 (kernel 203) for 8 cycles
        if (cycle == 15) {
            std::cout << "[Host] RUN single PE (5,5), bank 2, 8 cycles" << std::endl;
            reg_mode = 1; reg_tmode = TARGET_SINGLE_PE; reg_px = 5; reg_py = 5;
            reg_bank = 2; reg_cycles = 8;
            reg_valid = true;
        }
        if (cycle == 16) { reg_valid = false; }

        // Cycle 30: switch cluster 0 straight to bank 0 (kernel 101), no reload needed
        if (cycle == 30) {
            std::cout << "[Host] RUN cluster 0, bank 0, 5 cycles (bank switch)" << std::endl;
            reg_mode = 1; reg_tmode = TARGET_CLUSTER; reg_cluster = 0;
            reg_bank = 0; reg_cycles = 5;
            reg_valid = true;
        }
        if (cycle == 31) { reg_valid = false; }

        // Try requesting an out-of-range bank on a single PE (should be rejected)
        if (cycle == 40) {
            std::cout << "[Host] RUN single PE (0,0), bank 3 (invalid, only 2 loaded)" << std::endl;
            reg_mode = 1; reg_tmode = TARGET_SINGLE_PE; reg_px = 0; reg_py = 0;
            reg_bank = 3; reg_cycles = 5;
            reg_valid = true;
        }
        if (cycle == 41) { reg_valid = false; }

        // --- HARDWARE EXECUTION ---
        program_manager_top(
            &reg_mode, &reg_tmode, &reg_px, &reg_py, &reg_cluster, &reg_bank,
            &reg_kern, &reg_instr, &reg_cycles, &reg_valid, instr_stream,
            &reg_busy_mask, (uint32_t *)reg_map, (uint32_t *)reg_kmap,
            &reg_cfg_status, &reg_load_status, &reg_load_last
        );

        // --- MONITORING ---
        if (cycle == 3 || cycle == 5 || cycle == 12 || cycle == 20 ||
            cycle == 17 || cycle == 24 || cycle == 32 || cycle == 42) {
            std::cout << "\n=== Cycle " << cycle << " ===" << std::endl;
            print_pe_grid((uint32_t *)reg_map);
        }

        // --- CHECKS ---
        // The cycle-0 RUN must have taken effect with no RESET before it.
        if (cycle == 0) {
            int flat = 7 * DIM_X + 7;
            bool busy = (reg_map[flat / 32] >> (flat % 32)) & 1;
            if (!busy) {
                std::cout << "FAIL: RUN before RESET was dropped" << std::endl;
                errors++;
            }
        }

        // At cycle 20 cluster-0's RUN (started cycle 10, 10 cycles) should be done.
        if (cycle == 20) {
            int flat = 0 * DIM_X + 0; // PE (0,0) is in cluster 0
            bool busy = (reg_map[flat / 32] >> (flat % 32)) & 1;
            if (busy) {
                std::cout << "FAIL: cluster 0 still busy at cycle 20" << std::endl;
                errors++;
            }
        }
        // At cycle 17 PE (5,5) should still be busy (started cycle 15, 8 cycles).
        if (cycle == 17) {
            int flat = 5 * DIM_X + 5;
            bool busy = (reg_map[flat / 32] >> (flat % 32)) & 1;
            if (!busy) {
                std::cout << "FAIL: PE (5,5) expected busy at cycle 17" << std::endl;
                errors++;
            }
        }
        // Invalid bank request at cycle 40 must not make PE (0,0) busy.
        if (cycle == 42) {
            int flat = 0;
            bool busy = (reg_map[flat / 32] >> (flat % 32)) & 1;
            if (busy) {
                std::cout << "FAIL: PE (0,0) accepted an out-of-range bank id" << std::endl;
                errors++;
            }
        }
    }

    // =====================================================================
    // PHASE 2 -- the runtime cluster table.
    //
    // Phase 1 above issues no MODE_CONFIG command at all, so it exercises the
    // compiled-in fallback and its output must stay byte-identical to what the
    // pre-config design printed. Everything from here on is new behaviour, driven
    // procedurally rather than by cycle number.
    // =====================================================================
    std::cout << "\n--- Phase 2: runtime cluster table ---" << std::endl;

    // One free-running cycle of the DUT.
    auto step = [&]() {
        program_manager_top(
            &reg_mode, &reg_tmode, &reg_px, &reg_py, &reg_cluster, &reg_bank,
            &reg_kern, &reg_instr, &reg_cycles, &reg_valid, instr_stream,
            &reg_busy_mask, (uint32_t *)reg_map, (uint32_t *)reg_kmap,
            &reg_cfg_status, &reg_load_status, &reg_load_last
        );
    };

    // Present a command for one cycle, then drop valid -- the manager edge-detects
    // the 0->1 transition, so valid must return low before the next command.
    auto cmd_full = [&](uint8_t mode, uint8_t tmode, uint8_t px, uint8_t py,
                        uint8_t clus, uint8_t bank, uint8_t kern,
                        uint32_t instr, uint32_t cyc) {
        reg_mode = mode; reg_tmode = tmode; reg_px = px; reg_py = py;
        reg_cluster = clus; reg_bank = bank; reg_kern = kern;
        reg_instr = instr; reg_cycles = cyc;
        reg_valid = true;  step();
        reg_valid = false; step();
    };
    auto cmd = [&](uint8_t mode, uint8_t tmode, uint8_t px, uint8_t py,
                   uint8_t clus, uint8_t bank, uint32_t cyc) {
        cmd_full(mode, tmode, px, py, clus, bank, 0, 0, cyc);
    };

    auto busy = [&](int x, int y) {
        int flat = y * DIM_X + x;
        return (bool)((reg_map[flat / 32] >> (flat % 32)) & 1);
    };

    auto has_kernel = [&](int x, int y) {
        int flat = y * DIM_X + x;
        return (bool)((reg_kmap[flat / 32] >> (flat % 32)) & 1);
    };

    // Point pe_x/pe_y/bank_id at one slot and let the readback settle. IDLE, so
    // nothing is dispatched -- cfg_status is a live view of the addressed entry.
    auto probe = [&](int x, int y, int bank) {
        cmd_full(MODE_IDLE, TARGET_SINGLE_PE, (uint8_t)x, (uint8_t)y,
                 0, (uint8_t)bank, 0, 0, 0);
    };
    auto tag_of  = [&]() { return (uint8_t)(reg_cfg_status & 0xff); };
    auto clus_of = [&]() { return (uint8_t)((reg_cfg_status >> 8) & 0xff); };
    // Running kernel on the addressed PE, 0 when it is idle.
    auto run_of  = [&]() { return (uint8_t)((reg_cfg_status >> 16) & 0xff); };
    auto nres_of = [&]() { return (uint8_t)((reg_cfg_status >> 24) & 0x7); };

    auto check = [&](bool ok, const char *what) {
        if (!ok) { std::cout << "FAIL: " << what << std::endl; errors++; }
    };

    // The layout under test: cluster 0 is a 2x2 corner, cluster 1 is the whole of
    // row y=3, and every other PE is a spare. Deliberately nothing like the
    // compiled-in halves, so a fallback that failed to be overridden is obvious.
    auto want_cluster = [](int x, int y) -> uint8_t {
        if (x < 2 && y < 2) return 0;
        if (y == 3)         return 1;
        return CLUSTER_SPARE;
    };
    // ...and its residency: cluster 0 holds kernels 101 and 102 in banks 0 and 1,
    // cluster 1 holds 201 in bank 0 alone, spares hold nothing.
    auto want_tag = [&](int x, int y, int b) -> uint8_t {
        uint8_t c = want_cluster(x, y);
        if (c == 0) return b == 0 ? 101 : (b == 1 ? 102 : KERNEL_NONE);
        if (c == 1) return b == 0 ? 201 : KERNEL_NONE;
        return KERNEL_NONE;
    };

    cmd(MODE_RESET, TARGET_CLUSTER, 0, 0, CLUSTER_ALL, 0, 0);

    // --- Upload, but do not commit yet. Membership and residency are separate
    // sub-ops: one cluster write per PE, one kernel write per bank.
    auto push_layout = [&]() {
        for (int x = 0; x < DIM_X; x++) {
            for (int y = 0; y < DIM_Y; y++) {
                cmd_full(MODE_CONFIG, CFG_CLUSTER, (uint8_t)x, (uint8_t)y,
                         want_cluster(x, y), 0, 0, 0, 0);
                for (int b = 0; b < MAX_BANKS; b++)
                    cmd_full(MODE_CONFIG, CFG_KERNEL, (uint8_t)x, (uint8_t)y,
                             0, (uint8_t)b, want_tag(x, y, b), 0, 0);
            }
        }
    };
    push_layout();

    // Readback must show the table written but not yet in force. Before the
    // commit, cfg_status reports the compiled-in fallback, not what was written.
    probe(0, 3, 0);
    check((reg_cfg_status & 0x80000000u) == 0, "cfg_loaded set before commit");
    check(clus_of() == 0,  "pre-commit readback should show the compiled-in cluster");
    check(tag_of()  == 101, "pre-commit readback should show the compiled-in tag");

    // An uncommitted table changes nothing: cluster 0 must still mean the
    // compiled-in left half.
    cmd(MODE_RUN, TARGET_CLUSTER, 0, 0, 0, 0, 4);
    check(busy(3, 6), "uncommitted table already in force (3,6) should be busy");
    check(!busy(5, 0), "uncommitted table already in force (5,0) should be idle");
    cmd(MODE_RESET, TARGET_CLUSTER, 0, 0, CLUSTER_ALL, 0, 0);

    // --- Commit.
    cmd_full(MODE_CONFIG, CFG_CLUSTER, CFG_COMMIT, 0, 0, 0, 0, 0, 0);
    probe(0, 3, 0);
    check((reg_cfg_status & 0x80000000u) != 0, "cfg_loaded clear after commit");
    check(clus_of() == 1,   "readback cluster wrong for (0,3) after commit");
    check(tag_of()  == 201, "readback tag wrong for (0,3) bank 0 after commit");
    probe(0, 3, 1);
    check(tag_of() == KERNEL_NONE, "(0,3) bank 1 should be empty");
    check(nres_of() == 1, "(0,3) resident count should be 1");
    probe(0, 0, 1);
    check(tag_of() == 102, "(0,0) bank 1 should hold kernel 102");
    check(nres_of() == 2, "(0,0) resident count should be 2");

    // Cluster 0 is now exactly the 2x2 corner.
    cmd(MODE_RUN, TARGET_CLUSTER, 0, 0, 0, 0, 20);
    for (int x = 0; x < DIM_X; x++) {
        for (int y = 0; y < DIM_Y; y++) {
            bool want = (want_cluster(x, y) == 0);
            if (busy(x, y) != want) {
                std::cout << "FAIL: after commit, PE (" << x << "," << y
                          << ") busy=" << busy(x, y) << " want=" << want << std::endl;
                errors++;
            }
        }
    }
    std::cout << "\n=== Committed layout, RUN cluster 0 ===" << std::endl;
    print_pe_grid((uint32_t *)reg_map);

    // A spare refuses a RUN even when named directly.
    cmd(MODE_RUN, TARGET_SINGLE_PE, 5, 5, 0, 0, 20);
    check(!busy(5, 5), "spare PE (5,5) accepted a direct RUN");

    // ...and is not reachable by cluster targeting under any id.
    cmd(MODE_RUN, TARGET_CLUSTER, 0, 0, CLUSTER_SPARE, 0, 20);
    check(!busy(5, 5), "spare PE (5,5) matched a CLUSTER_SPARE dispatch");

    // Bank policy comes from the table too: cluster 1 has one resident bank, so
    // bank 1 is empty for it while bank 1 is fine for cluster 0.
    cmd(MODE_RUN, TARGET_CLUSTER, 0, 0, 1, 1, 20);
    check(!busy(0, 3), "cluster 1 accepted bank 1 with only 1 bank loaded");
    cmd(MODE_RUN, TARGET_CLUSTER, 0, 0, 1, 0, 20);
    check(busy(0, 3), "cluster 1 refused its own bank 0");

    // Reconfiguring mid-flight must not perturb a countdown. (0,0) is 2 cycles
    // into a 20-cycle run at this point; move it to cluster 7 and confirm it
    // neither restarts nor stops.
    cmd_full(MODE_CONFIG, CFG_CLUSTER, 0, 0, 7, 0, 0, 0, 0);
    check(busy(0, 0), "config write stopped an in-flight PE");
    probe(0, 0, 0);
    check(clus_of() == 7, "mid-flight config write did not land");
    check(busy(0, 0), "in-flight PE went idle early after a config write");
    // Put it back so phase 3 starts from the layout described above.
    cmd_full(MODE_CONFIG, CFG_CLUSTER, 0, 0, 0, 0, 0, 0, 0);

    // CLUSTER_ALL reaches every PE, spares included -- this is what pm-app's
    // `reset` uses, and hardcoded per-cluster resets would miss the spares.
    cmd(MODE_RESET, TARGET_CLUSTER, 0, 0, CLUSTER_ALL, 0, 0);
    for (int x = 0; x < DIM_X; x++)
        for (int y = 0; y < DIM_Y; y++)
            check(!busy(x, y), "CLUSTER_ALL reset left a PE busy");

    // =====================================================================
    // PHASE 3 -- the AXI-Stream instruction path.
    //
    // Instructions are 64-bit VLIW words on instr_in, not AXI-Lite writes. The
    // manager counts beats and discards them (the PEs have no instruction memory
    // yet), commits the residency tag on TLAST, and optionally starts the PEs it
    // just loaded. instr_stream stands in for the AXI DMA.
    // =====================================================================
    std::cout << "\n--- Phase 3: AXI-Stream instruction load ---" << std::endl;

    // The pattern pm-app generates for a dummy instruction set: the low half is
    // the word index, so load_last after n beats must read n-1. A constant filler
    // would hide a dropped or duplicated beat; this does not.
    auto push_beats = [&](uint8_t kern, uint32_t n, uint32_t last_at) {
        for (uint32_t i = 0; i < n; i++) {
            instr_beat_t b;
            b.data = ((uint64_t)(0xD0000000u | kern) << 32) | i;
            b.keep = -1; b.strb = -1;   /* user/id/dest are disabled in ap_axiu<64,0,0,0> */
            b.last = (i == last_at);
            instr_stream.write(b);
        }
    };
    // One beat is consumed per cycle, so idling for n+2 cycles drains n beats.
    auto drain = [&](uint32_t n) {
        reg_valid = false;
        for (uint32_t i = 0; i < n + 2; i++) step();
    };
    auto ld_beats    = [&]() { return reg_load_status & 0x00ffffffu; };
    auto ld_active   = [&]() { return (reg_load_status & 0x80000000u) != 0; };
    auto ld_refused  = [&]() { return (reg_load_status & 0x40000000u) != 0; };
    auto ld_short    = [&]() { return (reg_load_status & 0x20000000u) != 0; };
    auto ld_over     = [&]() { return (reg_load_status & 0x10000000u) != 0; };

    // --- The residency query: which PEs hold kernel 101?
    cmd_full(MODE_IDLE, TARGET_SINGLE_PE, 0, 0, 0, 0, 101, 0, 0);
    for (int x = 0; x < DIM_X; x++)
        for (int y = 0; y < DIM_Y; y++)
            check(has_kernel(x, y) == (want_cluster(x, y) == 0),
                  "kernel 101 query bitmap disagrees with the table");

    // --- RUN_KERNEL: name the kernel, not the slot.
    cmd_full(MODE_RUN_KERNEL, TARGET_CLUSTER, 0, 0, 0, 0, 102, 0, 20);
    for (int x = 0; x < DIM_X; x++)
        for (int y = 0; y < DIM_Y; y++)
            check(busy(x, y) == (want_cluster(x, y) == 0),
                  "RUN_KERNEL 102 did not start exactly cluster 0");
    cmd(MODE_RESET, TARGET_CLUSTER, 0, 0, CLUSTER_ALL, 0, 0);

    // A kernel nobody holds starts nobody.
    cmd_full(MODE_RUN_KERNEL, TARGET_CLUSTER, 0, 0, CLUSTER_ALL, 0, 199, 0, 20);
    for (int x = 0; x < DIM_X; x++)
        for (int y = 0; y < DIM_Y; y++)
            check(!busy(x, y), "RUN_KERNEL started a PE for an absent kernel");

    // Two PEs holding the same kernel in *different* slots must both start from
    // one command -- the reason the slot lookup lives in hardware at all.
    cmd_full(MODE_CONFIG, CFG_KERNEL, 0, 0, 0, 3, 77, 0, 0);   // (0,0) bank 3
    cmd_full(MODE_CONFIG, CFG_KERNEL, 1, 1, 0, 1, 77, 0, 0);   // (1,1) bank 1
    cmd_full(MODE_RUN_KERNEL, TARGET_CLUSTER, 0, 0, 0, 0, 77, 0, 30);
    check(busy(0, 0) && busy(1, 1), "RUN_KERNEL missed a PE holding it elsewhere");
    check(!busy(1, 0) && !busy(0, 1), "RUN_KERNEL started a PE without the kernel");

    // --- The running kernel, latched at dispatch and reported per PE.
    // (0,0) and (1,1) are two cycles into the 30-cycle RUN_KERNEL 77 above.
    probe(0, 0, 0);
    check(run_of() == 77, "RUN_KERNEL did not record what it started");
    check((reg_cfg_status & (1u << 30)) != 0, "busy bit clear on a running PE");
    probe(1, 1, 0);
    check(run_of() == 77, "running kernel wrong on the PE holding it elsewhere");
    probe(1, 0, 0);
    check(run_of() == KERNEL_NONE, "an idle PE reported a running kernel");

    // A start aimed at an already-busy PE must not overwrite the record.
    cmd_full(MODE_RUN_KERNEL, TARGET_SINGLE_PE, 0, 0, 0, 0, 101, 0, 30);
    probe(0, 0, 0);
    check(run_of() == 77, "a refused start overwrote the running kernel");

    cmd(MODE_RESET, TARGET_CLUSTER, 0, 0, CLUSTER_ALL, 0, 0);
    probe(0, 0, 0);
    check(run_of() == KERNEL_NONE, "RESET left a stale running kernel");

    // RUN by bank records the tag in the bank it named, not the bank index.
    cmd(MODE_RUN, TARGET_CLUSTER, 0, 0, 0, 1, 30);
    probe(0, 1, 1);
    check(run_of() == 102, "RUN by bank did not record that bank's tag");
    cmd(MODE_RESET, TARGET_CLUSTER, 0, 0, CLUSTER_ALL, 0, 0);

    // Put (0,0) back to work: the refusal test below needs a busy target.
    cmd_full(MODE_RUN_KERNEL, TARGET_CLUSTER, 0, 0, 0, 0, 77, 0, 30);
    check(busy(0, 0), "(0,0) should be busy again before the refusal test");

    // --- A load aimed at a busy PE is refused whole, and changes nothing.
    cmd_full(MODE_LOAD, TARGET_SINGLE_PE, 0, 0, 0, 2, 88, 8, 0);
    check(ld_refused(), "load onto a busy PE was not refused");
    check(!ld_active(), "refused load opened a transfer anyway");
    probe(0, 0, 2);
    check(tag_of() == KERNEL_NONE, "refused load touched the destination bank");

    cmd(MODE_RESET, TARGET_CLUSTER, 0, 0, CLUSTER_ALL, 0, 0);

    // --- A full transfer onto a spare PE, which owns no kernels at all.
    check(!busy(5, 5), "(5,5) should be idle before the load");
    cmd_full(MODE_LOAD, TARGET_SINGLE_PE, 5, 5, 0, 0, 55, 8, 0);
    check(ld_active(), "load did not open");
    check(!ld_refused(), "load onto an idle PE was refused");

    // While the transfer is open the destination reads as empty and refuses work.
    probe(5, 5, 0);
    check(tag_of() == KERNEL_NONE, "bank mid-load did not read as empty");
    check((reg_cfg_status & (1u << 29)) != 0, "cfg_status did not flag the load set");
    cmd(MODE_RUN, TARGET_SINGLE_PE, 5, 5, 0, 0, 10);
    check(!busy(5, 5), "a PE mid-load accepted a RUN");

    push_beats(55, 8, 7);
    drain(8);
    check(!ld_active(), "transfer still open after TLAST");
    check(ld_beats() == 8, "wrong beat count after an 8-beat transfer");
    check(reg_load_last == 7, "load_last is not the final beat payload");
    check(!ld_short() && !ld_over(), "a complete transfer reported short/overrun");

    // The tag is now published, so the spare can run the kernel it was given.
    probe(5, 5, 0);
    check(tag_of() == 55, "tag not committed on TLAST");
    cmd(MODE_RUN, TARGET_SINGLE_PE, 5, 5, 0, 0, 10);
    check(busy(5, 5), "loaded PE refused a RUN on the bank just filled");
    cmd(MODE_RESET, TARGET_CLUSTER, 0, 0, CLUSTER_ALL, 0, 0);

    // --- Auto-start: commit and start in the same cycle.
    cmd_full(MODE_LOAD, TARGET_SINGLE_PE, 6, 6, 0, 1, 56, 4, 25);
    push_beats(56, 4, 3);
    drain(4);
    check(busy(6, 6), "auto-start did not start the PE it just loaded");
    probe(6, 6, 1);
    check(run_of() == 56, "auto-start did not record the kernel it delivered");

    // --- PE countdowns must advance while a transfer is in flight. (6,6) has just
    // started a 25-cycle run. Everything below is counted in DUT cycles: probe is
    // 2, a command is 2, drain(n) is n+2. If a transfer froze the grid the second
    // check would still find (6,6) busy.
    probe(6, 6, 1);                                            // 2  -> 2
    check(tag_of() == 56, "auto-start did not commit the tag");
    cmd_full(MODE_LOAD, TARGET_SINGLE_PE, 7, 7, 0, 0, 60, 12, 0);  // 2  -> 4
    push_beats(60, 12, 11);
    drain(12);                                                 // 14 -> 18
    check(busy(6, 6), "a 25-cycle run ended after 18 cycles");
    check(ld_beats() == 12, "12-beat transfer did not deliver 12 beats");
    drain(12);                                                 // 14 -> 32
    check(!busy(6, 6), "a 25-cycle run outlived 32 cycles: the transfer stalled the grid");

    // --- Short transfer: TLAST before the declared count commits nothing.
    cmd(MODE_RESET, TARGET_CLUSTER, 0, 0, CLUSTER_ALL, 0, 0);
    cmd_full(MODE_LOAD, TARGET_SINGLE_PE, 7, 0, 0, 2, 57, 8, 20);
    push_beats(57, 5, 4);          // TLAST on beat 5 of a declared 8
    drain(5);
    check(ld_short(), "a short transfer was not flagged");
    check(!ld_active(), "a short transfer stayed open");
    probe(7, 0, 2);
    check(tag_of() == KERNEL_NONE, "a short transfer committed its tag anyway");
    check(!busy(7, 0), "a short transfer auto-started the PE");

    // --- Overrun: more beats than declared.
    cmd_full(MODE_LOAD, TARGET_SINGLE_PE, 7, 1, 0, 2, 58, 3, 0);
    push_beats(58, 5, 4);          // declared 3, TLAST on beat 5
    drain(5);
    check(ld_over(), "an overrunning transfer was not flagged");
    probe(7, 1, 2);
    check(tag_of() == KERNEL_NONE, "an overrunning transfer committed its tag");

    // --- Abort releases PEs held by a transfer whose stream never ends.
    cmd_full(MODE_LOAD, TARGET_SINGLE_PE, 4, 4, 0, 0, 59, 16, 0);
    push_beats(59, 4, 99);         // no TLAST
    drain(4);
    check(ld_active(), "transfer closed without TLAST");
    cmd(MODE_RUN, TARGET_SINGLE_PE, 4, 4, 0, 0, 10);
    check(!busy(4, 4), "a PE held by an open transfer accepted a RUN");
    cmd(MODE_LOAD_ABORT, TARGET_SINGLE_PE, 0, 0, 0, 0, 0);
    check(!ld_active(), "abort did not close the transfer");
    probe(4, 4, 0);
    check(tag_of() == KERNEL_NONE, "abort committed a tag");

    cmd(MODE_RESET, TARGET_CLUSTER, 0, 0, CLUSTER_ALL, 0, 0);
    std::cout << "\n=== After phase 3 ===" << std::endl;
    print_pe_grid((uint32_t *)reg_map);

    if (errors == 0) {
        std::cout << "\nTest Passed: all checks OK." << std::endl;
        return 0;
    } else {
        std::cout << "\nTest Failed: " << errors << " check(s) failed." << std::endl;
        return 1;
    }
}
