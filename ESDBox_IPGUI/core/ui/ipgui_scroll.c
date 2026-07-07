#include "ipgui_scroll.h"
#include "ipgui_widget.h"
#include "ipgui_animation.h"

/*
 * 惯性滚动 — 公式:
 *   v  = dx (px/tick, 1 tick = 1ms, 不除以 dt)
 *   a  = 0.0015 (常数减速度)
 *   s  = |v| / (2 * a)        → 滚动距离 (px)
 *   t  = |v| / a              → 持续时间 (tick)
 *   offset(n) = start_off + s * easeOut(n / t)
 *   easeOut(x) = 1 - (1 - x)³
 */

/* a = 0.0015 = 15 / 10000, 用定点运算避免浮点 */
#define A_NUM   15
#define A_DEN   10000

/* ease_out(x) = 1 - (1-x)³ x∈[0,1]，y∈[0,1] */
__IPGUI_STATIC__ ipgui_coord_t scroll_ease_out_cubic(ipgui_coord_t progress)
{
/* ease-out 精度: SCALE 越小精度越低但不会溢出 */
#define SCROLL_EASE_SCALE 256  /* 2^8, 除法变移位 */
    ipgui_coord_t one_minus = SCROLL_EASE_SCALE - progress;
    ipgui_coord_t cube      = one_minus * one_minus * one_minus;
    return SCROLL_EASE_SCALE - (cube >> 16);
}

__IPGUI_API__ ipgui_yes_no_t ipgui_scroll_is_active(ipgui_scroll_t * scroll)
{
    if (!scroll->active) return IPGUI_NO;
    return IPGUI_YES;
}

__IPGUI_API__ void ipgui_scroll_stop(struct ipgui_widget * widget)
{
    if (!widget) return;
    widget->scroll.active = 0;
}

__IPGUI_API__ void ipgui_scroll_start(
    struct ipgui_widget * widget,
    ipgui_coord_t         scroll_v,
    u8_t                  axis)
{
    if (!widget) return;

    /* |scroll_v| */
    ipgui_coord_t abs_v = scroll_v >= 0 ? scroll_v : -scroll_v;

    if (abs_v == 0) return;

    /*
     * s = |v| * 1000 / 3,  t = |v| * 2000 / 3
     * 用乘加移位替代除法, O0 也高效
     */
    ipgui_coord_t dist = (abs_v * 2048) >> 3;

    if (dist == 0) return;

    ipgui_tick_t duration = (u32_t)((abs_v * 4096) >> 3);
    /* 太短不处理 */
    if (duration < 5) return;

    /* 方向回符号 */
    if (scroll_v < 0) dist = -dist;

    widget->scroll.active     = 1;
    widget->scroll.axis       = axis;
    widget->scroll.start_off  = (axis == 0) ? widget->scroll_x : widget->scroll_y;
    widget->scroll.dist       = dist;
    widget->scroll.start_tick = ipgui_tick_now();
    widget->scroll.duration   = duration;

    ipgui_anim_dsc_t anim_dsc = {
        .anim_func = NULL,
        .t1 = 0,
        .t2 = duration,
        .loop_type = IPGUI_ANIM_LOOP_DEFAULT,
        .loop_count = 1,
        .start_delay = 0,
        .path_cb = NULL,
        .finish_cb = NULL
    };
    ipgui_anim_t * anim;
    anim = ipgui_anim_create(&anim_dsc);
    if (!anim) return;

    ipgui_anim_start(anim);
}

__IPGUI_API__ void ipgui_scroll_update(ipgui_scroll_t * scroll)
{
    if (!scroll->active) return;

    // ipgui_tick_t  elapsed_ticks = ipgui_tick_now() - scroll->start_tick;
    // u32_t         elapsed_ms    = ipgui_tick2millis(elapsed_ticks);

    // if (elapsed_ms >= widget->scroll.duration) {
    //     /* 动画结束 */
    //     ipgui_coord_t final_off = widget->scroll.start_off + widget->scroll.dist;
    //     if (widget->scroll.axis == 0) widget->scroll_x = final_off;
    //     else                          widget->scroll_y = final_off;
    //     widget->scroll.active = 0;
    //     ipgui_widget_mark_dirty(widget);
    //     return;
    // }

    // ipgui_coord_t progress = (ipgui_coord_t)(elapsed_ms * SCROLL_EASE_SCALE / widget->scroll.duration);
    // ipgui_coord_t eased    = ease_out_cubic(progress);
    // ipgui_coord_t offset   = widget->scroll.start_off
    //                        + ((widget->scroll.dist * eased) >> 7);  /* /128 */

    // if (widget->scroll.axis == 0) widget->scroll_x = offset;
    // else                          widget->scroll_y = offset;

    // ipgui_widget_mark_dirty(widget);
}
