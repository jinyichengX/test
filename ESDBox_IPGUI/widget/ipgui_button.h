#ifndef IPGUI_BUTTON_H
#define IPGUI_BUTTON_H

#include "ipgui_widget.h"
#include "ipgui_blend.h"
#include "ipgui_box_style.h"
#include "ipgui_draw_box_background.h"
#include "ipgui_draw_box_border.h"
#include "ipgui_draw_box_shadow.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 按钮样式 */
typedef struct {
    /* 背景 */
    ipgui_box_bg_style_t    bg;

    /* 边框 */
    ipgui_box_border_style_t border;

    /* 阴影（可多阴影叠加） */
    struct {
        ipgui_box_shadow_style_t style;
        u8_t                    enabled;
    } shadow;

    /* 盒子形状 */
    ipgui_box_style_t       shape;
} ipgui_button_style_t;

/* 按钮控件 */
typedef struct {
    ipgui_widget_t   base;    /* 基类，必须为第一个成员 */
    ipgui_button_style_t style;

    /* 文本预留 */
    const char *     text;    /* 按钮文本（暂不绘制，等待字体系统） */
} ipgui_button_t;

/* 创建按钮控件并挂载到 parent（parent 为 NULL 则创建游离控件） */
extern __IPGUI_API__ ipgui_button_t * ipgui_button_create(ipgui_widget_t * parent);

/* 初始化按钮样式为默认值 */
extern __IPGUI_API__ void ipgui_button_style_init(ipgui_button_style_t * style);

/* 获取按钮的父控件中的盒子区域（本地坐标），即 base.x, base.y, base.x+w, base.y+h */
extern __IPGUI_API__ void ipgui_button_get_box(ipgui_button_t * btn, ipgui_aabb_t * box);

#ifdef __cplusplus
}
#endif

#endif
