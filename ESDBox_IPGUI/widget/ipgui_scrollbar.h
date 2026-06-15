#ifndef IPGUI_SCROLLBAR_H
#define IPGUI_SCROLLBAR_H

#include "ipgui_widget.h"
#include "ipgui_box_style.h"
#include "ipgui_draw_box_background.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    IPGUI_SCROLLBAR_HORIZONTAL,
    IPGUI_SCROLLBAR_VERTICAL,
} ipgui_scrollbar_dir_t;

typedef struct {
    ipgui_scrollbar_dir_t dir;
    ipgui_color_t         track_color;
    ipgui_color_t         thumb_color;
    int                   thickness;  /* 轨道宽/高 */
    int                   value;      /* 0..max */
    int                   max;
    int                   thumb_len;  /* thumb 长度, 0=自动按比例 */
} ipgui_scrollbar_style_t;

typedef struct {
    ipgui_widget_t          base;
    ipgui_scrollbar_style_t style;
} ipgui_scrollbar_t;

extern __IPGUI_API__ ipgui_scrollbar_t * ipgui_scrollbar_create(ipgui_widget_t * parent);
extern __IPGUI_API__ void ipgui_scrollbar_style_init(ipgui_scrollbar_style_t * s);

#ifdef __cplusplus
}
#endif
#endif
