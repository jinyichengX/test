#ifndef IPGUI_BLEND_H
#define IPGUI_BLEND_H

#include "ipgui_blend_color.h"
#include "ipgui_blend_gradient_color.h"
#include "ipgui_blend_image.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    IPGUI_PAINT_COLOR, /* default: paint color */
    IPGUI_PAINT_GRADIENT,
    IPGUI_PAINT_IMAGE,
}ipgui_paint_type_t;

typedef struct {
    /* paint type */
    ipgui_paint_type_t    type;

    /* paint source */
    union {
        ipgui_color_t     color;
        ipgui_grad_src_t  grad_src;
        ipgui_image_src_t image_src;
    }src;

    // ipgui_gradient_mask * mask_image; //考虑将渐变蒙版加到这
}ipgui_paint_t;

void ipgui_blend(
    ipgui_surf_t         * surf,
    ipgui_aabb_t         * clip,
    ipgui_aabb_t         * dest,
    ipgui_paint_t        * paint,
    u8_t                   opacity,
    u8_t                 * mask,
    ipgui_aabb_t         * mask_aabb,
    ipgui_blend_mode_t     blend_mode);

#ifdef __cplusplus
}
#endif

#endif