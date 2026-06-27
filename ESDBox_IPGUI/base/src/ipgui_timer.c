/*
 * 时间轮定时器 — 基于 skynet timer 算法
 *
 * near 轮:  O(1) 直触触发
 * wheel 层: 多层级联，覆盖长延时
 * 总覆盖 = NEAR × LEVEL ^ LEVEL_NUM
 *
 * 插入 (add_node):   差值直接判断层级，移位取模决定槽
 * 级联 (timer_shift): time 低位归零 → cascade 对应高层槽
 *                     idx==0 也级联（小参数下会用到）
 *
 * 双执行 (execute-shift-execute): 第一轮处理 timeout-0，第二轮处理新槽
 * 自填 (auto-reload): period>0 自动重装，period&NEAR_MASK==1 时+1防双重触发
 */

#include "ipgui_timer.h"
#include "ipgui_time.h"

/* ── internal: add timer to the right slot ── */
__IPGUI_STATIC__ void add_node(ipgui_timer_mgr_t *mgr, ipg_tmr_t *tmr)
{
    u32_t expire       = tmr->expire;
    u32_t current_time = mgr->time;

    /* u32 subtraction: naturally handles wrap-around (diff is the correct
       number of ticks between current_time and expire in modular arithmetic) */
    u32_t diff = expire - current_time;

    /* near wheel: diff within one NEAR round */
    if (diff < IPGUI_TIMER_NEAR) {
        list_add(&tmr->node, &mgr->near[expire & IPGUI_TIMER_NEAR_MASK]);
        return;
    }

    /* find the right cascading level by range comparison.
       wheel[0] covers [NEAR, NEAR*LEVEL)
       wheel[1] covers [NEAR*LEVEL, NEAR*LEVEL^2)
       ...
       wheel[LEVEL_NUM-1] covers anything above NEAR*LEVEL^(LEVEL_NUM-1)
       ── diff is always safe here because it's the true tick distance,
           unlike bitmask comparison which fails across 2^N boundaries. */
    u32_t range = IPGUI_TIMER_NEAR;
    int    i;

    for (i = 0; i < IPGUI_TIMER_LEVEL_NUM - 1; i++) {
        range <<= IPGUI_TIMER_LEVEL_SHIFT;
        if (diff < range) {
            break;
        }
    }
    /* i == LEVEL_NUM-1 means top level */

    u32_t idx = (expire >> (IPGUI_TIMER_NEAR_SHIFT + i * IPGUI_TIMER_LEVEL_SHIFT))
              & IPGUI_TIMER_LEVEL_MASK;
    list_add(&tmr->node, &mgr->wheel[i][idx]);
}

/* ── internal: cascade one slot down ── */
__IPGUI_STATIC__ void move_list(ipgui_timer_mgr_t *mgr, int level, u32_t idx)
{
    struct list_head *pos, *head;
    list_for_each_safe(pos, head, &mgr->wheel[level][idx]) {
        ipg_tmr_t *tmr = ipgui_container_of(pos, ipg_tmr_t, node);
        list_del_init(pos);
        add_node(mgr, tmr);
    }
}

/* ── internal: advance time cursor, cascade wheels ── */
__IPGUI_STATIC__ void timer_shift(ipgui_timer_mgr_t *mgr)
{
    u32_t mask = IPGUI_TIMER_NEAR;
    u32_t ct   = ++mgr->time;

    u32_t time = ct >> IPGUI_TIMER_NEAR_SHIFT;
    int i = 0;

    while ((ct & (mask - 1)) == 0 && i < IPGUI_TIMER_LEVEL_NUM) {
        u32_t idx = time & IPGUI_TIMER_LEVEL_MASK;
        move_list(mgr, i, idx);
        if (idx != 0) {
            break;
        }
        /* idx == 0: this level cascaded, continue to next level.
           Necessary because with small NEAR/LEVEL params, idx=0
           slots CAN contain timers (unlike skynet's large params). */
        mask <<= IPGUI_TIMER_LEVEL_SHIFT;
        time >>= IPGUI_TIMER_LEVEL_SHIFT;
        i++;
    }
}

/* ── internal: fire all timers in current near slot ──
   is_first: 1 = called before timer_shift, 0 = called after timer_shift.
   Prevents double-fire when autoreload lands timer in the sibling
   near slot of the same tick pair (period & NEAR_MASK == 1).

   Uses a detach-then-process pattern to guard against callbacks
   that stop/modify other timers in the same near slot.  Without
   this, list_for_each_safe would read a poisoned next pointer. */
