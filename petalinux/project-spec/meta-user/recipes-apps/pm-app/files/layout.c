#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "layout.h"

/* ---------------------------------------------------------------------------
 * File format
 *
 *   grid 8 8
 *
 *   kernel 101 instr 256                       # 256 generated dummy words
 *   kernel 102 instr 512 image /lib/firmware/k102.bin
 *
 *   cluster 0  at 0 0  size 4 8  kernels 101 102
 *   cluster 1  at 4 0  size 4 8  kernels 201 202 203
 *
 * A cluster is one or more rectangles given as origin + size. Any PE no
 * rectangle covers is a spare: it belongs to no cluster, so it matches no
 * cluster dispatch and refuses every RUN.
 *
 * Overlap between different clusters is a hard error rather than last-one-wins.
 * A silently mis-assigned PE is the failure that costs an afternoon on the
 * board, and it is free to catch here.
 *
 * `kernels` is the preload manifest, position by position: the first id goes in
 * bank 0 of every PE in the rectangle, the second in bank 1, and so on. 0 leaves
 * a slot empty. This is what the hardware enforces -- a RUN naming an empty bank
 * is refused -- so the list is no longer decoration.
 *
 * `kernel <id>` says what to actually stream at preload. Without such a line the
 * tag is written and nothing is transferred, which is the cheap path when there
 * are no instruction images yet.
 * ------------------------------------------------------------------------- */

#define ERR(...) do { snprintf(err, errlen, __VA_ARGS__); return -1; } while (0)
#define LERR(fmt, ...) ERR("%s:%d: " fmt, path, lineno, __VA_ARGS__)

void layout_default(struct pm_layout *l)
{
    int x, y;
    static const uint8_t left[MAX_BANKS]  = { 101, 102, 0, 0 };
    static const uint8_t right[MAX_BANKS] = { 201, 202, 203, 0 };

    memset(l, 0, sizeof(*l));
    for (y = 0; y < DIM_Y; y++) {
        for (x = 0; x < DIM_X; x++) {
            int b;
            int is_left = (x < DIM_X / 2);
            l->cluster[y][x] = is_left ? 0 : 1;
            for (b = 0; b < MAX_BANKS; b++)
                l->kernel[y][x][b] = is_left ? left[b] : right[b];
        }
    }
    l->declared[0] = l->declared[1] = 1;
    l->n_spare = 0;
    /* No `kernel` lines: tags only, nothing streamed. The hardware's own
     * fallback is this same layout, so with no file there is nothing to upload
     * and nothing to transfer. */
}

int layout_slot_of(const struct pm_layout *l, int x, int y, uint8_t kernel)
{
    int b;

    if (kernel == KERNEL_NONE)
        return -1;
    for (b = 0; b < MAX_BANKS; b++)
        if (l->kernel[y][x][b] == kernel)
            return b;
    return -1;
}

int layout_free_slot(const struct pm_layout *l, int x, int y)
{
    int b;

    for (b = 0; b < MAX_BANKS; b++)
        if (l->kernel[y][x][b] == KERNEL_NONE)
            return b;
    return -1;
}

int layout_resident(const struct pm_layout *l, int x, int y)
{
    int b, n = 0;

    for (b = 0; b < MAX_BANKS; b++)
        if (l->kernel[y][x][b] != KERNEL_NONE)
            n++;
    return n;
}

void layout_gen_words(uint64_t *buf, uint32_t n, uint8_t kernel)
{
    uint32_t i;

    for (i = 0; i < n; i++)
        buf[i] = ((uint64_t)(0xD0000000u | kernel) << 32) | i;
}

/* strtol wrapper: rejects trailing junk, which plain atoi would swallow. */
static int parse_int(const char *tok, long *out)
{
    char *end;
    long v;

    if (!tok || !*tok)
        return -1;
    v = strtol(tok, &end, 10);
    if (*end)
        return -1;
    *out = v;
    return 0;
}

static int next_int(char **save, long *out)
{
    return parse_int(strtok_r(NULL, " \t\r\n", save), out);
}

