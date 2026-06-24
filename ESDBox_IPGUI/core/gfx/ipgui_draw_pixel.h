#ifndef IPGUI_DRAW_PIXEL_H
#define IPGUI_DRAW_PIXEL_H

#include "ipgui_blend_color.h"

#ifdef __cplusplus
extern "C" {
#endif

void ipgui_draw_pixel(
        ipgui_surf_t     * surf, 
        ipgui_aabb_t     * clip,
        ipgui_coord_t      x, 
        ipgui_coord_t      y, 
        ipgui_color_t      color, 
        u8_t               mask,
        u8_t               opacity,
        ipgui_blend_mode_t blend_mode);
        
#ifdef __cplusplus
}
#endif

#endif