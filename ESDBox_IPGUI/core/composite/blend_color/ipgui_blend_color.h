#ifndef IPGUI_BLEND_COLOR_H
#define IPGUI_BLEND_COLOR_H

#include "ipgui_core.h"
#include "ipgui_utils.h"
#include "ipgui_color.h"
#include "ipgui_blend_mode.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef u32_t (* solid_convert_func_t)(ipgui_color_t color, u8_t * pix);

typedef void (* premult_blend_func_t)(ipgui_color_t color, u8_t * pix, ipgui_blend_mode_t blend_mode);

__IPGUI_API__ void ipgui_fill_color(
    ipgui_surf_t     * surf, 
    ipgui_aabb_t     * clip,
    ipgui_aabb_t     * dest, 
    ipgui_color_t      color, 
    u8_t               opacity,
    ipgui_blend_mode_t blend_mode);

__IPGUI_API__ void ipgui_blend_color(
    ipgui_surf_t     * surf,
    ipgui_aabb_t     * clip,
    ipgui_aabb_t     * dest,
    ipgui_color_t      color,
    u8_t               opacity,
    u8_t             * mask,      /* mask覆盖mask_aabb区域 */
    ipgui_aabb_t     * mask_aabb,  /* mask对应的坐标区域，必须大于或等于dest区域 */
    ipgui_blend_mode_t blend_mode);

__IPGUI_API__ ipgui_color_t ipgui_color_combine_opacity_and_premultiply(
    ipgui_color_t * color, u8_t opacity);

#ifdef __cplusplus
}
#endif

#endif