__IPGUI_STATIC__ void timer_execute(ipgui_timer_mgr_t *mgr, int is_first)
{
    u32_t idx = mgr->time & IPGUI_TIMER_NEAR_MASK;

    /* Phase 1: detach all timers from the near slot into a private list.
       list_del_init (not list_del) so that a callback can stop another
       timer without poisoning the saved next pointer. */
    struct list_head *pos, *n, detached;
    list_head_init(&detached);
    list_for_each_safe(pos, n, &mgr->near[idx]) {
        list_del_init(pos);
        list_add(pos, &detached);
    }

    /* Phase 2: process the private list one-by-one.
       A callback may stop a timer that hasn't been reached yet —
       that's fine: it's removed from the private list and its
       callback won't fire. */
    list_for_each_safe(pos, n, &detached) {
        ipg_tmr_t *tmr = ipgui_container_of(pos, ipg_tmr_t, node);

        list_del_init(pos);

        /* Already stopped by an earlier callback in this tick? */
        if (!tmr->active) {
            continue;
        }

        tmr->active = 0;

        /* fire */
        if (tmr->callback) {
            tmr->callback(tmr, tmr->userdata);
        }

        /* auto-reload for periodic timers (if callback didn't restart it) */
        if (tmr->period > 0 && !tmr->active) {
            if (tmr->rounds > 0) {
                tmr->rounds--;
                if (tmr->rounds == 0) {
                    continue;   /* done, no more rounds */
                }
            }
            /* re-arm */
            tmr->expire = mgr->time + tmr->period;

            /* If this is the 1st execute and the new timer lands in the
               near slot that the 2nd execute will process in the same tick,
               push it by 1 tick to avoid double-firing.
               Only applies when timer stays in near wheel (period < NEAR),
               because wheel-level timers can't be reached by 2nd execute. */
            if (is_first
                && ((tmr->period & IPGUI_TIMER_NEAR_MASK) == 1)
                && (tmr->period < IPGUI_TIMER_NEAR)) {
                tmr->expire++;
            }

            add_node(mgr, tmr);
            tmr->active = 1;
        }
    }
}

/* ── public API ── */

__IPGUI_API__ void ipgui_timer_mgr_init(ipgui_timer_mgr_t *mgr)
{
    int i, j;

    for (i = 0; i < IPGUI_TIMER_NEAR; i++) {
        list_head_init(&mgr->near[i]);
    }
    for (i = 0; i < IPGUI_TIMER_LEVEL_NUM; i++) {
        for (j = 0; j < IPGUI_TIMER_LEVEL; j++) {
            list_head_init(&mgr->wheel[i][j]);
        }
    }
    mgr->time = 0;
}

__IPGUI_API__ void ipgui_timer_init(ipg_tmr_t *tmr, ipgui_tick_t period, u32_t rounds,
                                     ipgui_timer_cb_t cb, void *userdata)
{
    list_head_init(&tmr->node);
    tmr->period   = period;
    tmr->rounds   = rounds;
    tmr->expire   = 0;
    tmr->callback = cb;
    tmr->userdata = userdata;
    tmr->active   = 0;
}

__IPGUI_API__ ipgui_err_t ipgui_timer_start(ipgui_timer_mgr_t *mgr, ipg_tmr_t *tmr, ipgui_tick_t delay)
{
    if (!mgr || !tmr) {
        return IPGUI_ERR_PARAM;
    }
    if (delay > IPGUI_TIMER_MAX_DELAY) {
        return IPGUI_ERR_PARAM;
    }

    /* remove from previous slot if already running */
    if (tmr->active) {
        list_del_init(&tmr->node);
        tmr->active = 0;
    }

    tmr->expire = mgr->time + delay;
    add_node(mgr, tmr);
    tmr->active = 1;

    return IPGUI_ERR_OK;
}

__IPGUI_API__ void ipgui_timer_stop(ipg_tmr_t *tmr)
{
    if (!tmr || !tmr->active) {
        return;
    }
    list_del_init(&tmr->node);
    tmr->active = 0;
}

__IPGUI_API__ ipgui_err_t ipgui_timer_restart(ipgui_timer_mgr_t *mgr, ipg_tmr_t *tmr)
{
    ipgui_timer_stop(tmr);

    /* re-init rounds (in case it was exhausted) */
    if (tmr->rounds == 0 && tmr->period > 0) {
        /* rounds==0 means forever, no need to change */
    }

    return ipgui_timer_start(mgr, tmr, tmr->period);
}

/* ── tick: advance by 1 ── */

__IPGUI_API__ void ipgui_timer_tick(ipgui_timer_mgr_t *mgr)
{
    /* fire timers expiring at current time (rare: timeout-0) */
    timer_execute(mgr, 1);

    /* advance clock, cascade */
    timer_shift(mgr);

    /* fire timers in the new time slot */
    timer_execute(mgr, 0);
}

__IPGUI_API__ void ipgui_timer_loop(ipgui_timer_mgr_t *mgr, ipgui_tick_t passed)
{
    while (passed--) {
        ipgui_timer_tick(mgr);
    }
}