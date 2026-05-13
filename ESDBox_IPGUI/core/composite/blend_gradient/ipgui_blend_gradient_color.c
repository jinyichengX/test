#include "ipgui_blend_gradient_color.h"
#include "ipgui_blend_color.h"
#include "ipgui_gradient_color.h"
#include "ipgui_memory.h"
#include "ipgui_debug.h"

extern premult_blend_func_t premult_blend_table[PIX_FMT_MAX];
extern ipgui_color_t ipgui_color_combine_opacity(ipgui_color_t * color, u8_t opacity);
extern ipgui_color_t ipgui_color_premultiply(ipgui_color_t * color);
extern ipgui_color_t ipgui_color_combine_opacity_and_premultiply(ipgui_color_t * color, u8_t opacity);

__IPGUI_STATIC__ __IPGUI_INLINE__ u8_t ipgui_gradient_pos_at_xy(
    ipgui_grad_src_t * src,
    ipgui_coord_t x, ipgui_coord_t y)
{
    if (src->grad_type == IPGUI_GRADIENT_TYPE_LINEAR)
        return ipgui_get_liner_gradient_pos_at_xy(&src->grad.liner_grad, x, y);
    else if (src->grad_type == IPGUI_GRADIENT_TYPE_RADIAL)
        return ipgui_get_radial_gradient_pos_at_xy(&src->grad.radial_grad, x, y);
    else if (src->grad_type == IPGUI_GRADIENT_TYPE_CONIC)
        return ipgui_get_conic_gradient_pos_at_xy(&src->grad.conic_grad, x, y);
    else return 0;
}

__IPGUI_STATIC__ __IPGUI_INLINE__ void ipgui_gradient_color_get(
    ipgui_grad_src_t * src,
    u8_t pos, ipgui_color_t * res)
{
    if (src->grad_type == IPGUI_GRADIENT_TYPE_LINEAR)
        ipgui_liner_gradient_color_get(&src->grad.liner_grad, pos, res);
    else if (src->grad_type == IPGUI_GRADIENT_TYPE_RADIAL)
        ipgui_radial_gradient_color_get(&src->grad.radial_grad, pos, res);
    else if (src->grad_type == IPGUI_GRADIENT_TYPE_CONIC)
        ipgui_conic_gradient_color_get(&src->grad.conic_grad, pos, res);
    else {
        res->v = 0; /* error */
    }
}

