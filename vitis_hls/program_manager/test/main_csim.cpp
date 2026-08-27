#include <iostream>
#include <bitset>
#include "dispatch.hpp"
#include "grid_utils.hpp"

// Prototype matching the AXI-Lite pointer interface
void program_manager_top(
    volatile uint8_t  *mode,
    volatile uint8_t  *target_mode,
    volatile uint8_t  *pe_x,
    volatile uint8_t  *pe_y,
    volatile uint8_t  *cluster_id,
    volatile uint8_t  *bank_id,
    volatile uint32_t *cycles,
    volatile bool     *valid_signal,
    volatile uint32_t *busy_mask_out,
    volatile uint32_t *busy_map_out_bits,
    volatile uint32_t *cfg_status
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
    volatile uint32_t reg_cycles = 0;
    volatile bool     reg_valid = false;

    volatile uint32_t reg_busy_mask = 0;
    volatile uint32_t reg_map[PE_Map::NUM_WORDS] = {0};
    volatile uint32_t reg_cfg_status = 0;

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
            &reg_cycles, &reg_valid, &reg_busy_mask, (uint32_t *)reg_map,
            &reg_cfg_status
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
            &reg_cycles, &reg_valid, &reg_busy_mask, (uint32_t *)reg_map,
            &reg_cfg_status
        );
    };

    // Present a command for one cycle, then drop valid -- the manager edge-detects
    // the 0->1 transition, so valid must return low before the next command.
    auto cmd = [&](uint8_t mode, uint8_t tmode, uint8_t px, uint8_t py,
                   uint8_t clus, uint8_t bank, uint32_t cyc) {
        reg_mode = mode; reg_tmode = tmode; reg_px = px; reg_py = py;
        reg_cluster = clus; reg_bank = bank; reg_cycles = cyc;
        reg_valid = true;  step();
        reg_valid = false; step();
    };

    auto busy = [&](int x, int y) {
        int flat = y * DIM_X + x;
        return (bool)((reg_map[flat / 32] >> (flat % 32)) & 1);
    };

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
    auto want_banks = [&](int x, int y) -> uint8_t {
        uint8_t c = want_cluster(x, y);
        return c == 0 ? 2 : (c == 1 ? 1 : 0);
    };

    cmd(MODE_RESET, TARGET_CLUSTER, 0, 0, CLUSTER_ALL, 0, 0);

    // --- Upload, but do not commit yet.
    for (int x = 0; x < DIM_X; x++)
        for (int y = 0; y < DIM_Y; y++)
            cmd(MODE_CONFIG, 0, (uint8_t)x, (uint8_t)y,
                want_cluster(x, y), want_banks(x, y), 0);

    // Readback must show the table written but not yet in force.
    cmd(MODE_IDLE, TARGET_SINGLE_PE, 0, 3, 0, 0, 0);
    check((reg_cfg_status & 0x80000000u) == 0, "cfg_loaded set before commit");
    check(((reg_cfg_status >> 8) & 0xff) == 1,  "readback cluster wrong for (0,3)");
    check((reg_cfg_status & 0xff) == 1,         "readback banks wrong for (0,3)");

    // An uncommitted table changes nothing: cluster 0 must still mean the
    // compiled-in left half.
    cmd(MODE_RUN, TARGET_CLUSTER, 0, 0, 0, 0, 4);
    check(busy(3, 6), "uncommitted table already in force (3,6) should be busy");
    check(!busy(5, 0), "uncommitted table already in force (5,0) should be idle");
    cmd(MODE_RESET, TARGET_CLUSTER, 0, 0, CLUSTER_ALL, 0, 0);

    // --- Commit.
    cmd(MODE_CONFIG, 0, CFG_COMMIT, 0, 0, 0, 0);
    cmd(MODE_IDLE, TARGET_SINGLE_PE, 0, 3, 0, 0, 0);
    check((reg_cfg_status & 0x80000000u) != 0, "cfg_loaded clear after commit");

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

    // Bank policy comes from the table too: cluster 1 has one bank, so bank 1 is
    // out of range for it while bank 1 is fine for cluster 0.
    cmd(MODE_RUN, TARGET_CLUSTER, 0, 0, 1, 1, 20);
    check(!busy(0, 3), "cluster 1 accepted bank 1 with only 1 bank loaded");
    cmd(MODE_RUN, TARGET_CLUSTER, 0, 0, 1, 0, 20);
    check(busy(0, 3), "cluster 1 refused its own bank 0");

    // Reconfiguring mid-flight must not perturb a countdown. (0,0) is 2 cycles
    // into a 20-cycle run at this point; move it to cluster 7 and confirm it
    // neither restarts nor stops.
    cmd(MODE_CONFIG, 0, 0, 0, 7, 3, 0);
    check(busy(0, 0), "config write stopped an in-flight PE");
    cmd(MODE_IDLE, TARGET_SINGLE_PE, 0, 0, 0, 0, 0);
    check(((reg_cfg_status >> 8) & 0xff) == 7, "mid-flight config write did not land");
    check(busy(0, 0), "in-flight PE went idle early after a config write");

    // CLUSTER_ALL reaches every PE, spares included -- this is what pm-app's
    // `reset` uses, and hardcoded per-cluster resets would miss the spares.
    cmd(MODE_RESET, TARGET_CLUSTER, 0, 0, CLUSTER_ALL, 0, 0);
    for (int x = 0; x < DIM_X; x++)
        for (int y = 0; y < DIM_Y; y++)
            check(!busy(x, y), "CLUSTER_ALL reset left a PE busy");

    if (errors == 0) {
        std::cout << "\nTest Passed: all checks OK." << std::endl;
        return 0;
    } else {
        std::cout << "\nTest Failed: " << errors << " check(s) failed." << std::endl;
        return 1;
    }
}
