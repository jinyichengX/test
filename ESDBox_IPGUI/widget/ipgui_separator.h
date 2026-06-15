#ifndef IPGUI_SEPARATOR_H
#define IPGUI_SEPARATOR_H

#include "ipgui_widget.h"
#include "ipgui_box_style.h"
#include "ipgui_draw_box_background.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    IPGUI_SEP_HORIZONTAL,
    IPGUI_SEP_VERTICAL,
} ipgui_separator_dir_t;

typedef struct {
    ipgui_separator_dir_t dir;
    ipgui_color_t         color;
    int                   thickness;
} ipgui_separator_style_t;

typedef struct {
    ipgui_widget_t          base;
    ipgui_separator_style_t style;
} ipgui_separator_t;

extern __IPGUI_API__ ipgui_separator_t * ipgui_separator_create(ipgui_widget_t * parent);
extern __IPGUI_API__ void ipgui_separator_style_init(ipgui_separator_style_t * s);

#ifdef __cplusplus
}
#endif
#endif
