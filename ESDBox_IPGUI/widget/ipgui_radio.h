#ifndef IPGUI_RADIO_H
#define IPGUI_RADIO_H

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
    ipgui_color_t           dot_color;
    u8_t                    selected;
    const char            * text;
    ipgui_color_t           text_color;
} ipgui_radio_style_t;

typedef struct {
    ipgui_widget_t        base;
    ipgui_radio_style_t   style;
} ipgui_radio_t;

extern __IPGUI_API__ ipgui_radio_t * ipgui_radio_create(ipgui_widget_t * parent);
extern __IPGUI_API__ void ipgui_radio_style_init(ipgui_radio_style_t * s);

#ifdef __cplusplus
}
#endif
#endif
