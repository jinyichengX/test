#ifndef IPGUI_SCROLL_H
#define IPGUI_SCROLL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ipgui_types.h"
#include "ipgui_coord.h"
#include "ipgui_utils.h"
#include "ipgui_animation.h"

typedef struct {
    ipgui_coord_t  start_off;    /* 动画起始 scroll 偏移 */
    ipgui_coord_t  dist;         /* 带符号的滚动距离 s (px) */
    ipgui_tick_t   duration;     /* 动画总时长 (ms) */
    u32_t          recip_fp;     /* (1<<32)/dur 定点倒数，避免 per-tick 64 除 */
    ipgui_anim_t * anim;         /* 惯性滚动动画对象 */
    u8_t           axis;         /* 0 = x 轴, 1 = y 轴 */
} ipgui_scroll_t;

/* 停止当前惯性滚动 */
extern __IPGUI_API__ void ipgui_inertia_scroll_stop(struct ipgui_widget * widget);

extern __IPGUI_API__ void ipgui_scroll_start(
    struct ipgui_widget * widget,
    s32_t                 scroll_v,
    u8_t                  axis);

#ifdef __cplusplus
}
#endif

#endif
