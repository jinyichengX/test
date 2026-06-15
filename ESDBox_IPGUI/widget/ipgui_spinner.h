#ifndef IPGUI_SPINNER_H
#define IPGUI_SPINNER_H

#include "ipgui_widget.h"
#include "ipgui_draw_arc.h"
#include "ipgui_blend.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    ipgui_color_t       color;
    ipgui_color_t       track_color;
    int                 progress;     /* 0-100 */
    int                 line_width;
    int                 indeterminate; /* 不停旋转模式 */
} ipgui_spinner_style_t;

typedef struct {
    ipgui_widget_t       base;
    ipgui_spinner_style_t style;
} ipgui_spinner_t;

extern __IPGUI_API__ ipgui_spinner_t * ipgui_spinner_create(ipgui_widget_t * parent);
extern __IPGUI_API__ void ipgui_spinner_style_init(ipgui_spinner_style_t * s);

#ifdef __cplusplus
}
#endif
#endif
