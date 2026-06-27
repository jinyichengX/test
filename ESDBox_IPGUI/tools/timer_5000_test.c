/*
 * IPGUI Timer Wheel — 5000+ Test Harness
 *
 * Self-contained C test that exactly mirrors ipgui_timer.c algorithm.
 * Tests multiple macro configurations, all timer types, wrap-around,
 * concurrent modifications, and corner cases.
 *
 * Compile:  gcc -o timer_test timer_5000_test.c && ./timer_test
 *           or any C compiler
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* ── type aliases ── */
typedef uint32_t u32_t;
typedef uint8_t  u8_t;
typedef int32_t  s32_t;
typedef uint64_t u64_t;

/* ── minimal doubly-linked list (exact match for elNET list) ── */
struct list_head {
    struct list_head *next, *prev;
};

typedef struct list_head ipgui_node_t;
typedef struct list_head ipgui_list_t;

#define LIST_POISON1 ((void*)0x00100100)
#define LIST_POISON2 ((void*)0x00200200)

static void list_init(struct list_head *list) {
    list->next = list;
    list->prev = list;
}

static void __list_add(struct list_head *n, struct list_head *prev, struct list_head *next) {
    next->prev = n;
    n->next = next;
    n->prev = prev;
    prev->next = n;
}

static void list_add(struct list_head *n, struct list_head *head) {
    __list_add(n, head, head->next);
}

static void __list_del(struct list_head *prev, struct list_head *next) {
    next->prev = prev;
    prev->next = next;
}

static void list_del(struct list_head *entry) {
    __list_del(entry->prev, entry->next);
    entry->next = (struct list_head*)LIST_POISON1;
    entry->prev = (struct list_head*)LIST_POISON2;
}

#define list_for_each_safe(pos, n, head) \
    for (pos = (head)->next, n = pos->next; pos != (head); pos = n, n = pos->next)

#define offsetof_(type, member) ((uintptr_t)&((type*)0)->member)
#define container_of(ptr, type, member) (type*)((char*)(ptr) - offsetof_(type, member))

/* ── configurable timer parameters (changed per test run) ── */

#define NEAR(m)      (1U << (m).near_shift)
#define LEVEL(m)     (1U << (m).level_shift)
#define NEAR_MASK(m) (NEAR(m) - 1)
#define LEVEL_MASK(m)(LEVEL(m) - 1)
#define TOTAL(m)     ((u64_t)NEAR(m) << ((u64_t)(m).level_shift * (m).level_num))

/* ── timer structure ── */
typedef void (*timer_cb_t)(void *tmr, void *ud);

typedef struct {
    ipgui_node_t  node;
    u32_t         period;
    u32_t         rounds;
    u32_t         expire;
    timer_cb_t    callback;
    void         *userdata;
    u8_t          active;
} ipg_tmr_t;

/* ── manager ── */
#define MAX_NEAR  256
#define MAX_LEVEL 64
#define MAX_LAYERS 8

typedef struct {
    int            near_shift;
    int            level_shift;
    int            level_num;
    ipgui_list_t   near[MAX_NEAR];
    ipgui_list_t   wheel[MAX_LAYERS][MAX_LEVEL];
    u32_t          time;
    u32_t          fire_count;
} mgr_t;

/* ── internal functions (exact replica of ipgui_timer.c) ── */
static void init_mgr(mgr_t *mgr, int ns, int ls, int ln) {
    mgr->near_shift  = ns;
    mgr->level_shift = ls;
    mgr->level_num   = ln;
    mgr->time        = 0;
    mgr->fire_count  = 0;
    int i, j;
    for (i = 0; i < NEAR(*mgr); i++)        list_init(&mgr->near[i]);
    for (i = 0; i < mgr->level_num; i++)
        for (j = 0; j < LEVEL(*mgr); j++)   list_init(&mgr->wheel[i][j]);
}

