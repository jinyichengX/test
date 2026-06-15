#ifndef IPGUI_LABEL_H
#define IPGUI_LABEL_H

#include "ipgui_widget.h"
#include "ipgui_draw_box_background.h"
#include "ipgui_draw_box_border.h"
#include "ipgui_box_style.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    IPGUI_TEXT_ALIGN_LEFT,
    IPGUI_TEXT_ALIGN_CENTER,
    IPGUI_TEXT_ALIGN_RIGHT,
} ipgui_text_align_t;

typedef struct {
    ipgui_box_style_t       shape;
    ipgui_box_bg_style_t    bg;
    ipgui_box_border_style_t border;
    ipgui_color_t           text_color;
    ipgui_text_align_t      align;
    const char            * text;
} ipgui_label_style_t;

typedef struct {
    ipgui_widget_t      base;
    ipgui_label_style_t style;
} ipgui_label_t;

extern __IPGUI_API__ ipgui_label_t * ipgui_label_create(ipgui_widget_t * parent);
extern __IPGUI_API__ void ipgui_label_style_init(ipgui_label_style_t * s);

#ifdef __cplusplus
}
#endif
#endif
