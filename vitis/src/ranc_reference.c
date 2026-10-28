/* Independent event oracle for the fixed Fair mapping; no RTL/DMA dependency.
 * The exporter asserts threshold=1, negative threshold=0, linear asymmetric
 * reset, zero leak/initial state/delay for every neuron before emitting data.
 */
#include "ranc_reference.h"
#include "fixed_mapping.h"
#include <string.h>

static int32_t potential[RANC_NEURONS];
static uint8_t active[1024], next[1024];

/* Both signs use two branches; sign selects axon and each unit of magnitude adds a tick. */
int ranc_encode(const int32_t x[5], uint32_t words[RANC_MAX_PACKETS])
{
    int copy, i, t, count = 0;
    for (i = 0; i < 5; ++i) if (x[i] < -15 || x[i] > 15) return -1;
    for (copy = 0; copy < 2; ++copy)
        for (i = 0; i < 5; ++i) {
            int axon = 2*i + (x[i] < 0) + copy*10;
            int magnitude = x[i] < 0 ? -x[i] : x[i];
            for (t = 0; t < magnitude; ++t) words[count++] = (uint32_t)((axon << 4) | t);
        }
    return count;
}

/* Pure matrix arithmetic checks the spike computation without simulating neurons. */
void ranc_cpu_vmm(const int32_t x[5], int32_t y[8])
{
    int i, j;
    for (j = 0; j < 8; ++j) {
        y[j] = 0;
        for (i = 0; i < 5; ++i) y[j] += x[i] * ranc_matrix[i][j];
    }
}

/* Advance routed spikes one logical step at a time until the entire mapping drains. */
int ranc_reference(const int32_t x[5], uint16_t *masks, int capacity, int32_t *peak)
{
    int tick, i, a, quiet = 0;
    if (!masks || !peak || capacity < 2 || capacity > RANC_MAX_TICKS) return -1;
    for (i = 0; i < 5; ++i) if (x[i] < -15 || x[i] > 15) return -1;
    memset(potential, 0, sizeof potential);
    memset(active, 0, sizeof active);
    *peak = 0;
    masks[0] = 0; /* Initial silent interval, matching CPU row 1 / DMA tag 1. */
    for (tick = 1; tick < capacity; ++tick) {
        uint16_t mask = 0;
        int quiescent = 1;
        memset(next, 0, sizeof next);
        for (i = 0; i < 5; ++i) {
            int magnitude = x[i] < 0 ? -x[i] : x[i];
            if (tick <= magnitude) {
                a = 2*i + (x[i] < 0);
                active[a] = active[a+10] = 1;
            }
            if (tick < magnitude) quiescent = 0;
        }
        /* Spans index only the nonzero synapses driven by this active axon. */
        for (a = 0; a < 1024; ++a) if (active[a]) {
            struct ranc_span s = ranc_spans[a];
            unsigned e;
            for (e = s.first; e < (unsigned)s.first+s.count; ++e)
                potential[ranc_edges[e].neuron] += ranc_edges[e].weight;
        }
        for (i = 0; i < RANC_NEURONS; ++i) {
            int32_t p = potential[i], magnitude = p < 0 ? -p : p;
            if (magnitude > *peak) *peak = magnitude;
            if (p < -32768 || p > 32767) return -2;
            if (p >= 1) {
                struct ranc_node n = ranc_nodes[i];
                potential[i] = p-1;
                if (n.core == 4) {
                    if (n.axon >= 16 || (mask & (1u << n.axon))) return -3;
                    mask |= (uint16_t)(1u << n.axon);
                } else next[n.core*256+n.axon] = 1;
            }
            /* Linear asymmetric negative reset subtracts the negative
             * threshold (zero here), so negative potential is retained. */
            if (potential[i]) quiescent = 0;
        }
        for (a = 0; a < 1024; ++a) if (next[a]) quiescent = 0;
        memcpy(active, next, sizeof active);
        masks[tick] = mask;
        quiet = quiescent ? quiet+1 : 0;
        /* Require sustained silence, including internal state, before reusing the network. */
        if (quiet >= 8) return tick+1;
    }
    return -4;
}
