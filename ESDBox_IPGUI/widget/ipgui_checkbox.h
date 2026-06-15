#ifndef IPGUI_CHECKBOX_H
#define IPGUI_CHECKBOX_H

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
    ipgui_color_t           check_color;
    u8_t                    checked;
    const char            * text;
    ipgui_color_t           text_color;
} ipgui_checkbox_style_t;

typedef struct {
    ipgui_widget_t          base;
    ipgui_checkbox_style_t  style;
} ipgui_checkbox_t;

extern __IPGUI_API__ ipgui_checkbox_t * ipgui_checkbox_create(ipgui_widget_t * parent);
extern __IPGUI_API__ void ipgui_checkbox_style_init(ipgui_checkbox_style_t * s);

#ifdef __cplusplus
}
#endif
#endif
