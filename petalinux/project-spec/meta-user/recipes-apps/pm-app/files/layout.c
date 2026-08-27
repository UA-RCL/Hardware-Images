#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "layout.h"

/* ---------------------------------------------------------------------------
 * File format
 *
 *   grid 8 8
 *   cluster 0  at 0 0  size 4 8  banks 2  kernels 101 102
 *   cluster 1  at 4 0  size 4 8  banks 3  kernels 201 202 203
 *
 * A cluster is one or more rectangles given as origin + size. Any PE no
 * rectangle covers is a spare: it belongs to no cluster, so it matches no
 * cluster dispatch and refuses every RUN.
 *
 * Overlap between different clusters is a hard error rather than last-one-wins.
 * A silently mis-assigned PE is the failure that costs an afternoon on the
 * board, and it is free to catch here.
 *
 * `kernels` records which kernel is resident in each bank. The hardware has no
 * such notion -- bank_kernel_id has never had behavioural effect -- so it is
 * carried for display and for cross-checking against `banks`.
 * ------------------------------------------------------------------------- */

#define ERR(...) do { snprintf(err, errlen, __VA_ARGS__); return -1; } while (0)
#define LERR(fmt, ...) ERR("%s:%d: " fmt, path, lineno, __VA_ARGS__)

void layout_default(struct pm_layout *l)
{
    int x, y, b;

    memset(l, 0, sizeof(*l));
    for (y = 0; y < DIM_Y; y++) {
        for (x = 0; x < DIM_X; x++) {
            int left = (x < DIM_X / 2);
            l->cluster[y][x] = left ? 0 : 1;
            l->banks[y][x]   = left ? 2 : 3;
        }
    }
    l->declared[0] = l->declared[1] = 1;
    l->n_spare = 0;

    for (b = 0; b < MAX_BANKS; b++)
        l->kernels[0][b] = l->kernels[1][b] = 0;
    l->kernels[0][0] = 101; l->kernels[0][1] = 102;
    l->kernels[1][0] = 201; l->kernels[1][1] = 202; l->kernels[1][2] = 203;
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
    int x, y;

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
        int have_at = 0, have_size = 0, have_banks = 0, nk = 0;

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

        if (strcmp(tok, "cluster")) {
            fclose(f);
            LERR("unknown directive '%s' (expected 'grid' or 'cluster')", tok);
        }

        if (next_int(&save, &id) || id < 0 || id >= MAX_CLUSTERS) {
            fclose(f);
            LERR("cluster id must be 0..%d", MAX_CLUSTERS - 1);
        }

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
                if (next_int(&save, &nb)) {
                    fclose(f);
                    LERR("'banks' needs a count%s", "");
                }
                have_banks = 1;
            } else if (!strcmp(tok, "kernels")) {
                /* Consumes the rest of the line. */
                long k;
                while (!next_int(&save, &k)) {
                    if (nk >= MAX_BANKS) {
                        fclose(f);
                        LERR("more than MAX_BANKS (%d) kernel ids", MAX_BANKS);
                    }
                    if (k < 0 || k > 0xffff) {
                        fclose(f);
                        LERR("kernel id %ld out of range", k);
                    }
                    tmp.kernels[id][nk++] = (uint16_t)k;
                }
            } else {
                fclose(f);
                LERR("unexpected '%s' in a cluster line", tok);
            }
        }

        if (!have_at || !have_size || !have_banks) {
            fclose(f);
            LERR("cluster %ld needs 'at', 'size' and 'banks'", id);
        }
        if (nb < 0 || nb > MAX_BANKS) {
            fclose(f);
            LERR("cluster %ld: banks must be 0..%d", id, MAX_BANKS);
        }
        if (nk && nk != nb) {
            fclose(f);
            LERR("cluster %ld declares %d banks but %d kernel ids", id, (int)nb, nk);
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
                tmp.banks[y][x]   = (uint8_t)nb;
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
static char cluster_char(uint8_t c)
{
    if (c == CLUSTER_SPARE) return '.';
    if (c < 10)             return (char)('0' + c);
    if (c < 36)             return (char)('a' + c - 10);
    return (char)('A' + c - 36);
}

void layout_print(const struct pm_layout *l)
{
    int x, y, c, b;

    printf("   Cluster Map (x ->):\n");
    for (y = 0; y < DIM_Y; y++) {
        printf("   |");
        for (x = 0; x < DIM_X; x++)
            printf(" %c", cluster_char(l->cluster[y][x]));
        printf("|\n");
    }

    for (c = 0; c < MAX_CLUSTERS; c++) {
        int n = 0;
        int banks = 0;
        if (!l->declared[c])
            continue;
        for (y = 0; y < DIM_Y; y++)
            for (x = 0; x < DIM_X; x++)
                if (l->cluster[y][x] == c) { n++; banks = l->banks[y][x]; }
        printf("   cluster %d ('%c'): %d PEs, %d bank%s",
               c, cluster_char((uint8_t)c), n, banks, banks == 1 ? "" : "s");
        if (l->kernels[c][0]) {
            printf(" [");
            for (b = 0; b < banks && b < MAX_BANKS; b++)
                printf("%s%u", b ? " " : "", l->kernels[c][b]);
            printf("]");
        }
        printf("\n");
    }
    if (l->n_spare)
        printf("   spare ('.'): %d PEs, in no cluster\n", l->n_spare);
}
