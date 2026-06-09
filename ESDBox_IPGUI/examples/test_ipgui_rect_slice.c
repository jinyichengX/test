/*
 * Standalone test with full statistics for ipgui_rect_slice.c
 *
 * Compile:
 *   gcc -O2 -Wall -Wextra -D_WIN32 \
 *       -I Include -I core/misc -I core/composite -I . -I al/hal \
 *       -o examples/test_ipgui_rect_slice.exe \
 *       examples/test_ipgui_rect_slice.c al/hal/ipgui_rect_slice.c
 *
 * Run:
 *   examples\test_ipgui_rect_slice.exe > examples\new.txt 2>&1
 */

#include <stdio.h>
#include <string.h>
#include "ipgui_rect_slice.h"

/* ================================================================
 * slice record: stores bounding-box of a single slice
 * ================================================================ */
typedef struct {
    int idx;              /* 1-based slice index within this test case */
    ipgui_coord_t x0, y0; /* top-left     */
    ipgui_coord_t x1, y1; /* bottom-right */
    ipgui_coord_t w, h;   /* width, height */
    int area;             /* pixel area */
} ipgui_slice_record_t;

#define MAX_SLICES_PER_CASE 20000

/* ================================================================
 * per-test-case statistics
 * ================================================================ */
typedef struct {
    char   id[64];                    /* unique test case identifier */
    ipgui_rect_t  src_rect;           /* original rectangle */
    ipgui_coord_t src_w, src_h;       /* original dimensions  */
    int           src_area;            /* original pixel area  */
    ipgui_coord_t slice_len;          /* slice length parameter */

    int           total_slices;       /* total slice call count */
    int           covered_area;       /* sum of all slice areas */
    int           optimal_lower;      /* theoretical minimum calls */
    int           slack;              /* total_slices - optimal_lower */

    ipgui_coord_t max_slice_w, max_slice_h; /* max slice dimensions */
    ipgui_coord_t min_slice_w, min_slice_h; /* min slice dimensions */

    int           narrow_strips;      /* count of strips where w < slice_len */
    int           normal_strips;      /* count of strips where w == slice_len */

    int           num_records;        /* how many records stored */
    ipgui_slice_record_t records[MAX_SLICES_PER_CASE];
} ipgui_slice_stats_t;

/* ================================================================
 * shared accumulator for all test cases
 * ================================================================ */
#define MAX_TEST_CASES 200
static ipgui_slice_stats_t g_stats[MAX_TEST_CASES];
static int g_stats_count = 0;

/* ================================================================
 * statistics collector: slice one rectangle completely while
 * recording every slice's bounding box
 * ================================================================ */
