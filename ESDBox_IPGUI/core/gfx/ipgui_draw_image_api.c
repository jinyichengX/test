#include "ipgui_draw_image_api.h"
#include "ipgui_image_geometry_transform.h"

__IPGUI_API__ void ipgui_draw_image_at(
    ipgui_surf_t                   * surf,
    ipgui_image_data_t             * img,
    ipgui_coord_t                    x,
    ipgui_coord_t                    y,
    const ipgui_image_draw_style_t * style,
    ipgui_image_quality_t            quality)
{
    if (!surf || !img || !style) return;

    ipgui_point_t pivot  = {0, 0};
    ipgui_point_t anchor = {x, y};

    ipgui_draw_image(surf, (ipgui_aabb_t *)0, img,
                     &pivot, &anchor,
                     (ipgui_trans_mat_t *)0,
                     (ipgui_image_draw_style_t *)style,
                      quality);
}

__IPGUI_API__ void ipgui_draw_image_centered(
    ipgui_surf_t                   * surf,
    ipgui_image_data_t             * img,
    ipgui_coord_t                    cx,
    ipgui_coord_t                    cy,
    const ipgui_image_draw_style_t * style,
    ipgui_image_quality_t            quality)
{
    if (!surf || !img || !style) return;

    ipgui_point_t pivot  = {img->w / 2, img->h / 2};
    ipgui_point_t anchor = {cx, cy};

    ipgui_draw_image(surf, (ipgui_aabb_t *)0, img,
                     &pivot, &anchor,
                     (ipgui_trans_mat_t *)0,
                     (ipgui_image_draw_style_t *)style,
                      quality);
}

__IPGUI_STATIC__ void ipgui_compute_align_offset(
    ipgui_image_align_t align,
    ipgui_coord_t       container_w,
    ipgui_coord_t       container_h,
    ipgui_coord_t       item_w,
    ipgui_coord_t       item_h,
    ipgui_coord_t     * ox,
    ipgui_coord_t     * oy)
{
    switch (align) {
        case IPGUI_IMG_ALIGN_TOP_LEFT:     * ox = 0;                           * oy = 0;                           return;
        case IPGUI_IMG_ALIGN_TOP:          * ox = (container_w - item_w) / 2;  * oy = 0;                           return;
        case IPGUI_IMG_ALIGN_TOP_RIGHT:    * ox = container_w - item_w;        * oy = 0;                           return;
        case IPGUI_IMG_ALIGN_LEFT:         * ox = 0;                           * oy = (container_h - item_h) / 2;  return;
        case IPGUI_IMG_ALIGN_CENTER:       * ox = (container_w - item_w) / 2;  * oy = (container_h - item_h) / 2;  return;
        case IPGUI_IMG_ALIGN_RIGHT:        * ox = container_w - item_w;        * oy = (container_h - item_h) / 2;  return;
        case IPGUI_IMG_ALIGN_BOTTOM_LEFT:  * ox = 0;                           * oy = container_h - item_h;        return;
        case IPGUI_IMG_ALIGN_BOTTOM:       * ox = (container_w - item_w) / 2;  * oy = container_h - item_h;        return;
        case IPGUI_IMG_ALIGN_BOTTOM_RIGHT: * ox = container_w - item_w;        * oy = container_h - item_h;        return;
    }
}

__IPGUI_API__ void ipgui_draw_image_in_rect(
    ipgui_surf_t                   * surf,
    ipgui_image_data_t             * img,
    const ipgui_aabb_t             * target,
    ipgui_image_align_t              align,
    ipgui_image_fit_t                fit,
    const ipgui_image_draw_style_t * style,
    ipgui_image_quality_t            quality)
{
    ipgui_coord_t tw, th;
    ipgui_coord_t iw, ih;

    if (!surf || !img || !target || !style) return;

    tw = target->end.x - target->start.x + 1;
    th = target->end.y - target->start.y + 1;
    iw = img->w;
    ih = img->h;

    if (fit == IPGUI_IMG_FIT_NONE) {
        ipgui_coord_t ox, oy;
        ipgui_coord_t draw_w, draw_h;
        ipgui_point_t pivot, anchor;

        draw_w = iw;
        draw_h = ih;
        ipgui_compute_align_offset(align, tw, th, draw_w, draw_h, &ox, &oy);

        pivot.x  = 0;
        pivot.y  = 0;
        anchor.x = target->start.x + ox;
        anchor.y = target->start.y + oy;

        ipgui_draw_image(surf, (ipgui_aabb_t *)target, img,
                         &pivot, &anchor,
                         (ipgui_trans_mat_t *)0,
                         (ipgui_image_draw_style_t *)style,
                          quality);
        return;
    }

    /* 需要缩放变换 */
    {
        ipgui_img_geo_trans_t geo_trans;
        ipgui_coord_t         scaled_w, scaled_h;
        ipgui_coord_t         ox, oy;
        ipgui_point_t         anchor;
        float                 sx, sy;

        ipgui_image_trans_init(&geo_trans);

        if (fit == IPGUI_IMG_FIT_STRETCH) {
            sx = (float)tw / iw;
            sy = (float)th / ih;
            scaled_w = tw;
            scaled_h = th;
        } else {
            s32_t width_is_limiting = ((s64_t)tw * ih <= (s64_t)th * iw);

            if (fit == IPGUI_IMG_FIT_FIT) {
                if (width_is_limiting) {
                    sx = (float)tw / iw;
                    scaled_w = tw;
                    scaled_h = (ipgui_coord_t)(((s64_t)ih * tw + iw / 2) / iw);
                } else {
                    sx = (float)th / ih;
                    scaled_w = (ipgui_coord_t)(((s64_t)iw * th + ih / 2) / ih);
                    scaled_h = th;
                }
            } else { /* IPGUI_IMG_FIT_FILL */
                if (!width_is_limiting) {
                    sx = (float)tw / iw;
                    scaled_w = tw;
                    scaled_h = (ipgui_coord_t)(((s64_t)ih * tw + iw / 2) / iw);
                } else {
                    sx = (float)th / ih;
                    scaled_w = (ipgui_coord_t)(((s64_t)iw * th + ih / 2) / ih);
                    scaled_h = th;
                }
            }
            sy = sx;
        }

        ipgui_image_trans_scale(&geo_trans, sx, sy);

        ipgui_compute_align_offset(align, tw, th, scaled_w, scaled_h, &ox, &oy);

        ipgui_image_trans_pivot(&geo_trans,
            (ipgui_point_t){iw / 2, ih / 2});

        anchor.x = target->start.x + ox + scaled_w / 2;
        anchor.y = target->start.y + oy + scaled_h / 2;

        ipgui_draw_image(surf, (ipgui_aabb_t *)target, img,
                         &geo_trans.pivot, &anchor,
                         (ipgui_trans_mat_t *)&geo_trans.mat,
                         (ipgui_image_draw_style_t *)style,
                          quality);
    }
}