static void add_node(mgr_t *mgr, ipg_tmr_t *tmr) {
    u32_t expire = tmr->expire;
    u32_t current_time = mgr->time;
    u32_t diff = expire - current_time;

    if (diff < NEAR(*mgr)) {
        list_add(&tmr->node, &mgr->near[expire & NEAR_MASK(*mgr)]);
        return;
    }

    u32_t range = NEAR(*mgr);
    int i;
    for (i = 0; i < mgr->level_num - 1; i++) {
        range <<= mgr->level_shift;
        if (diff < range) break;
    }

    u32_t idx = (expire >> (mgr->near_shift + i * mgr->level_shift)) & LEVEL_MASK(*mgr);
    list_add(&tmr->node, &mgr->wheel[i][idx]);
}

static void move_list(mgr_t *mgr, int level, u32_t idx) {
    struct list_head *pos, *n;
    list_for_each_safe(pos, n, &mgr->wheel[level][idx]) {
        ipg_tmr_t *tmr = container_of(pos, ipg_tmr_t, node);
        list_del(pos);
        add_node(mgr, tmr);
    }
}

static void timer_shift(mgr_t *mgr) {
    u32_t mask = NEAR(*mgr);
    u32_t ct = ++mgr->time;

    if (ct == 0) {
        move_list(mgr, mgr->level_num - 1, 0);
        return;
    }

    u32_t time = ct >> mgr->near_shift;
    int i = 0;

    while ((ct & (mask - 1)) == 0 && i < mgr->level_num) {
        u32_t idx = time & LEVEL_MASK(*mgr);
        move_list(mgr, i, idx);
        if (idx != 0) {
            break;
        }
        mask <<= mgr->level_shift;
        time >>= mgr->level_shift;
        i++;
    }
}

static void timer_execute(mgr_t *mgr, int is_first) {
    u32_t idx = mgr->time & NEAR_MASK(*mgr);
    struct list_head *pos, *n;

    list_for_each_safe(pos, n, &mgr->near[idx]) {
        ipg_tmr_t *tmr = container_of(pos, ipg_tmr_t, node);

        list_del(pos);
        tmr->active = 0;

        if (tmr->callback) {
            tmr->callback(tmr, tmr->userdata);
        }
        mgr->fire_count++;

        if (tmr->period > 0 && !tmr->active) {
            if (tmr->rounds > 0) {
                tmr->rounds--;
                if (tmr->rounds == 0) continue;
            }
            tmr->expire = mgr->time + tmr->period;
            if (is_first && ((tmr->period & NEAR_MASK(*mgr)) == 1)) {
                tmr->expire++;
            }
            add_node(mgr, tmr);
            tmr->active = 1;
        }
    }
}

static void timer_tick(mgr_t *mgr) {
    timer_execute(mgr, 1);
    timer_shift(mgr);
    timer_execute(mgr, 0);
}

static void timer_init(ipg_tmr_t *tmr, u32_t period, u32_t rounds, timer_cb_t cb, void *ud) {
    list_init(&tmr->node);
    tmr->period   = period;
    tmr->rounds   = rounds;
    tmr->expire   = 0;
    tmr->callback = cb;
    tmr->userdata = ud;
    tmr->active   = 0;
}

static void timer_start(mgr_t *mgr, ipg_tmr_t *tmr, u32_t delay) {
    if (tmr->active) {
        list_del(&tmr->node);
        tmr->active = 0;
    }
    tmr->expire = mgr->time + delay;
    add_node(mgr, tmr);
    tmr->active = 1;
}

static void timer_stop(ipg_tmr_t *tmr) {
    if (!tmr->active) return;
    list_del(&tmr->node);
    tmr->active = 0;
}

/* ── test infrastructure ── */
static int  g_tests_total   = 0;
static int  g_tests_passed  = 0;
static int  g_tests_failed  = 0;

#define TEST_ASSERT(cond, msg) do { \
    g_tests_total++; \
    if (!(cond)) { \
        g_tests_failed++; \
        printf("  FAIL [%s] %s\n", __func__, msg); \
        return 0; \
    } else { \
        g_tests_passed++; \
    } \
} while(0)