__IPGUI_API__ void ipgui_fill_gradient_color(
    ipgui_surf_t         * surf,
    ipgui_aabb_t         * clip,
    ipgui_aabb_t         * dest,
    ipgui_grad_src_t     * gradient,
    u8_t                   opacity,
    ipgui_blend_mode_t     blend_mode)
{
    if ((!dest) || (!surf) || (!gradient) || (opacity < 3))
        return;

    ipgui_aabb_t fill_aabb;
    if (0 != ipgui_aabb_overlap(&fill_aabb, dest, &(surf->surf)))
        return;

    if (clip) {
        if (0 != ipgui_aabb_overlap(&fill_aabb, &fill_aabb, clip))
            return;
    }

    /* 检查线性渐变的方向
     * 如果是垂直/水平渐变，退化为分段纯色填充
     */
    if (gradient->grad_type == IPGUI_GRADIENT_TYPE_LINEAR) {
#define GRAD_COLOR_GET_AND_FILL \
do {\
    ipgui_liner_gradient_color_get(&gradient->grad.liner_grad, \
        ipgui_get_liner_gradient_pos_at_xy(&gradient->grad.liner_grad, \
            band.start.x, band.start.y), &cr);\
    ipgui_fill_color(surf, (ipgui_aabb_t *)0, &band,\
    cr, opacity, blend_mode);\
} while(0)

        ipgui_color_t cr;
        ipgui_aabb_t band;
        if (ipgui_if_liner_gradient_hor(&gradient->grad.liner_grad)) {
            band.start.x = fill_aabb.start.x;
            band.start.y = fill_aabb.start.y;
            band.end.x = band.start.x;
            band.end.y = fill_aabb.end.y;
            for (; band.start.x <= fill_aabb.end.x; 
                band.start.x ++, band.end.x = band.start.x) {
                GRAD_COLOR_GET_AND_FILL;
            }
            return;
        } else if (ipgui_if_liner_gradient_ver(&gradient->grad.liner_grad)) {
            band.start.x = fill_aabb.start.x;
            band.start.y = fill_aabb.start.y;
            band.end.x = fill_aabb.end.x;
            band.end.y = band.start.y;
            for (; band.start.y <= fill_aabb.end.y; 
                band.start.y ++, band.end.y = band.start.y) {
                GRAD_COLOR_GET_AND_FILL;
            }
            return;
        }

#undef GRAD_COLOR_GET_AND_FILL
    }

    ipgui_coord_t x, y, x_span, y_span;
    s8_t pix_size;
    s32_t stride;
    u8_t * dest_cr_buf;

    x = fill_aabb.start.x - surf->surf.start.x;
    y = fill_aabb.start.y - surf->surf.start.y;
    dest_cr_buf = ipgui_surf_color_get(surf, x, y);
    x_span = ipgui_aabb_width(&fill_aabb);
    y_span = ipgui_aabb_height(&fill_aabb);
    pix_size = surf->pix_size;
    stride = surf->stride;

    premult_blend_func_t blend_fn = premult_blend_table[surf->pix_fmt];
    if (!blend_fn) return;

    /* 绝对坐标起点（fill_aabb 在 surf 坐标系中的起点） */
    ipgui_coord_t abs_x0 = fill_aabb.start.x;
    ipgui_coord_t abs_y0 = fill_aabb.start.y;

    s32_t row_pix_off;
    u8_t pos;
    ipgui_color_t grad_cr, premult;
    
    for (y = 0; y < y_span; y ++) {
        /* blend the current row */
        row_pix_off = 0;
        for (x = 0; x < x_span; x ++) {

            pos = ipgui_gradient_pos_at_xy(gradient, abs_x0 + x, abs_y0 + y);
            ipgui_gradient_color_get(gradient, pos, &grad_cr);

            /* combine opacity */
            premult = ipgui_color_combine_opacity_and_premultiply(&grad_cr, opacity);
            /* blend pixel */
            if (IPGUI_COLOR_A(premult) > 2) {
                blend_fn(premult, &dest_cr_buf[row_pix_off], blend_mode);
            }

            row_pix_off += pix_size;
        }

        /* go to the next row */
        dest_cr_buf += stride;
    }
}

