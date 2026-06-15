#ifndef IPGUI_SWITCH_H
#define IPGUI_SWITCH_H

#include "ipgui_widget.h"
#include "ipgui_box_style.h"
#include "ipgui_draw_box_background.h"
#include "ipgui_draw_box_border.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    ipgui_box_style_t       shape;
    ipgui_box_bg_style_t    bg_on;
    ipgui_box_bg_style_t    bg_off;
    ipgui_box_border_style_t border;
    ipgui_color_t           knob_color;
    u8_t                    toggled;
} ipgui_switch_style_t;

typedef struct {
    ipgui_widget_t         base;
    ipgui_switch_style_t   style;
} ipgui_switch_t;

extern __IPGUI_API__ ipgui_switch_t * ipgui_switch_create(ipgui_widget_t * parent);
extern __IPGUI_API__ void ipgui_switch_style_init(ipgui_switch_style_t * s);
extern __IPGUI_API__ void ipgui_switch_toggle(ipgui_switch_t * sw);

#ifdef __cplusplus
}
#endif
#endif
