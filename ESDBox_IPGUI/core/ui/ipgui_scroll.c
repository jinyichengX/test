#include "ipgui_scroll.h"

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

/* ease_out(x) = 1 - (1-x)³ x∈[0,1]，y∈[0,1] 
 * return 0-256
 */
__IPGUI_STATIC__ u16_t scroll_ease_out_cubic(u16_t progress/* 0-256 */)
{
/* ease-out 精度: SCALE 越小精度越低但不会溢出 */
#define SCROLL_EASE_SCALE 256  /* 2^8, 除法变移位 */
    u32_t one_minus = SCROLL_EASE_SCALE - progress;
    u32_t cube      = one_minus * one_minus * one_minus;
    return SCROLL_EASE_SCALE - (cube >> 16);
}

__IPGUI_STATIC__ ipgui_coord_t ipgui_scroll_anim_func(ipgui_anim_t * anim, ipgui_tick_t t, void * data)
{
    ipgui_scroll_t * s = (ipgui_scroll_t *)data;
    u16_t progress = (u32_t)t * SCROLL_EASE_SCALE / anim->duration;
    return (scroll_ease_out_cubic(progress) * s->dist) >> 8;
}

__IPGUI_STATIC__ void scroll_path(ipgui_anim_t * anim, ipgui_anim_value_t value, void * path_cb_user_data)
{
    ipgui_widget_t * widget = (ipgui_widget_t *)path_cb_user_data;
    ipgui_scroll_t * s = &widget->scroll;
    if (!s->active) return;
    if (s->axis == 0) {
        widget->scroll_x = widget->scroll.start_off + value;
    } else {
        widget->scroll_y = widget->scroll.start_off + value;
    }
}

__IPGUI_API__ void ipgui_scroll_stop(struct ipgui_widget * widget)
{
    // if (!widget) return;

    // if (!widget->scroll.anim) return;
    
    // /* delete animation */
    // ipgui_anim_delete(widget->scroll.anim);
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

    // widget->scroll.active     = 1;
    // widget->scroll.axis       = axis;
    // widget->scroll.start_off  = (axis == 0) ? widget->scroll_x : widget->scroll_y;
    // widget->scroll.dist       = dist;
    // widget->scroll.duration   = duration;

    ipgui_anim_dsc_t anim_dsc = {
        .anim_func = ipgui_scroll_anim_func,
        .data = (void *)&widget->scroll,
        .t1 = 0,
        .t2 = duration,
        .loop_type = IPGUI_ANIM_LOOP_DEFAULT,
        .loop_count = 1,
        .start_delay = 0,
        .path_cb = (ipgui_anim_path_cb_t)0,
        .path_cb_user_data = (void *)&widget,
        .finish_cb = (ipgui_anim_finish_cb_t)0
    };
    ipgui_anim_t * anim;
    anim = ipgui_anim_create(&anim_dsc);
    if (!anim) return;

    ipgui_anim_start(anim);
}
