#include "ipgui_widget.h"
#include "ipgui_scroll.h"

ipgui_coord_t scroll_anim_func(
    ipgui_anim_t * anim, 
    ipgui_tick_t   t, 
    void         * data)
{
    ipgui_scroll_t * s = (ipgui_scroll_t *)data;

    /* ease_out(x) = 1 - (1-x)³, 16-bit 定点精度
     * progress = t / duration * 65536, 用预计算倒数避免 per-tick 除法
     */
    u32_t progress = (u32_t)(((u64_t)t * s->recip_fp) >> 16);
    u32_t diff       = 65536U - progress;

    /* 用两次 UMULL 直接算 diff³ >> 32，避免通用 64 位乘法
     * diff³ >> 32  =  (diff² >> 16) * diff >> 16
     * 两次 (u64)u32 * u32 编译器会映射为单周期 UMULL */
    u32_t diff_sq_hi = (u32_t)(((u64_t)diff * diff) >> 16);
    u32_t ease_out   = 65536U - (u32_t)(((u64_t)diff_sq_hi * diff) >> 16);

    return (ipgui_coord_t)(((s64_t)ease_out * s->dist) >> 16);
}

__IPGUI_STATIC__ void scroll_path(
    ipgui_anim_t     * anim,
    ipgui_anim_value_t value,
    void             * p)
{
    ipgui_widget_t * widget = (ipgui_widget_t *)p;
    if (widget->scroll.axis == 0) {
        widget->scroll_x = widget->scroll.start_off - value;
    } else {
        widget->scroll_y = widget->scroll.start_off - value;
    }
    ipgui_widget_mark_dirty(widget);
}

__IPGUI_STATIC__ void scroll_anim_finish_callback(
    ipgui_anim_t * anim, 
    void         * p)
{
    ipgui_scroll_t * s = (ipgui_scroll_t *)p;
    s->anim = (ipgui_anim_t *)0;
}

void ipgui_inertia_scroll_stop(struct ipgui_widget * widget)
{
    if (!widget->scroll.anim)
        return;

    ipgui_anim_delete(widget->scroll.anim);
    widget->scroll.anim = (ipgui_anim_t *)0;
}

__IPGUI_API__ void ipgui_scroll_start(
    struct ipgui_widget * widget,
    s32_t                 scroll_v,
    u8_t                  axis)
{
    if (!widget) return;

    /* |scroll_v| */
    s32_t abs_v = scroll_v >= 0 ? scroll_v : -scroll_v;
    
    if (abs_v == 0) return;
    /*
     * 惯性滚动 — 公式:
     *   v  = dx (px/tick, 1 tick = 1ms, 不除以 dt)
     *   a  = 0.0015 (常数减速度)
     *   s  = |v| / (2 * a)        → 滚动距离 (px)
     *   t  = |v| / a              → 持续时间 (tick)
     *   offset(n) = start_off + s * easeOut(n / t)
     *   easeOut(x) = 1 - (1 - x)³
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

    widget->scroll.axis       = axis;
    widget->scroll.start_off  = (axis == 0) ? widget->scroll_x : widget->scroll_y;
    widget->scroll.dist       = dist;
    widget->scroll.duration   = duration;
    /* 预计算 1/duration 定点倒数，避免 per-tick 64 位除法 */
    widget->scroll.recip_fp   = (u32_t)((1ULL << 32) / (u64_t)(duration + 1));

    ipgui_anim_dsc_t anim_dsc = {
        .anim_func = scroll_anim_func,
        .data = (void *)&widget->scroll,
        .t1 = 0,
        .t2 = duration,
        .loop_type = IPGUI_ANIM_LOOP_DEFAULT,
        .loop_count = 1,
        .start_delay = 0,
        .path_cb = scroll_path,
        .path_cb_user_data = (void *)widget,
        .finish_cb = scroll_anim_finish_callback,
        .finish_cb_user_data = (void *)&widget->scroll,
    };
    ipgui_anim_t * anim;
    anim = ipgui_anim_create(&anim_dsc);
    if (!anim) return;

    widget->scroll.anim = anim;
    ipgui_anim_start(anim);
}
