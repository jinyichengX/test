#ifndef IPGUI_AVATAR_H
#define IPGUI_AVATAR_H

#include "ipgui_widget.h"
#include "ipgui_box_style.h"
#include "ipgui_draw_box_background.h"
#include "ipgui_draw_box_border.h"
#include "ipgui_gradient_color.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    ipgui_box_style_t       shape;
    ipgui_box_bg_style_t    bg;
    ipgui_box_border_style_t border;
    int                     online;     /* 1=绿点, 2=红点, 0=离线 */
    ipgui_color_t           dot_color;  /* 覆盖颜色 */
} ipgui_avatar_style_t;

typedef struct {
    ipgui_widget_t       base;
    ipgui_avatar_style_t style;
} ipgui_avatar_t;

extern __IPGUI_API__ ipgui_avatar_t * ipgui_avatar_create(ipgui_widget_t * parent);
extern __IPGUI_API__ void ipgui_avatar_style_init(ipgui_avatar_style_t * s);

#ifdef __cplusplus
}
#endif
#endif