int layout_parse(const char *path, struct pm_layout *l, char *err, size_t errlen)
{
    struct pm_layout tmp;
    char line[512];
    FILE *f;
    int lineno = 0;
    int saw_grid = 0;
    int x, y, b;

    f = fopen(path, "r");
    if (!f)
        ERR("%s: cannot open", path);

    /* Build into a scratch copy so a bad file cannot leave a half-applied
     * layout behind -- *l is only touched once the whole file validates. */
    memset(&tmp, 0, sizeof(tmp));
    for (y = 0; y < DIM_Y; y++)
        for (x = 0; x < DIM_X; x++)
            tmp.cluster[y][x] = CLUSTER_SPARE;

    while (fgets(line, sizeof(line), f)) {
        char *save = NULL, *tok, *hash;
        long id, ax = -1, ay = -1, sw = -1, sh = -1, nb = -1;
        int have_at = 0, have_size = 0, nk = 0;
        uint8_t klist[MAX_BANKS];

        lineno++;
        if ((hash = strchr(line, '#')))
            *hash = '\0';

        tok = strtok_r(line, " \t\r\n", &save);
        if (!tok)
            continue;                     /* blank or comment-only */

        if (!strcmp(tok, "grid")) {
            long gx, gy;
            if (next_int(&save, &gx) || next_int(&save, &gy)) {
                fclose(f);
                LERR("grid needs two integers%s", "");
            }
            if (gx != DIM_X || gy != DIM_Y) {
                fclose(f);
                LERR("grid %ld %ld does not match the bitstream's %dx%d -- "
                     "grid size is fixed at synthesis, rebuild with "
                     "GRID_DIM=%ld to change it", gx, gy, DIM_X, DIM_Y, gx);
            }
            saw_grid = 1;
            continue;
        }

        /* kernel <id> [instr <n>] [image <path>] -- what to stream at preload. */
        if (!strcmp(tok, "kernel")) {
            struct kernel_info *ki;

            if (next_int(&save, &id) || id < 1 || id > MAX_KERNEL_ID) {
                fclose(f);
                LERR("kernel id must be 1..%d (the residency table is one byte "
                     "per bank)", MAX_KERNEL_ID);
            }
            ki = &tmp.kern[id];
            ki->src = KSRC_GEN;           /* an id with no image is generated */

            while ((tok = strtok_r(NULL, " \t\r\n", &save))) {
                if (!strcmp(tok, "instr")) {
                    long n;
                    if (next_int(&save, &n) || n < 0) {
                        fclose(f);
                        LERR("kernel %ld: 'instr' needs a non-negative count", id);
                    }
                    ki->n_instr = (uint32_t)n;
                } else if (!strcmp(tok, "image")) {
                    const char *p = strtok_r(NULL, " \t\r\n", &save);
                    if (!p) {
                        fclose(f);
                        LERR("kernel %ld: 'image' needs a path", id);
                    }
                    if (strlen(p) >= MAX_IMAGE_PATH) {
                        fclose(f);
                        LERR("kernel %ld: image path longer than %d characters",
                             id, MAX_IMAGE_PATH - 1);
                    }
                    strcpy(ki->image, p);
                    ki->src = KSRC_IMAGE;
                } else {
                    fclose(f);
                    LERR("unexpected '%s' in a kernel line", tok);
                }
            }
            if (ki->n_instr == 0) {
                /* Nothing to send is not an error, but it is almost certainly a
                 * typo, and silently skipping the transfer would be confusing. */
                fclose(f);
                LERR("kernel %ld declares no instructions -- give it "
                     "'instr <n>', or drop the line to preload the tag alone", id);
            }
            continue;
        }

        if (strcmp(tok, "cluster")) {
            fclose(f);
            LERR("unknown directive '%s' (expected 'grid', 'kernel' or 'cluster')",
                 tok);
        }

        if (next_int(&save, &id) || id < 0 || id >= MAX_CLUSTERS) {
            fclose(f);
            LERR("cluster id must be 0..%d", MAX_CLUSTERS - 1);
        }

        memset(klist, 0, sizeof(klist));

        while ((tok = strtok_r(NULL, " \t\r\n", &save))) {
            if (!strcmp(tok, "at")) {
                if (next_int(&save, &ax) || next_int(&save, &ay)) {
                    fclose(f);
                    LERR("'at' needs x and y%s", "");
                }
                have_at = 1;
            } else if (!strcmp(tok, "size")) {
                if (next_int(&save, &sw) || next_int(&save, &sh)) {
                    fclose(f);
                    LERR("'size' needs width and height%s", "");
                }
                have_size = 1;
            } else if (!strcmp(tok, "banks")) {
                /* Kept for files written before residency was per bank. It is
                 * now derived from the kernel list and only cross-checked. */
                if (next_int(&save, &nb)) {
                    fclose(f);
                    LERR("'banks' needs a count%s", "");
                }
            } else if (!strcmp(tok, "kernels")) {
                /* Consumes the rest of the line. Position is the bank index. */
                long k;
                while (!next_int(&save, &k)) {
                    if (nk >= MAX_BANKS) {
                        fclose(f);
                        LERR("more than MAX_BANKS (%d) kernel ids", MAX_BANKS);
                    }
                    if (k < 0 || k > MAX_KERNEL_ID) {
                        fclose(f);
                        LERR("kernel id %ld out of range (1..%d, or 0 for an "
                             "empty bank)", k, MAX_KERNEL_ID);
                    }
                    klist[nk++] = (uint8_t)k;
                }
            } else {
                fclose(f);
                LERR("unexpected '%s' in a cluster line", tok);
            }
        }

        if (!have_at || !have_size) {
            fclose(f);
            LERR("cluster %ld needs 'at' and 'size'", id);
        }
        if (nk == 0) {
            fclose(f);
            LERR("cluster %ld lists no kernels -- every one of its PEs would "
                 "refuse every RUN", id);
        }
        if (nb >= 0 && nb != nk) {
            fclose(f);
            LERR("cluster %ld declares %ld banks but %d kernel ids -- 'banks' is "
                 "now derived from the list, so drop it or make them agree",
                 id, nb, nk);
        }
        if (sw < 1 || sh < 1) {
            fclose(f);
            LERR("cluster %ld: size must be at least 1x1", id);
        }
        if (ax < 0 || ay < 0 || ax + sw > DIM_X || ay + sh > DIM_Y) {
            fclose(f);
            LERR("cluster %ld: %ldx%ld at (%ld,%ld) runs off the %dx%d grid",
                 id, sw, sh, ax, ay, DIM_X, DIM_Y);
        }

        for (y = (int)ay; y < (int)(ay + sh); y++) {
            for (x = (int)ax; x < (int)(ax + sw); x++) {
                uint8_t owner = tmp.cluster[y][x];
                if (owner != CLUSTER_SPARE && owner != (uint8_t)id) {
                    fclose(f);
                    LERR("PE (%d,%d) claimed by both cluster %u and cluster %ld",
                         x, y, owner, id);
                }
                tmp.cluster[y][x] = (uint8_t)id;
                for (b = 0; b < MAX_BANKS; b++)
                    tmp.kernel[y][x][b] = klist[b];
            }
        }
        tmp.declared[id] = 1;
    }
    fclose(f);

    if (!saw_grid)
        ERR("%s: no 'grid' line -- refusing to guess the grid size", path);

    for (y = 0; y < DIM_Y; y++)
        for (x = 0; x < DIM_X; x++)
            if (tmp.cluster[y][x] == CLUSTER_SPARE)
                tmp.n_spare++;

    *l = tmp;
    return 0;
}

