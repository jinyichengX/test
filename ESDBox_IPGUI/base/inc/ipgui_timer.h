#ifndef ipgui_timer_h
#define ipgui_timer_h

#include "ipgui_utils.h"
#include "ipgui_list.h"

/* ── timer wheel geometry ──
   NEAR:  near-range wheel,  handles expiry within (1 << NEAR_SHIFT) ticks
   LEVEL: cascading wheels,  each level covers (1 << LEVEL_SHIFT) slots
   Total coverage: (1<<NEAR_SHIFT) * (1<<LEVEL_SHIFT)^LEVEL_NUM  ticks

   Default: 16 * 8^4 = 65536 ticks.  At 1ms/tick = ~65 seconds max delay.
   Override these macros before #include to tune memory/range tradeoff.
   ───────────────────────────────────────────────────────────── */
#ifndef IPGUI_TIMER_NEAR_SHIFT
#define IPGUI_TIMER_NEAR_SHIFT   4
#endif
#ifndef IPGUI_TIMER_LEVEL_SHIFT
#define IPGUI_TIMER_LEVEL_SHIFT  3
#endif
#ifndef IPGUI_TIMER_LEVEL_NUM
#define IPGUI_TIMER_LEVEL_NUM    4
#endif

#define IPGUI_TIMER_NEAR        (1U << IPGUI_TIMER_NEAR_SHIFT)
#define IPGUI_TIMER_LEVEL       (1U << IPGUI_TIMER_LEVEL_SHIFT)
#define IPGUI_TIMER_NEAR_MASK   (IPGUI_TIMER_NEAR  - 1)
#define IPGUI_TIMER_LEVEL_MASK  (IPGUI_TIMER_LEVEL - 1)
/* Maximum legal delay: must fit within total wheel coverage */
#define IPGUI_TIMER_MAX_DELAY   ((ipgui_tick_t)(IPGUI_TIMER_NEAR) * ((ipgui_tick_t)1 << (IPGUI_TIMER_LEVEL_SHIFT * IPGUI_TIMER_LEVEL_NUM)) - 1)

/* ── types ── */
typedef struct ipgui_timer_st        ipg_tmr_t;
typedef struct ipgui_timer_mgr_st    ipgui_timer_mgr_t;
typedef void (* ipgui_timer_cb_t)(ipg_tmr_t *tmr, void *userdata);

struct ipgui_timer_st {
    ipgui_node_t      node;         /* 链表节点 */
    ipgui_tick_t      period;       /* 周期, 0 = 单次 */
    u32_t             rounds;       /* 剩余轮数, 0 = 无限 */
    ipgui_tick_t      expire;       /* 绝对到期时刻 */
    ipgui_timer_cb_t  callback;     /* 到期回调 */
    void             *userdata;     /* 回调参数 */
    u8_t              active;       /* 是否在时间轮中 */
};

struct ipgui_timer_mgr_st {
    ipgui_list_t      near  [IPGUI_TIMER_NEAR];
    ipgui_list_t      wheel [IPGUI_TIMER_LEVEL_NUM][IPGUI_TIMER_LEVEL];
    ipgui_tick_t      time;         /* 当前时刻游标 */
};

/* ── API ── */
__IPGUI_API__ void ipgui_timer_mgr_init (ipgui_timer_mgr_t *mgr);
__IPGUI_API__ void ipgui_timer_init     (ipg_tmr_t *tmr, ipgui_tick_t period, u32_t rounds,
                                          ipgui_timer_cb_t cb, void *userdata);
__IPGUI_API__ ipgui_err_t ipgui_timer_start   (ipgui_timer_mgr_t *mgr, ipg_tmr_t *tmr, ipgui_tick_t delay);
__IPGUI_API__ void        ipgui_timer_stop    (ipg_tmr_t *tmr);
__IPGUI_API__ ipgui_err_t ipgui_timer_restart (ipgui_timer_mgr_t *mgr, ipg_tmr_t *tmr);
__IPGUI_API__ void        ipgui_timer_tick    (ipgui_timer_mgr_t *mgr);
__IPGUI_API__ void        ipgui_timer_loop    (ipgui_timer_mgr_t *mgr, ipgui_tick_t passed);
__IPGUI_API__ void        ipgui_timer_handler (ipgui_timer_mgr_t *mgr);

#endif
