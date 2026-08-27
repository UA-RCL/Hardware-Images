/*
 * layout.h -- the cluster layout pm-app uploads to the program manager.
 *
 * Cluster membership used to be compiled into the bitstream (grid_layout_entry()
 * in the HLS sources). It is now a runtime table: this parses a text file into a
 * per-PE map, and pm-app.c pushes it over the command bus with MODE_CONFIG before
 * the shell opens. Re-clustering the grid therefore needs no Vitis and no Vivado.
 *
 * Grid dimensions are still fixed at synthesis -- DIM_X/DIM_Y size the hardware's
 * register arrays -- so a layout file partitions the compiled grid, it cannot
 * grow it. That is why `grid` in the file is checked rather than obeyed.
 */
#ifndef PM_LAYOUT_PARSE_H
#define PM_LAYOUT_PARSE_H

#include <stdint.h>
#include <stddef.h>

#include "pm_layout.h"   /* DIM_X, DIM_Y, MAX_BANKS, NUM_CLUSTERS */

/* Mirrors the reserved ids in vitis_hls/program_manager/include/dispatch.hpp. */
#define CLUSTER_SPARE  255u   /* in no cluster: matches nothing, runs nothing */
#define CLUSTER_ALL    254u   /* targets every PE, spares included */

/* Ids above this are reserved, so a file may not use them. */
#define MAX_CLUSTERS   64

struct pm_layout {
    uint8_t  cluster[DIM_Y][DIM_X];              /* CLUSTER_SPARE = unassigned */
    uint8_t  banks  [DIM_Y][DIM_X];              /* resident banks, 0 for a spare */
    uint16_t kernels[MAX_CLUSTERS][MAX_BANKS];   /* display + validation only */
    uint8_t  declared[MAX_CLUSTERS];             /* 1 if the file declared this id */
    int      n_spare;
};

/* The compiled-in default: cluster 0 is the left half, cluster 1 the right.
 * Used when no layout file is present, and it matches what the hardware falls
 * back to before a table is committed, so the two cannot disagree. */
void layout_default(struct pm_layout *l);

/* Returns 0 on success. On failure writes a one-line reason into err (which
 * includes the offending line number) and leaves *l untouched. */
int layout_parse(const char *path, struct pm_layout *l, char *err, size_t errlen);

/* Human-readable map: one character per PE, '.' for a spare. */
void layout_print(const struct pm_layout *l);

#endif