/* ── test callbacks ── */
static u32_t g_cb_idx;
static u32_t g_cb_times[256];

static void cb_record_time(void *tmr, void *ud) {
    ipg_tmr_t *t = (ipg_tmr_t*)tmr;
    mgr_t *m = (mgr_t*)ud;
    g_cb_times[g_cb_idx++] = m->time;
    (void)t;
}

static void cb_nop(void *tmr, void *ud) {
    (void)tmr; (void)ud;
}

/* ── Test 1: basic periodic, no wrap ── */
static int test_periodic_basic(void) {
    mgr_t mgr;
    init_mgr(&mgr, 4, 3, 4);

    ipg_tmr_t tmr;
    timer_init(&tmr, 500, 0, cb_nop, NULL);
    timer_start(&mgr, &tmr, 0);

    for (u32_t t = 0; t < 10000; t++) timer_tick(&mgr);

    /* delay=0: fires at time 0, 500, 1000, ..., 10000 = 21 fires */
    TEST_ASSERT(mgr.fire_count == 21, "periodic 500 should fire 21 times in 10000 ticks");
    TEST_ASSERT(tmr.active == 1, "should still be active");
    return 1;
}

/* ── Test 2: one-shot timer ── */
static int test_oneshot(void) {
    mgr_t mgr;
    init_mgr(&mgr, 4, 3, 4);

    ipg_tmr_t tmr;
    timer_init(&tmr, 0, 1, cb_nop, NULL);
    timer_start(&mgr, &tmr, 1000);

    for (u32_t t = 0; t < 2000; t++) timer_tick(&mgr);

    TEST_ASSERT(mgr.fire_count == 1, "one-shot should fire exactly once");
    return 1;
}

/* ── Test 3: limited rounds ── */
static int test_rounds(void) {
    mgr_t mgr;
    init_mgr(&mgr, 4, 3, 4);

    ipg_tmr_t tmr;
    timer_init(&tmr, 100, 5, cb_nop, NULL);
    timer_start(&mgr, &tmr, 0);

    for (u32_t t = 0; t < 2000; t++) timer_tick(&mgr);

    TEST_ASSERT(mgr.fire_count == 5, "limited to 5 rounds");
    return 1;
}

/* ── Test 4: period=1 timer ── */
static int test_period_1(void) {
    mgr_t mgr;
    init_mgr(&mgr, 4, 3, 4);

    ipg_tmr_t tmr;
    timer_init(&tmr, 1, 0, cb_nop, NULL);
    timer_start(&mgr, &tmr, 0);

    for (u32_t t = 0; t < 2000; t++) timer_tick(&mgr);

    TEST_ASSERT(mgr.fire_count == 2000, "period=1 should fire every tick");
    return 1;
}

/* ── Test 5: delay=0 fires immediately ── */
static int test_delay_zero(void) {
    mgr_t mgr;
    init_mgr(&mgr, 4, 3, 4);

    ipg_tmr_t tmr;
    timer_init(&tmr, 0, 1, cb_nop, NULL);
    timer_start(&mgr, &tmr, 0);

    timer_tick(&mgr);
    TEST_ASSERT(mgr.fire_count == 1, "delay=0 fires immediately");
    return 1;
}

/* ── Test 6: stop mid-flight ── */
static int test_stop(void) {
    mgr_t mgr;
    init_mgr(&mgr, 4, 3, 4);

    ipg_tmr_t tmr;
    timer_init(&tmr, 500, 0, cb_nop, NULL);
    timer_start(&mgr, &tmr, 0);

    for (u32_t t = 0; t < 750; t++) timer_tick(&mgr);
    u32_t before = mgr.fire_count;
    timer_stop(&tmr);

    for (u32_t t = 0; t < 2000; t++) timer_tick(&mgr);

    TEST_ASSERT(mgr.fire_count == before, "fire count shouldn't increase after stop");
    return 1;
}

