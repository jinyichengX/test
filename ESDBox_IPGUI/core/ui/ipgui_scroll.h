#ifndef IPGUI_SCROLL_H
#define IPGUI_SCROLL_H

#include "ipgui_types.h"
#include "ipgui_coord.h"
#include "ipgui_utils.h"
#include "ipgui_widget.h"
#include "ipgui_animation.h"

typedef enum {
    IPGUI_SCROLL_DIR_AUTO_XY = 0, /* 自动选择滚动轴x或y */
    IPGUI_SCROLL_DIR_X,           /* 沿x轴滚动 */
    IPGUI_SCROLL_DIR_Y,           /* 沿y轴滚动 */
    IPGUI_SCROLL_DIR_GESTURE,     /* 沿手势滚动 */
    IPGUI_SCROLL_DIR_CUSTOM_VEC,  /* 自定义滚动方向 */
} ipgui_scroll_dir_t;

typedef struct {
    ipgui_scroll_dir_t dir;
    u8_t           active;       /* 1 = 惯性滚动进行中 */
    u8_t           axis;         /* 0 = x 轴, 1 = y 轴 */
    ipgui_coord_t  start_off;    /* 动画起始 scroll_x/y */
    ipgui_coord_t  dist;         /* 带符号的滚动距离 s (px) */
    ipgui_tick_t   duration;     /* 动画总时长 (ms) */
    ipgui_anim_t * anim;         /* 惯性滚动动画对象 */
} ipgui_scroll_t;

/* 停止当前惯性滚动 */
extern __IPGUI_API__ void ipgui_scroll_stop(struct ipgui_widget * widget);

/* 启动惯性滚动 (v: 滚动方向速度 px/tick, axis: 0=x 1=y) */
extern __IPGUI_API__ void ipgui_scroll_start(struct ipgui_widget * widget, ipgui_coord_t scroll_v, u8_t axis);

#endif
