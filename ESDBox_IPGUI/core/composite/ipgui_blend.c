#include "ipgui_blend.h"

void ipgui_blend(
    ipgui_surf_t         * surf,
    ipgui_aabb_t         * clip,
    ipgui_aabb_t         * dest,
    ipgui_paint_t        * paint,
    u8_t                   opacity,
    u8_t                 * mask,
    ipgui_aabb_t         * mask_aabb,
    ipgui_blend_mode_t     blend_mode)
{
    switch (paint->type)
    {
    case IPGUI_PAINT_COLOR:
        ipgui_blend_color(
            surf,
            clip,
            dest,
            paint->src.color,
            opacity,
            mask,
            mask_aabb,
            blend_mode);
        break;
    case IPGUI_PAINT_GRADIENT:
        ipgui_blend_gradient_color(
            surf,
            clip,
            dest,
            &(paint->src.grad_src), 
            opacity,
            mask,
            mask_aabb,
            blend_mode);
        break;
    case IPGUI_PAINT_IMAGE:
        ipgui_blend_image_v2(
            surf,
            clip,
            dest,
            &(paint->src.image_src),
            opacity,
            mask,
            mask_aabb,
            blend_mode);
        break;
    default:
        break;
    }
}