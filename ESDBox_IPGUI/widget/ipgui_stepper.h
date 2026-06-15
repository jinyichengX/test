#ifndef IPGUI_STEPPER_H
#define IPGUI_STEPPER_H

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
    ipgui_color_t           btn_color;  /* +/- 块颜色 */
    ipgui_color_t           btn_text;   /* +/- 符号颜色 */
    int                     value;
    int                     min;
    int                     max;
    int                     step;
} ipgui_stepper_style_t;

typedef struct {
    ipgui_widget_t         base;
    ipgui_stepper_style_t  style;
} ipgui_stepper_t;

extern __IPGUI_API__ ipgui_stepper_t * ipgui_stepper_create(ipgui_widget_t * parent);
extern __IPGUI_API__ void ipgui_stepper_style_init(ipgui_stepper_style_t * s);

#ifdef __cplusplus
}
#endif
#endif