/* 0-9 then a-z then A-Z, so ids stay one column wide up to MAX_CLUSTERS. */
char layout_cluster_char(uint8_t c)
{
    if (c == CLUSTER_SPARE) return '.';
    if (c < 10)             return (char)('0' + c);
    if (c < 36)             return (char)('a' + c - 10);
    return (char)('A' + c - 36);
}

void layout_print(const struct pm_layout *l)
{
    int x, y, c, b, k;

    printf("   Cluster Map (x ->):\n");
    for (y = 0; y < DIM_Y; y++) {
        printf("   |");
        for (x = 0; x < DIM_X; x++)
            printf(" %c", layout_cluster_char(l->cluster[y][x]));
        printf("|\n");
    }

    for (c = 0; c < MAX_CLUSTERS; c++) {
        int n = 0, fx = -1, fy = -1;
        if (!l->declared[c])
            continue;
        for (y = 0; y < DIM_Y; y++) {
            for (x = 0; x < DIM_X; x++) {
                if (l->cluster[y][x] == c) {
                    n++;
                    if (fx < 0) { fx = x; fy = y; }
                }
            }
        }
        printf("   cluster %d ('%c'): %d PEs, banks [", c, layout_cluster_char((uint8_t)c), n);
        for (b = 0; b < MAX_BANKS && fx >= 0; b++) {
            uint8_t id = l->kernel[fy][fx][b];
            printf("%s%s", b ? " " : "", id ? "" : "-");
            if (id) printf("%u", id);
        }
        printf("]\n");
    }

    /* Which kernels get streamed at preload, as opposed to merely declared. */
    for (k = 1; k < MAX_KERNELS; k++) {
        const struct kernel_info *ki = &l->kern[k];
        if (ki->src == KSRC_NONE)
            continue;
        printf("   kernel %d: %u instructions from %s\n", k, ki->n_instr,
               ki->src == KSRC_IMAGE ? ki->image : "a generated pattern");
    }

    if (l->n_spare)
        printf("   spare ('.'): %d PEs, in no cluster\n", l->n_spare);
}
