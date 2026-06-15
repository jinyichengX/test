#ifndef IPGUI_RATING_H
#define IPGUI_RATING_H

#include "ipgui_widget.h"
#include "ipgui_box_style.h"
#include "ipgui_draw_box_background.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    IPGUI_RATING_STYLE_STAR,   /* 实心方块星 */
    IPGUI_RATING_STYLE_DOT,    /* 圆点 */
    IPGUI_RATING_STYLE_BLOCK,  /* 方块 */
} ipgui_rating_display_t;

typedef struct {
    ipgui_rating_display_t display;
    ipgui_color_t          active_color;
    ipgui_color_t          inactive_color;
    int                    value;     /* 当前值 */
    int                    max;       /* 最大值 */
    int                    size;      /* 单个星/点大小 */
} ipgui_rating_style_t;

typedef struct {
    ipgui_widget_t       base;
    ipgui_rating_style_t style;
} ipgui_rating_t;

extern __IPGUI_API__ ipgui_rating_t * ipgui_rating_create(ipgui_widget_t * parent);
extern __IPGUI_API__ void ipgui_rating_style_init(ipgui_rating_style_t * s);

#ifdef __cplusplus
}
#endif
#endif
