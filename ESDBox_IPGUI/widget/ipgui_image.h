#ifndef IPGUI_IMAGE_WIDGET_H
#define IPGUI_IMAGE_WIDGET_H

#include "ipgui_widget.h"
#include "ipgui_box_style.h"
#include "ipgui_draw_image.h"
#include "ipgui_draw_image_api.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    ipgui_box_style_t           shape;
    ipgui_image_draw_style_t    draw_style;
    ipgui_image_align_t         align;
    ipgui_image_fit_t           fit;
    ipgui_image_data_t        * img;
} ipgui_image_style_t;

typedef struct {
    ipgui_widget_t       base;
    ipgui_image_style_t  style;
} ipgui_image_t;

extern __IPGUI_API__ ipgui_image_t * ipgui_image_create(ipgui_widget_t * parent);
extern __IPGUI_API__ void ipgui_image_style_init(ipgui_image_style_t * s);

#ifdef __cplusplus
}
#endif
#endif
