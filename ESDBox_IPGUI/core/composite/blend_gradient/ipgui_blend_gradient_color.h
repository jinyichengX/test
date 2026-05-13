#ifndef IPGUI_BLEND_COLOR_GRADIENT_H
#define IPGUI_BLEND_COLOR_GRADIENT_H

#include "ipgui_conf.h"
#include "ipgui_gradient_color.h"
#include "ipgui_core.h"
#include "ipgui_blend_mode.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    ipgui_gradient_type_t grad_type;
    union {
        ipgui_liner_gradient_color_t liner_grad;
        ipgui_radial_gradient_color_t radial_grad;
        ipgui_conic_gradient_color_t conic_grad;
    }grad;
}ipgui_grad_src_t;

__IPGUI_API__ void ipgui_fill_gradient_color(
    ipgui_surf_t         * surf,
    ipgui_aabb_t         * clip,
    ipgui_aabb_t         * dest,
    ipgui_grad_src_t     * gradient,
    u8_t                   opacity,
    ipgui_blend_mode_t     blend_mode);

__IPGUI_API__ void ipgui_blend_gradient_color(
    ipgui_surf_t         * surf,
    ipgui_aabb_t         * clip,
    ipgui_aabb_t         * dest,
    ipgui_grad_src_t     * gradient,
    u8_t                   opacity,
    u8_t                 * mask,      /* mask覆盖mask_aabb区域 */
    ipgui_aabb_t         * mask_aabb, /* mask_aabb必须大于或等于dest区域 */
    ipgui_blend_mode_t     blend_mode);

#ifdef __cplusplus
}
#endif

#endif