/* ── Test 7: restart ── */
static int test_restart(void) {
    mgr_t mgr;
    init_mgr(&mgr, 4, 3, 4);

    ipg_tmr_t tmr;
    timer_init(&tmr, 500, 0, cb_nop, NULL);
    timer_start(&mgr, &tmr, 0);

    for (u32_t t = 0; t < 100; t++) timer_tick(&mgr);
    timer_stop(&tmr);
    u32_t fires_before = mgr.fire_count;
    timer_start(&mgr, &tmr, 500);

    for (u32_t t = 0; t < 2000; t++) timer_tick(&mgr);

    TEST_ASSERT(mgr.fire_count > fires_before, "should fire after restart");
    return 1;
}

/* ── Test 8: concurrent timers ── */
static int test_concurrent(void) {
    mgr_t mgr;
    init_mgr(&mgr, 4, 3, 4);

    ipg_tmr_t tmrs[10];
    for (int i = 0; i < 10; i++) {
        timer_init(&tmrs[i], 100 + i * 10, 0, cb_nop, NULL);
        timer_start(&mgr, &tmrs[i], i * 5);
    }

    for (u32_t t = 0; t < 5000; t++) timer_tick(&mgr);

    TEST_ASSERT(mgr.fire_count > 100, "concurrent timers should all fire many times");
    return 1;
}

/* ── Test 9: wrap-around ── */
static int test_wraparound(void) {
    mgr_t mgr;
    init_mgr(&mgr, 4, 3, 4);

    /* start near u32 max */
    mgr.time = 0xFFFFFFF0;

    ipg_tmr_t tmr;
    timer_init(&tmr, 100, 0, cb_nop, NULL);
    timer_start(&mgr, &tmr, 100);
    TEST_ASSERT(tmr.active == 1, "timer should be active");

    /* tick through wrap */
    for (u32_t i = 0; i < 500; i++) timer_tick(&mgr);

    TEST_ASSERT(mgr.fire_count >= 4, "should fire after wrap (period 100, 500 ticks)");
    return 1;
}

/* ── Test 10: max delay ── */
static int test_max_delay(void) {
    mgr_t mgr;
    init_mgr(&mgr, 4, 3, 4);

    ipg_tmr_t tmr;
    timer_init(&tmr, 0, 1, cb_nop, NULL);
    timer_start(&mgr, &tmr, 65500);
    TEST_ASSERT(tmr.active == 1, "large delay accepted");

    for (u32_t t = 0; t < 70000; t++) timer_tick(&mgr);
    TEST_ASSERT(mgr.fire_count == 1, "timer with max delay should fire exactly once");
    return 1;
}

/* ── Test 11: cross-boundary expiration (the bug wang found) ── */
static int test_cross_boundary(void) {
    mgr_t mgr;
    init_mgr(&mgr, 4, 3, 4);

    /* simulate exactly the scenario that crashed before:
     * time ≈ 65500, autoreload with period=500 → expire crosses 65536 */
    mgr.time = 65500;

    ipg_tmr_t tmr;
    timer_init(&tmr, 500, 0, cb_nop, NULL);
    timer_start(&mgr, &tmr, 0);

    /* run through boundary */
    for (u32_t t = 0; t < 10000; t++) timer_tick(&mgr);

    TEST_ASSERT(mgr.fire_count > 15, "cross-boundary periodic should keep firing");
    return 1;
}

/* ── Test 12: callback stops another timer ── */
static ipg_tmr_t *g_tmr_a, *g_tmr_b;
static mgr_t      *g_mgr_12;

static void cb_stop_b(void *tmr, void *ud) {
    (void)tmr; (void)ud;
    timer_stop(g_tmr_b);
}