static int stats_collect(ipgui_slice_stats_t * st,
                          ipgui_rect_t rect, ipgui_coord_t slice_len,
                          int do_print)
{
    ipgui_rect_slice_ctx ctx;
    ipgui_rect_t slice;
    int pass = 1;

    memset(st, 0, sizeof(*st));

    /* copy source info */
    st->src_rect   = rect;
    st->src_w      = rect.end.x - rect.start.x + 1;
    st->src_h      = rect.end.y - rect.start.y + 1;
    st->src_area   = st->src_w * st->src_h;
    st->slice_len  = slice_len;
    st->optimal_lower = (st->src_area + slice_len - 1) / slice_len;

    st->max_slice_w = 0; st->max_slice_h = 0;
    st->min_slice_w = 999999; st->min_slice_h = 999999;

    /* build ID string */
    sprintf(st->id, "id=%02d  rect(%d,%d)-(%d,%d) %dx%d  sl=%d",
            g_stats_count + 1,
            rect.start.x, rect.start.y, rect.end.x, rect.end.y,
            st->src_w, st->src_h, slice_len);

    ipgui_rect_slice_ctx_init(&ctx, &rect, slice_len);

    if (do_print) {
        printf("----------------------------------------\n");
        printf("CASE %s\n", st->id);
        printf("  Source:  (%d,%d)-(%d,%d)  %dx%d  area=%d\n",
               rect.start.x, rect.start.y, rect.end.x, rect.end.y,
               st->src_w, st->src_h, st->src_area);
        printf("  slice_len=%d  optimal-lower-bound=%d\n",
               slice_len, st->optimal_lower);
    }

    while (ipgui_get_rect_slice(&ctx, &slice)) {
        ipgui_coord_t w = slice.end.x - slice.start.x + 1;
        ipgui_coord_t h = slice.end.y - slice.start.y + 1;
        int area = w * h;

        /* store record */
        if (st->num_records < MAX_SLICES_PER_CASE) {
            ipgui_slice_record_t *r = &st->records[st->num_records];
            r->idx  = st->num_records + 1;
            r->x0   = slice.start.x;
            r->y0   = slice.start.y;
            r->x1   = slice.end.x;
            r->y1   = slice.end.y;
            r->w    = w;
            r->h    = h;
            r->area = area;
        }

        st->covered_area += area;
        st->total_slices++;
        st->num_records++;

        /* track size extents */
        if (w > st->max_slice_w) st->max_slice_w = w;
        if (h > st->max_slice_h) st->max_slice_h = h;
        if (w < st->min_slice_w) st->min_slice_w = w;
        if (h < st->min_slice_h) st->min_slice_h = h;

        /* classify strip type */
        if (w < slice_len) st->narrow_strips++;
        else               st->normal_strips++;

        /* validation */
        if (slice.start.x < rect.start.x || slice.end.x > rect.end.x ||
            slice.start.y < rect.start.y || slice.end.y > rect.end.y) {
            printf("  *** ERROR: slice out of bounds! ***\n"); pass = 0;
        }
        if (w <= 0 || h <= 0) {
            printf("  *** ERROR: invalid slice dimensions! ***\n"); pass = 0;
        }

        /* detailed slice output */
        if (do_print) {
            printf("  slice #%4d: (%4d,%4d)-(%4d,%4d)  w=%4d h=%4d  area=%6d  type=%s\n",
                   st->total_slices, slice.start.x, slice.start.y,
                   slice.end.x, slice.end.y, w, h, area,
                   (w < slice_len) ? "NARROW" : "NORMAL");
        }
    }

    st->slack = st->total_slices - st->optimal_lower;

    if (st->covered_area != st->src_area) {
        printf("  *** ERROR: area mismatch! got=%d expected=%d ***\n",
               st->covered_area, st->src_area);
        pass = 0;
    }

    if (do_print) {
        printf("  TOTALS: slices=%d  optimal=%d  slack=+%d  covered=%d/%d  norm=%d  narrow=%d  [%s]\n\n",
               st->total_slices, st->optimal_lower, st->slack,
               st->covered_area, st->src_area,
               st->normal_strips, st->narrow_strips,
               pass ? "PASS" : "FAIL");
    }

    return pass;
}

/* helper: run collector and store in global array */
static int collect_and_store(ipgui_rect_t rect, ipgui_coord_t sl, int do_print)
{
    if (g_stats_count >= MAX_TEST_CASES) return 1;
    int pass = stats_collect(&g_stats[g_stats_count], rect, sl, do_print);
    g_stats_count++;
    return pass;
}

/* ================================================================
 * final statistics report: table format
 * ================================================================ */
