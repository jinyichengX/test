#ifndef IPGUI_DROPDOWN_H
#define IPGUI_DROPDOWN_H

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
    ipgui_color_t           text_color;
    ipgui_color_t           arrow_color;
    const char            * value;
    const char            * options[16];
    int                     opt_count;
    int                     selected;
    int                     expanded;
} ipgui_dropdown_style_t;

typedef struct {
    ipgui_widget_t          base;
    ipgui_dropdown_style_t  style;
} ipgui_dropdown_t;

extern __IPGUI_API__ ipgui_dropdown_t * ipgui_dropdown_create(ipgui_widget_t * parent);
extern __IPGUI_API__ void ipgui_dropdown_style_init(ipgui_dropdown_style_t * s);

#ifdef __cplusplus
}
#endif
#endif
