/*
 * layout.h -- the cluster + kernel residency layout pm-app uploads to the
 * program manager.
 *
 * Cluster membership used to be compiled into the bitstream (grid_layout_entry()
 * in the HLS sources). Both membership and residency are now runtime tables: this
 * parses a text file into a per-PE map, and pm-app.c pushes it over the command
 * bus with MODE_CONFIG before the shell opens. Re-clustering the grid, and
 * deciding which kernel sits in which bank, therefore need no Vitis and no Vivado.
 *
 * Residency is per bank, not a count: cfg_kernel[x][y][bank] holds a kernel id or
 * 0. That is what lets a RUN name a kernel instead of a slot, and it is why the
 * `kernels` list in the file is now the preload manifest rather than decoration.
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
#define KERNEL_NONE      0u   /* an empty bank */

/* Ids above this are reserved, so a file may not use them. */
#define MAX_CLUSTERS   64

/* The residency table is one byte per bank, so a kernel id must fit in one. */
#define MAX_KERNEL_ID  255
#define MAX_KERNELS    256

#define MAX_IMAGE_PATH 128

/* Where a kernel's instructions come from when it is preloaded. The hardware
 * discards them either way -- the PEs have no instruction memory yet -- but the
 * transfer itself is real DMA traffic over the AXI-Stream port. */
enum kernel_src {
    KSRC_NONE = 0,   /* declare the tag only, stream nothing */
    KSRC_GEN,        /* n generated dummy VLIW words */
    KSRC_IMAGE       /* n words read from a file */
};

struct kernel_info {
    uint8_t  src;                        /* enum kernel_src */
    uint32_t n_instr;                    /* 64-bit instruction words */
    char     image[MAX_IMAGE_PATH];      /* only when src == KSRC_IMAGE */
};

struct pm_layout {
    uint8_t  cluster[DIM_Y][DIM_X];              /* CLUSTER_SPARE = unassigned */
    uint8_t  kernel [DIM_Y][DIM_X][MAX_BANKS];   /* kernel id per bank, 0 = empty */
    uint8_t  declared[MAX_CLUSTERS];             /* 1 if the file declared this id */
    struct kernel_info kern[MAX_KERNELS];        /* indexed by kernel id */
    int      n_spare;
};

/* The compiled-in default: cluster 0 is the left half with kernels 101/102,
 * cluster 1 the right half with 201/202/203. Used when no layout file is present,
 * and it matches what the hardware falls back to before a table is committed, so
 * the two cannot disagree. */
void layout_default(struct pm_layout *l);

/* Returns 0 on success. On failure writes a one-line reason into err (which
 * includes the offending line number) and leaves *l untouched. */
int layout_parse(const char *path, struct pm_layout *l, char *err, size_t errlen);

/* Human-readable map: one character per PE, '.' for a spare. */
void layout_print(const struct pm_layout *l);

/* The glyph layout_print() uses for a cluster id: 0-9, then a-z, then A-Z, and
 * '.' for a spare. Shared so pm-app's hardware-read maps look the same. */
char layout_cluster_char(uint8_t c);

/* The lowest-numbered bank of this PE holding `kernel`, or -1. Mirrors
 * kernel_slot() in the hardware. */
int layout_slot_of(const struct pm_layout *l, int x, int y, uint8_t kernel);

/* The lowest-numbered empty bank of this PE, or -1 if all are occupied. */
int layout_free_slot(const struct pm_layout *l, int x, int y);

/* How many of this PE's banks hold something. */
int layout_resident(const struct pm_layout *l, int x, int y);

/* Fill `buf` with n generated dummy VLIW words for kernel k. Word i is
 * (0xD0000000 | k) << 32 | i, so the manager's load_last must read n-1 after the
 * transfer and a dropped or duplicated beat is visible. */
void layout_gen_words(uint64_t *buf, uint32_t n, uint8_t kernel);

#endif
