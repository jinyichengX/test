#ifndef IPGUI_SCROLL_H
#define IPGUI_SCROLL_H

#include "ipgui_types.h"
#include "ipgui_coord.h"
#include "ipgui_utils.h"

struct ipgui_widget;

typedef struct {
    u8_t           active;       /* 1 = 惯性滚动进行中 */
    u8_t           axis;         /* 0 = x 轴, 1 = y 轴 */
    ipgui_coord_t  start_off;    /* 动画起始 scroll_x/y */
    ipgui_coord_t  dist;         /* 带符号的滚动距离 s (px) */
    ipgui_tick_t   start_tick;   /* 动画起始时刻 */
    u32_t           duration;     /* 动画总时长 (ms) */
} ipgui_scroll_t;

/* 停止当前惯性滚动 */
extern __IPGUI_API__ void ipgui_scroll_stop(struct ipgui_widget * widget);

/* 启动惯性滚动 (v: 滚动方向速度 px/tick, axis: 0=x 1=y) */
extern __IPGUI_API__ void ipgui_scroll_start(
    struct ipgui_widget * widget,
    ipgui_coord_t         scroll_v,
    u8_t                  axis);

/* 推进滚动动画 (每 tick 调用一次, 在主循环中调用) */
extern __IPGUI_API__ void ipgui_scroll_update(struct ipgui_widget * widget);

#endif