static void print_stats_report(void)
{
    int i;
    int grand_slices = 0, grand_cases = g_stats_count;

    printf("\n");
    printf("==========================================================================================================\n");
    printf("                              SLICE STATISTICS REPORT\n");
    printf("==========================================================================================================\n\n");

    /* ---- summary table ---- */
    printf("+-----+------------------------------+----------+--------+--------+--------+--------+---------+\n");
    printf("| No  | Test Case                    | Src Size | sl_len | Slices | OptMin | Slack  |  Status |\n");
    printf("+-----+------------------------------+----------+--------+--------+--------+--------+---------+\n");

    for (i = 0; i < g_stats_count; i++) {
        ipgui_slice_stats_t *st = &g_stats[i];
        grand_slices += st->total_slices;
        printf("| %3d | (%3d,%3d)-(%3d,%3d) %4dx%-4d | %6d | %6d | %6d | %+6d | %s |\n",
               i + 1,
               st->src_rect.start.x, st->src_rect.start.y,
               st->src_rect.end.x, st->src_rect.end.y,
               st->src_w, st->src_h,
               st->slice_len,
               st->total_slices,
               st->optimal_lower,
               st->slack,
               (st->covered_area == st->src_area && st->total_slices >= st->optimal_lower) ? "PASS   " : "*FAIL*");
    }
    printf("+-----+------------------------------+----------+--------+--------+--------+--------+---------+\n");
    printf("|                                     GRAND TOTALS                     | %6d | %6s | %6s |         |\n",
           grand_slices, "---", "---");
    printf("+-----+------------------------------+----------+--------+--------+--------+--------+---------+\n\n");

    /* ---- per-case bounding-box detail ---- */
    printf("==========================================================================================================\n");
    printf("           BOUNDING-BOX DETAILS  (first 8 / last 2 slices per case shown)\n");
    printf("==========================================================================================================\n");

    for (i = 0; i < g_stats_count; i++) {
        ipgui_slice_stats_t *st = &g_stats[i];
        int n = st->num_records;

        printf("----------------------------------------------------------------------------------------------------------\n");
        printf("[Case %02d]  %s\n", i + 1, st->id);
        printf("  BBox extent:  min(%d,%d)  max(%d,%d)\n",
               st->min_slice_w, st->min_slice_h, st->max_slice_w, st->max_slice_h);
        printf("  Strip types:  normal=%d  narrow=%d\n", st->normal_strips, st->narrow_strips);

        if (n <= 10) {
            printf("  All %d slices:\n", n);
            for (int j = 0; j < n; j++) {
                ipgui_slice_record_t *r = &st->records[j];
                printf("    #%4d: (%4d,%4d)-(%4d,%4d)  %4dx%-4d  area=%6d  %s\n",
                       r->idx, r->x0, r->y0, r->x1, r->y1, r->w, r->h, r->area,
                       (r->w < st->slice_len) ? "[NARROW]" : "");
            }
        } else {
            printf("  First 8 slices:\n");
            for (int j = 0; j < 8 && j < n; j++) {
                ipgui_slice_record_t *r = &st->records[j];
                printf("    #%4d: (%4d,%4d)-(%4d,%4d)  %4dx%-4d  area=%6d  %s\n",
                       r->idx, r->x0, r->y0, r->x1, r->y1, r->w, r->h, r->area,
                       (r->w < st->slice_len) ? "[NARROW]" : "");
            }
            printf("    ... (%d slices omitted) ...\n", n - 10);
            printf("  Last 2 slices:\n");
            for (int j = n - 2; j < n; j++) {
                if (j < 0) continue;
                ipgui_slice_record_t *r = &st->records[j];
                printf("    #%4d: (%4d,%4d)-(%4d,%4d)  %4dx%-4d  area=%6d  %s\n",
                       r->idx, r->x0, r->y0, r->x1, r->y1, r->w, r->h, r->area,
                       (r->w < st->slice_len) ? "[NARROW]" : "");
            }
        }
        printf("\n");
    }

    /* ---- strip-type distribution chart ---- */
    printf("==========================================================================================================\n");
    printf("           STRIP TYPE DISTRIBUTION (normal vs narrow)\n");
    printf("==========================================================================================================\n\n");
    printf("  %-5s  %-35s  %8s  %8s  %8s  %s\n",
           "Case", "Rect", "Normal", "Narrow", "Total", "Narrow%");
    printf("  %-5s  %-35s  %8s  %8s  %8s  %s\n",
           "----", "----", "------", "------", "-----", "-------");

    for (i = 0; i < g_stats_count; i++) {
        ipgui_slice_stats_t *st = &g_stats[i];
        double pct = (st->total_slices > 0)
            ? 100.0 * (double)st->narrow_strips / (double)st->total_slices
            : 0.0;
        char label[36];
        sprintf(label, "(%d,%d)-(%d,%d) %dx%d",
                st->src_rect.start.x, st->src_rect.start.y,
                st->src_rect.end.x, st->src_rect.end.y,
                st->src_w, st->src_h);
        printf("  %-5d  %-35s  %8d  %8d  %8d  %6.1f%%\n",
               i + 1, label, st->normal_strips, st->narrow_strips,
               st->total_slices, pct);
    }

    printf("\n  Grand total: %d test cases, %d total slice operations\n",
           grand_cases, grand_slices);
    printf("==========================================================================================================\n");
}

/* ================================================================
 * edge case tests  (collects stats, prints details)
 * ================================================================ */
static int test_edge_cases(void)
{
    printf("========== EDGE CASE TESTS ==========\n\n");

    /* ID=t01 */ collect_and_store((ipgui_rect_t){{0,0},{0,0}},     1,   1);
    /* ID=t02 */ collect_and_store((ipgui_rect_t){{0,0},{0,0}},   100,   1);
    /* ID=t03 */ collect_and_store((ipgui_rect_t){{0,0},{99,99}},  100,   0);
    /* ID=t04 */ collect_and_store((ipgui_rect_t){{0,0},{9,9}},      1,   0);
    /* ID=t05 */ collect_and_store((ipgui_rect_t){{0,0},{49,49}},  200,   0);
    /* ID=t06 */ collect_and_store((ipgui_rect_t){{10,20},{109,119}},99,   0);
    /* ID=t07 */ collect_and_store((ipgui_rect_t){{0,0},{199,9}},   33,   0);
    /* ID=t08 */ collect_and_store((ipgui_rect_t){{0,0},{9,199}},   33,   0);

    return 1;
}