__IPGUI_API__ void ipgui_blend_gradient_color(
    ipgui_surf_t         * surf,
    ipgui_aabb_t         * clip,
    ipgui_aabb_t         * dest,
    ipgui_grad_src_t     * gradient,
    u8_t                   opacity,
    u8_t                 * mask,      /* mask覆盖mask_aabb区域 */
    ipgui_aabb_t         * mask_aabb, /* mask_aabb必须大于或等于dest区域 */
    ipgui_blend_mode_t     blend_mode)
{
    if ((!dest) || (!surf) || (!gradient) || (opacity < 3))
        return;

    /* 无mask时退化为渐变填充 */
    if (!mask) {
        ipgui_fill_gradient_color(surf, clip, dest, gradient, opacity, blend_mode);
        return;
    }

    if (!mask_aabb)
        return;

    ipgui_aabb_t blend_aabb;
    if (0 != ipgui_aabb_overlap(&blend_aabb, dest, &(surf->surf)))
        return;

    if (clip) {
        if (0 != ipgui_aabb_overlap(&blend_aabb, &blend_aabb, clip))
            return;
    }

    /* 检查线性渐变方向
     * 如果是水平/垂直渐变，退化为分段纯色混合
     */
    if (gradient->grad_type == IPGUI_GRADIENT_TYPE_LINEAR) {
#define GRAD_COLOR_GET_AND_BLEND \
do { \
    ipgui_liner_gradient_color_get(&gradient->grad.liner_grad, \
        ipgui_get_liner_gradient_pos_at_xy(&gradient->grad.liner_grad, \
            band.start.x, band.start.y), &cr); \
    ipgui_blend_color(surf, (ipgui_aabb_t *)0, &band, \
        cr, opacity, mask, mask_aabb, blend_mode); \
} while (0)

        ipgui_color_t cr;
        ipgui_aabb_t band;

        if (ipgui_if_liner_gradient_hor(&gradient->grad.liner_grad)) {
            band.start.x = blend_aabb.start.x;
            band.start.y = blend_aabb.start.y;
            band.end.x = band.start.x;
            band.end.y = blend_aabb.end.y;

            for (; band.start.x <= blend_aabb.end.x;
                 band.start.x ++, band.end.x = band.start.x) {
                GRAD_COLOR_GET_AND_BLEND;
            }
            return;
        } else if (ipgui_if_liner_gradient_ver(&gradient->grad.liner_grad)) {
            band.start.x = blend_aabb.start.x;
            band.start.y = blend_aabb.start.y;
            band.end.x = blend_aabb.end.x;
            band.end.y = band.start.y;

            for (; band.start.y <= blend_aabb.end.y;
                 band.start.y ++, band.end.y = band.start.y) {
                GRAD_COLOR_GET_AND_BLEND;
            }
            return;
        }

#undef GRAD_COLOR_GET_AND_BLEND
    }

    ipgui_coord_t x, y, x_span, y_span;
    s8_t pix_size;
    s32_t stride;
    u8_t * dest_cr_buf;

    x = blend_aabb.start.x - surf->surf.start.x;
    y = blend_aabb.start.y - surf->surf.start.y;
    dest_cr_buf = ipgui_surf_color_get(surf, x, y);
    x_span = ipgui_aabb_width(&blend_aabb);
    y_span = ipgui_aabb_height(&blend_aabb);
    pix_size = surf->pix_size;
    stride = surf->stride;

    ipgui_coord_t mask_stride = ipgui_aabb_width(mask_aabb);
    ipgui_coord_t mask_x0 = blend_aabb.start.x - mask_aabb->start.x;
    ipgui_coord_t mask_y0 = blend_aabb.start.y - mask_aabb->start.y;
    const u8_t * mask_row = mask + mask_y0 * mask_stride + mask_x0;

    premult_blend_func_t blend_fn = premult_blend_table[surf->pix_fmt];
    if (!blend_fn)
        return;

    ipgui_coord_t abs_x0 = blend_aabb.start.x;
    ipgui_coord_t abs_y0 = blend_aabb.start.y;

    ipgui_color_t grad_cr, premult;
    u8_t mask_opacity_combined;
    u8_t mask_val;
    u8_t pos;
    s32_t row_pix_off;

    for (y = 0; y < y_span; y ++) {
        row_pix_off = 0;
        for (x = 0; x < x_span; x ++) {
            mask_val = mask_row[x];

            if (mask_val > 2) {
                pos = ipgui_gradient_pos_at_xy(gradient, abs_x0 + x, abs_y0 + y);
                ipgui_gradient_color_get(gradient, pos, &grad_cr);

                /* mix mask and opacity */
                mask_opacity_combined =
                    (u8_t)(((u32_t)opacity * mask_val + 127) >> 8);

                premult = ipgui_color_combine_opacity_and_premultiply(
                    &grad_cr, mask_opacity_combined);

                if (IPGUI_COLOR_A(premult) > 2) {
                    blend_fn(premult, &dest_cr_buf[row_pix_off], blend_mode);
                }
            }

            row_pix_off += pix_size;
        }

        dest_cr_buf += stride;
        mask_row += mask_stride;
    }
}