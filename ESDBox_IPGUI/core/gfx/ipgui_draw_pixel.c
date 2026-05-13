#include "ipgui_draw_pixel.h"

extern premult_blend_func_t premult_blend_table[PIX_FMT_MAX];

void ipgui_draw_pixel(
        ipgui_surf_t     * surf, 
        ipgui_aabb_t     * clip,
        ipgui_coord_t      x, 
        ipgui_coord_t      y, 
        ipgui_color_t      color, 
        u8_t               mask,
        u8_t               opacity,
        ipgui_blend_mode_t blend_mode)
{
    ipgui_aabb_t draw;
    if (clip) {
        if (0 != ipgui_aabb_overlap(&draw, clip, &surf->surf))
            return;
    } else {
        draw = surf->surf;
    }

    if (!ipgui_point_in_aabb(x, y, &draw))
        return;
    
    premult_blend_func_t blend_fn = premult_blend_table[surf->pix_fmt];
    if (!blend_fn) return;

    /* draw the pixel */
    ipgui_color_t premult;
    u8_t * dest_cr_buf;
    u8_t mask_opacity_combined;

    /* mix mask and opacity */
    mask_opacity_combined = (u8_t)(((u32_t)opacity * mask + 127) >> 8);
    premult = ipgui_color_combine_opacity_and_premultiply(&color, mask_opacity_combined);

    dest_cr_buf = ipgui_surf_color_get(
        surf, 
        x - surf->surf.start.x, 
        y - surf->surf.start.y);
        
    blend_fn(premult, dest_cr_buf, blend_mode);
}