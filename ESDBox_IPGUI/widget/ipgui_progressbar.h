#ifndef IPGUI_PROGRESSBAR_H
#define IPGUI_PROGRESSBAR_H

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
    ipgui_box_border_style_t fill_border;
    int                     value;
    int                     max;
} ipgui_progressbar_style_t;

typedef struct {
    ipgui_widget_t             base;
    ipgui_progressbar_style_t  style;
} ipgui_progressbar_t;

extern __IPGUI_API__ ipgui_progressbar_t * ipgui_progressbar_create(ipgui_widget_t * parent);
extern __IPGUI_API__ void ipgui_progressbar_style_init(ipgui_progressbar_style_t * s);

#ifdef __cplusplus
}
#endif
#endif