static int test_callback_stop_other(void) {
    mgr_t mgr;
    init_mgr(&mgr, 4, 3, 4);
    g_mgr_12 = &mgr;

    ipg_tmr_t tmr_a, tmr_b;
    g_tmr_a = &tmr_a;
    g_tmr_b = &tmr_b;

    timer_init(&tmr_a, 500, 0, cb_stop_b, NULL);
    timer_init(&tmr_b, 100, 0, cb_nop,  NULL);
    timer_start(&mgr, &tmr_a, 500);
    timer_start(&mgr, &tmr_b, 50);

    (void)g_tmr_a;
    for (u32_t t = 0; t < 2000; t++) timer_tick(&mgr);

    /* b should stop when a first fires at t=500 */
    TEST_ASSERT(!tmr_b.active, "timer B should be stopped by A's callback");
    TEST_ASSERT(tmr_a.active,  "timer A should keep running");
    return 1;
}

/* ── Test 13: callback restarts itself with different delay ── */
static ipg_tmr_t *g_tmr_13;
static mgr_t      *g_mgr_13;
static u32_t       g_restart_count;

static void cb_restart_self(void *tmr, void *ud) {
    (void)ud;
    g_restart_count++;
    timer_stop((ipg_tmr_t*)tmr);
    timer_start(g_mgr_13, (ipg_tmr_t*)tmr, 50);
}

static int test_callback_restart_self(void) {
    mgr_t mgr;
    init_mgr(&mgr, 4, 3, 4);
    g_mgr_13 = &mgr;
    g_restart_count = 0;

    ipg_tmr_t tmr;
    g_tmr_13 = &tmr;
    timer_init(&tmr, 0, 1, cb_restart_self, NULL);
    timer_start(&mgr, &tmr, 100);

    for (u32_t t = 0; t < 2000; t++) timer_tick(&mgr);

    TEST_ASSERT(g_restart_count >= 2, "callback should restart itself multiple times");
    return 1;
}

/* ── Test 14: various macro configurations ── */
static int test_config(int ns, int ls, int ln, const char *label, 
                       u32_t period, u32_t run_ticks, u32_t expected_min_fires) {
    mgr_t mgr;
    init_mgr(&mgr, ns, ls, ln);

    ipg_tmr_t tmr;
    timer_init(&tmr, period, 0, cb_nop, NULL);
    timer_start(&mgr, &tmr, 0);

    for (u32_t t = 0; t < run_ticks; t++) timer_tick(&mgr);

    TEST_ASSERT(mgr.fire_count >= expected_min_fires, label);
    return 1;
}

static int test_all_configs(void) {
    /* default */
    test_config(4, 3, 4, "cfg 4-3-4 p100",   100,  2000, 19);
    test_config(4, 3, 4, "cfg 4-3-4 p500",   500,  5000, 9);
    test_config(4, 3, 4, "cfg 4-3-4 delay",  0,    1,    1);

    /* small */
    test_config(3, 2, 3,  "cfg 3-2-3 p10",   10,   500,  49);
    test_config(3, 3, 3,  "cfg 3-3-3 p50",   50,   2000, 39);

    /* medium */
    test_config(5, 3, 4,  "cfg 5-3-4 p100",  100,  2000, 19);
    test_config(6, 3, 4,  "cfg 6-3-4 p200",  200,  2000, 9);

    /* large */
    test_config(8, 6, 4,  "cfg 8-6-4 p10",   10,   2000, 199);
    test_config(5, 4, 5,  "cfg 5-4-5 p100",  100,  2000, 19);

    /* level_num=1 */
    test_config(6, 3, 1,  "cfg 6-3-1 p5",    5,    500,  99);

    /* level_num=2 */
    test_config(5, 5, 2,  "cfg 5-5-2 p1",    1,    200,  199);

    return 1;
}

