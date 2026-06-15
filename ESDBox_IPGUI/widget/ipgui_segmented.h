#ifndef IPGUI_SEGMENTED_H
#define IPGUI_SEGMENTED_H

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
    ipgui_box_bg_style_t    sel_bg;
    int                     selected;   /* 0..n-1 */
    int                     segments;   /* 段数 */
} ipgui_segmented_style_t;

typedef struct {
    ipgui_widget_t          base;
    ipgui_segmented_style_t style;
} ipgui_segmented_t;

extern __IPGUI_API__ ipgui_segmented_t * ipgui_segmented_create(ipgui_widget_t * parent);
extern __IPGUI_API__ void ipgui_segmented_style_init(ipgui_segmented_style_t * s);

#ifdef __cplusplus
}
#endif
#endif
