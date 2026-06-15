#ifndef IPGUI_PANEL_H
#define IPGUI_PANEL_H

#include "ipgui_widget.h"
#include "ipgui_box_style.h"
#include "ipgui_draw_box_background.h"
#include "ipgui_draw_box_border.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    ipgui_box_style_t       shape;
    ipgui_box_bg_style_t    bg;
    ipgui_box_border_style_t border;
    const char            * title;
    ipgui_color_t           title_color;
} ipgui_panel_style_t;

typedef struct {
    ipgui_widget_t       base;
    ipgui_panel_style_t  style;
} ipgui_panel_t;

extern __IPGUI_API__ ipgui_panel_t * ipgui_panel_create(ipgui_widget_t * parent);
extern __IPGUI_API__ void ipgui_panel_style_init(ipgui_panel_style_t * s);

#ifdef __cplusplus
}
#endif
#endif