/* ── Test 15: precision - timers fire at exact expected times ── */
static int test_precision(void) {
    mgr_t mgr;
    init_mgr(&mgr, 4, 3, 4);
    g_cb_idx = 0;
    memset(g_cb_times, 0, sizeof(g_cb_times));

    ipg_tmr_t tmr;
    timer_init(&tmr, 100, 0, cb_record_time, &mgr);
    timer_start(&mgr, &tmr, 0);

    for (u32_t t = 0; t < 5100; t++) timer_tick(&mgr);

    /* Check first 10 fires: should be at 99,199,299,... (delay=0 → first fire at t=0) */
    /* Actually with delay=0, timer fires at time=0 (the first execute), then every 100 */
    /* mgr.time starts at 0, first tick: execute(0), shift(time→1), execute(1). */
    /* expire=0 for start, so fires at time=0 execute. Then expire=100, fires at time=100. */
    /* Fires at: 0, 100, 200, 300, ... */
    for (int i = 0; i < (int)g_cb_idx; i++) {
        char buf[64];
        snprintf(buf, sizeof(buf), "precision: fire %d at %u (expected %u)", 
                 i, g_cb_times[i], (u32_t)i * 100);
        TEST_ASSERT(g_cb_times[i] == (u32_t)i * 100, buf);
    }

    return 1;
}

/* ── Test 16: bulk random scenarios ── */
static int test_bulk_scenarios(void) {
    /* Generate 200+ systematic scenarios varying:
     * - near_shift: 3, 4, 5
     * - level_shift: 2, 3, 4
     * - level_num: 2, 3, 4
     * - period: 1, 7, 13, 50, 100, 500, 1025, 5000
     * - start_delay: 0, period/2, period, period*2
     * - rounds: 0(inf), 1, 3, 10
     * - start_time: 0, near mid, near max
     */

    int near_shifts[]  = {3, 4, 5, 6};
    int level_shifts[] = {2, 3, 4};
    int level_nums[]   = {2, 3, 4};
    u32_t periods[]    = {1, 7, 13, 50, 100, 500, 1025, 5000};
    u32_t delays[]     = {0, 1, 50, 200};
    u32_t rounds_arr[] = {0, 1, 3, 10};
    u32_t start_times[]= {0, 10000, 0xFFFF0000};

    int scenario = 0;
    for (int ns_i = 0; ns_i < 4; ns_i++) {
     for (int ls_i = 0; ls_i < 3; ls_i++) {
      for (int ln_i = 0; ln_i < 3; ln_i++) {
       int ns = near_shifts[ns_i];
       int ls = level_shifts[ls_i];
       int ln = level_nums[ln_i];

       /* skip invalid combos */
       if (ns + (ln - 1) * ls > 31) continue;

       for (int p_i = 0; p_i < 8; p_i++) {
        u32_t period = periods[p_i];
        for (int d_i = 0; d_i < 4; d_i++) {
         u32_t delay = delays[d_i];
         for (int r_i = 0; r_i < 4; r_i++) {
          u32_t rounds = rounds_arr[r_i];
          for (int st_i = 0; st_i < 3; st_i++) {

            mgr_t mgr;
            init_mgr(&mgr, ns, ls, ln);
            mgr.time = start_times[st_i];

            ipg_tmr_t tmr;
            timer_init(&tmr, period, rounds, cb_nop, NULL);
            timer_start(&mgr, &tmr, delay);

            u32_t expected = (rounds == 0) ? 0xFFFFFFFF : rounds;
            u32_t fires    = 0;
            u32_t tick_end = 200 + delay + period * expected;
            if (tick_end > 200000) tick_end = 200000; /* cap for very long periods */

            for (u32_t t = 0; t < tick_end; t++) {
                u32_t before = mgr.fire_count;
                timer_tick(&mgr);
                if (mgr.fire_count > before) fires++;
            }

            /* verify not stuck (fires within expected range) */
            if (rounds > 0) {
                char buf[80];
                snprintf(buf, sizeof(buf), 
                    "bulk #%d (%d-%d-%d p%u d%u r%u t%u) fires=%u expected=%u",
                    scenario, ns, ls, ln, period, delay, rounds, start_times[st_i],
                    fires, expected);
                TEST_ASSERT(fires == expected, buf);
            } else {
                char buf[80];
                snprintf(buf, sizeof(buf), 
                    "bulk #%d (%d-%d-%d p%u d%u r%u t%u) fires=%u (expect > 0)",
                    scenario, ns, ls, ln, period, delay, rounds, start_times[st_i],
                    fires);
                TEST_ASSERT(fires > 0, buf);
            }
            scenario++;
          }
         }
        }
       }
      }
     }
    }

    printf("  bulk scenarios executed: %d\n", scenario);
    return 1;
}

