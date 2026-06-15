#ifndef IPGUI_COLOR_SWATCH_H
#define IPGUI_COLOR_SWATCH_H

#include "ipgui_widget.h"
#include "ipgui_box_style.h"
#include "ipgui_draw_box_background.h"
#include "ipgui_draw_box_border.h"
#include "ipgui_draw_box_shadow.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    ipgui_color_t           color;
    ipgui_box_style_t       shape;
    ipgui_box_border_style_t border;
    ipgui_box_shadow_style_t shadow;
    int                     has_shadow;
    int                     selected;    /* 选中时加外圈 */
} ipgui_color_swatch_style_t;

typedef struct {
    ipgui_widget_t             base;
    ipgui_color_swatch_style_t style;
} ipgui_color_swatch_t;

extern __IPGUI_API__ ipgui_color_swatch_t * ipgui_color_swatch_create(ipgui_widget_t * parent);
extern __IPGUI_API__ void ipgui_color_swatch_style_init(ipgui_color_swatch_style_t * s);

#ifdef __cplusplus
}
#endif
#endif
