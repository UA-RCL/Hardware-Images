// Unit test for a single PE, with no manager and no grid in sight. This is the
// point of the split: before it, a PE could not be exercised on its own.

#include <iostream>
#include "pe_unit.hpp"

static int failures = 0;

static void check(bool cond, const char *what) {
    if (!cond) {
        std::cout << "FAIL: " << what << std::endl;
        failures++;
    }
}

// Ticks with start low until the PE goes idle; returns how many ticks it was
// busy for, counting the tick work started on.
static int run_to_idle(PE_State s, int limit = 64) {
    int busy_ticks = s.busy ? 1 : 0;
    for (int i = 0; i < limit && s.busy; i++) {
        s = pe_tick(s, false, false, 0);
        if (s.busy) busy_ticks++;
    }
    return busy_ticks;
}

int main() {
    std::cout << "--- PE unit test ---" << std::endl;

    // Fresh state is idle.
    {
        PE_State s = pe_initial_state();
        check(!s.busy, "fresh PE is idle");
    }

    // A start with no prior reset is accepted -- the PE holds no layout state
    // that has to be seeded first.
    {
        PE_State s = pe_initial_state();
        s = pe_tick(s, true, false, 3);
        check(s.busy, "start is accepted from power-up state");
        check(run_to_idle(s) == 3, "busy for exactly `cycles` ticks");
    }

    // Zero-cycle requests still occupy the PE for one tick.
    {
        PE_State s = pe_tick(pe_initial_state(), true, false, 0);
        check(s.busy, "zero-cycle request starts");
        check(run_to_idle(s) == 1, "zero-cycle request lasts one tick");
    }

    // A single-cycle request behaves the same way.
    {
        PE_State s = pe_tick(pe_initial_state(), true, false, 1);
        check(run_to_idle(s) == 1, "one-cycle request lasts one tick");
    }

    // A busy PE ignores start, and does not adopt the new length.
    {
        PE_State s = pe_tick(pe_initial_state(), true, false, 5);
        s = pe_tick(s, true, false, 1);   // would finish early if adopted
        check(s.busy, "still busy after ignored start");
        check(s.target_cycles == 5, "latched length survives an ignored start");
        check(run_to_idle(s) + 1 == 5, "original length is served in full");
    }

    // clear abandons work in flight.
    {
        PE_State s = pe_tick(pe_initial_state(), true, false, 100);
        s = pe_tick(s, false, true, 0);
        check(!s.busy, "clear aborts a run in flight");
        check(s.count == 0 && s.target_cycles == 0, "clear wipes the counters");
    }

    // clear on an idle PE is harmless.
    {
        PE_State s = pe_tick(pe_initial_state(), false, true, 0);
        check(!s.busy, "clear on an idle PE keeps it idle");
    }

    // clear wins over a simultaneous start.
    {
        PE_State s = pe_tick(pe_initial_state(), true, true, 10);
        check(!s.busy, "clear takes precedence over start");
    }

    // An idle PE left alone stays idle.
    {
        PE_State s = pe_initial_state();
        for (int i = 0; i < 10; i++) s = pe_tick(s, false, false, 0);
        check(!s.busy, "idle PE stays idle without a start");
    }

    if (failures == 0) {
        std::cout << "Test Passed: all checks OK." << std::endl;
        return 0;
    }
    std::cout << "Test Failed: " << failures << " check(s) failed." << std::endl;
    return 1;
}