/* ── Test 17: many concurrent timers with different periods ── */
static int test_many_timers(void) {
    mgr_t mgr;
    init_mgr(&mgr, 4, 3, 4);

    #define N_TIMERS 50
    ipg_tmr_t tmrs[N_TIMERS];

    for (int i = 0; i < N_TIMERS; i++) {
        u32_t period = 10 + i * 13;
        timer_init(&tmrs[i], period, 0, cb_nop, NULL);
        timer_start(&mgr, &tmrs[i], i);
    }

    for (u32_t t = 0; t < 10000; t++) {
        timer_tick(&mgr);
    }

    TEST_ASSERT(mgr.fire_count > 500, "50 concurrent timers should fire many times");
    return 1;
}

/* ── Test 18: extreme period and delay values ── */
static int test_extreme_values(void) {
    mgr_t mgr;
    init_mgr(&mgr, 4, 3, 4);

    /* period = total coverage - 1 */
    u32_t total = (u32_t)((1ULL << 4) * (1ULL << (4*3)));
    ipg_tmr_t tmr;
    timer_init(&tmr, total - 1, 1, cb_nop, NULL);
    timer_start(&mgr, &tmr, total - 1);

    for (u32_t t = 0; t < total + 1000; t++) timer_tick(&mgr);

    TEST_ASSERT(mgr.fire_count >= 1, "near-max delay should fire");
    return 1;
}

/* ── Test 19: delay larger than total coverage ── */
static int test_exceed_coverage(void) {
    mgr_t mgr;
    init_mgr(&mgr, 4, 3, 4);

    ipg_tmr_t tmr;
    timer_init(&tmr, 0, 1, cb_nop, NULL);
    u32_t total = (u32_t)TOTAL(mgr);
    timer_start(&mgr, &tmr, total + 1000);

    /* should eventually fire after full wrap cycle */
    for (u32_t t = 0; t < total + 2000; t++) timer_tick(&mgr);

    TEST_ASSERT(mgr.fire_count == 1, "exceeding coverage should still fire (degraded accuracy)");
    return 1;
}

/* ── Test 20: multiple starts on same timer ── */
static int test_multiple_starts(void) {
    mgr_t mgr;
    init_mgr(&mgr, 4, 3, 4);

    ipg_tmr_t tmr;
    timer_init(&tmr, 0, 1, cb_nop, NULL);

    timer_start(&mgr, &tmr, 1000);
    /* restart before it fires */
    timer_start(&mgr, &tmr, 500);

    for (u32_t t = 0; t < 2000; t++) timer_tick(&mgr);

    TEST_ASSERT(mgr.fire_count == 1, "restart before fire should only fire once");
    return 1;
}

/* ── main ── */
int main(void) {
    printf("=== IPGUI Timer Wheel — 5000+ Test Suite ===\n\n");

    /* run all tests */
    test_periodic_basic();
    test_oneshot();
    test_rounds();
    test_period_1();
    test_delay_zero();
    test_stop();
    test_restart();
    test_concurrent();
    test_wraparound();
    test_max_delay();
    test_cross_boundary();
    test_callback_stop_other();
    test_callback_restart_self();
    test_all_configs();
    test_precision();
    test_bulk_scenarios();
    test_many_timers();
    test_extreme_values();
    test_exceed_coverage();
    test_multiple_starts();

    printf("\n===========================================\n");
    printf("  TOTAL:  %d\n", g_tests_total);
    printf("  PASSED: %d\n", g_tests_passed);
    printf("  FAILED: %d\n", g_tests_failed);
    printf("===========================================\n");

    if (g_tests_failed == 0) {
        printf("\n  ALL TESTS PASSED.\n");
        return 0;
    } else {
        printf("\n  %d TEST(S) FAILED!\n", g_tests_failed);
        return 1;
    }
}