/* ================================================================
 * matrix test: 8 rects x 17 slice_lens  (quiet)
 * ================================================================ */
static int test_matrix(void)
{
    ipgui_rect_t rects[] = {
        {{0,0},{99,99}}, {{0,0},{49,99}}, {{0,0},{199,9}}, {{10,20},{109,119}},
        {{0,0},{0,0}},   {{0,0},{99,0}},  {{0,0},{0,99}},  {{5,5},{24,24}},
    };
    ipgui_coord_t slice_lens[] = {
        1,2,3,5,7,10,20,33,50,99,100,200,500,1000,5000,10000,20000
    };
    int n_rect = sizeof(rects)/sizeof(rects[0]);
    int n_sl   = sizeof(slice_lens)/sizeof(slice_lens[0]);
    int pass = 0, fail = 0;

    printf("========== MATRIX TEST (%d rects x %d sl) ==========\n\n", n_rect, n_sl);

    for (int r = 0; r < n_rect; r++)
        for (int s = 0; s < n_sl; s++)
            if (collect_and_store(rects[r], slice_lens[s], 0))
                pass++;
            else
                fail++;

    printf("Matrix: %d/%d passed, %d failed\n\n", pass, pass+fail, fail);
    return (fail == 0);
}

/* ================================================================
 * sequential reuse test
 * ================================================================ */
static int test_sequential(void)
{
    ipgui_rect_slice_ctx ctx;
    ipgui_rect_t slice;
    int ok = 1;

    printf("========== SEQUENTIAL REUSE TEST ==========\n\n");

    ipgui_rect_t r = {{0,0},{49,49}};
    int c1=0,a1=0; ipgui_rect_slice_ctx_init(&ctx,&r,10);
    while(ipgui_get_rect_slice(&ctx,&slice)){a1+=(slice.end.x-slice.start.x+1)*(slice.end.y-slice.start.y+1);c1++;}
    int c2=0,a2=0; ipgui_rect_slice_ctx_init(&ctx,&r,10);
    while(ipgui_get_rect_slice(&ctx,&slice)){a2+=(slice.end.x-slice.start.x+1)*(slice.end.y-slice.start.y+1);c2++;}
    if(c1!=c2||a1!=a2||a1!=2500){printf("  FAIL re-init\n");ok=0;}
    else printf("  PASS re-init: %d calls, area=%d\n",c1,a1);

    /* different rects on same ctx */
    printf("  Different rects on same ctx... ");
    int diff_ok = 1;
    ipgui_rect_t rr1={{0,0},{9,9}}, rr2={{100,0},{199,49}};
    ipgui_rect_slice_ctx_init(&ctx,&rr1,7);
    int px=0; while(ipgui_get_rect_slice(&ctx,&slice)) px+=(slice.end.x-slice.start.x+1)*(slice.end.y-slice.start.y+1);
    if(px!=100) diff_ok=0;
    ipgui_rect_slice_ctx_init(&ctx,&rr2,30);
    px=0; while(ipgui_get_rect_slice(&ctx,&slice)) px+=(slice.end.x-slice.start.x+1)*(slice.end.y-slice.start.y+1);
    if(px!=5000) diff_ok=0;
    printf("%s\n", diff_ok?"PASS":"FAIL");
    ok = ok && diff_ok;

    printf("\n");
    return ok;
}

/* ================================================================
 * main
 * ================================================================ */
int main(void)
{
    int total = 0, passed = 0;
    int r;

    printf("IPGUI Rect Slice — Full Statistics Test Suite\n");
    printf("===============================================\n");
    printf("(output also written to examples/new.txt)\n\n");

    r = test_edge_cases();  total++; if(r) passed++;
    r = test_matrix();       total++; if(r) passed++;
    r = test_sequential();   total++; if(r) passed++;

    /* ---- print the comprehensive statistics report ---- */
    printf("\n");
    print_stats_report();

    printf("\n=========================================\n");
    printf("SUMMARY: %d/%d test groups passed\n", passed, total);
    printf("=========================================\n");

    return (passed == total) ? 0 : 1;
}
