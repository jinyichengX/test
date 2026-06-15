#ifndef IPGUI_SLIDER_H
#define IPGUI_SLIDER_H

#include "ipgui_widget.h"
#include "ipgui_box_style.h"
#include "ipgui_draw_box_background.h"
#include "ipgui_draw_box_border.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    ipgui_box_style_t       shape;
    ipgui_box_bg_style_t    track_bg;
    ipgui_box_border_style_t track_border;
    ipgui_box_bg_style_t    fill_bg;
    ipgui_box_style_t       knob_shape;
    ipgui_box_bg_style_t    knob_bg;
    ipgui_box_border_style_t knob_border;
    int                     value;
    int                     min;
    int                     max;
} ipgui_slider_style_t;

typedef struct {
    ipgui_widget_t        base;
    ipgui_slider_style_t  style;
} ipgui_slider_t;

extern __IPGUI_API__ ipgui_slider_t * ipgui_slider_create(ipgui_widget_t * parent);
extern __IPGUI_API__ void ipgui_slider_style_init(ipgui_slider_style_t * s);

#ifdef __cplusplus
}
#endif
#endif
