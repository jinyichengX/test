#ifndef IPGUI_BADGE_H
#define IPGUI_BADGE_H

#include "ipgui_widget.h"
#include "ipgui_box_style.h"
#include "ipgui_draw_box_background.h"
#include "ipgui_draw_box_border.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    IPGUI_BADGE_TYPE_DOT,    /* 纯圆点 */
    IPGUI_BADGE_TYPE_RING,   /* 空心圆环 */
} ipgui_badge_type_t;

typedef struct {
    ipgui_badge_type_t  type;
    ipgui_color_t       color;
    ipgui_color_t       ring_color;
    int                 ring_width;
} ipgui_badge_style_t;

typedef struct {
    ipgui_widget_t       base;
    ipgui_badge_style_t  style;
} ipgui_badge_t;

extern __IPGUI_API__ ipgui_badge_t * ipgui_badge_create(ipgui_widget_t * parent);
extern __IPGUI_API__ void ipgui_badge_style_init(ipgui_badge_style_t * s);

#ifdef __cplusplus
}
#endif
#endif
