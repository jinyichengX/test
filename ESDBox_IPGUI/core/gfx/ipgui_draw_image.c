#include "ipgui_draw_image.h"
#include "ipgui_debug.h"
#include "ipgui_image_buf.h"

/* some fixed macros */
#ifndef IPGUI_FIXED_BITS
#define IPGUI_FIXED_BITS 10
#endif

#ifndef IPGUI_FIXED_PRECI
#define IPGUI_FIXED_PRECI (1 << IPGUI_FIXED_BITS) /* 像素精度1/(1 << 10) = 1/1024 ≈ 0.000976 */
#endif

#ifndef IPGUI_FIXED_MASK
#define IPGUI_FIXED_MASK ((~0U) << IPGUI_FIXED_BITS)
#endif

#ifndef IPGUI_FIXED_HALF
#define IPGUI_FIXED_HALF  (1 << (IPGUI_FIXED_BITS - 1))
#endif

/* 计算定点数前面的整数点（向负无穷方向） */
#define IPGUI_FIXED_FLOOR(fixed_val) ((fixed_val) >> IPGUI_FIXED_BITS)

/* 计算定点数后面的整数点（向正无穷方向） */
#define IPGUI_FIXED_CEIL(fixed_val) ((fixed_val + IPGUI_FIXED_PRECI - 1) >> IPGUI_FIXED_BITS)

/* 扩展为整数包围盒 */
__IPGUI_STATIC__ void ipgui_aabb_expand_integer(ipgui_saabb_t * in, ipgui_aabb_t * out)
{
    out->start.x = IPGUI_FIXED_FLOOR(in->start.x);
    out->start.y = IPGUI_FIXED_FLOOR(in->start.y);
    out->end.x   = IPGUI_FIXED_CEIL(in->end.x);
    out->end.y   = IPGUI_FIXED_CEIL(in->end.y);
}

/* 根据直觉设计画图片的API
 * 指定一个表面surf，然后将图钉钉在图片的某个点pivot 类似于鼠标点住这个点进行拖动，
 * 然后再把图钉钉在surf的某个点anchor（可以在surf之外）类似于鼠标拖动到的点，
 * 然后图片进行变换，如果图片中某个点变换后还在surf中那就在surf中显示。
 */

__IPGUI_API__ void ipgui_draw_image(
    ipgui_surf_t             * surf,
    ipgui_aabb_t             * clip,
    ipgui_image_data_t       * img_data,
    ipgui_point_t            * pivot,    /* 相对于图片的变换点 如果是子图那么就是相对于子图的 */
    ipgui_point_t            * anchor,
    ipgui_trans_mat_t        * trans,
    ipgui_image_draw_style_t * style)
{
    if ((!surf) || (!img_data) || (!pivot) || (!anchor) || (!style))
        return;

    if (style->opacity < 3)
        return;

    if (!trans) {
        ipgui_aabb_t img_aabb;
        img_aabb.start.x = anchor->x - pivot->x;
        img_aabb.start.y = anchor->y - pivot->y;
        img_aabb.end.x   = img_aabb.start.x + img_data->w - 1;
        img_aabb.end.y   = img_aabb.start.y + img_data->h - 1;

        ipgui_image_src_t img_src;
        img_src.buf       = img_data->pixmap;
        img_src.img_pxfmt = img_data->fmt;
        img_src.px_size   = img_data->px_size;
        img_src.stride    = img_data->stride;
        img_src.img_aabb  = &img_aabb;

        ipgui_blend_image_v1(
            surf,
            clip,
            &img_src,
            (ipgui_aabb_t *)0,
            (ipgui_aabb_t *)0,
            style->opacity,
            style->blend_mode);
        return;
    }
    /* draw transformed image */

    /* check if matrix is invertible */
    if (trans->a * trans->d == trans->b * trans->c) {
        ipgui_dbg_warning("warning: the transfrom matrix err\r\n");
        return;
    }

    ipgui_spoint_t p_vec[4]; /* the transformed image's 4 vertexes */
    /* 这四个点是以变换中心为原点 */
    /* screen coordinate
     * p_vec[0]     p_vec[1]
     *
     * p_vec[2]     p_vec[3]
     */
    ipgui_saabb_t aabb_trans;
    ipgui_aabb_t img_aabb;
    for (int i = 0; i < 4; i ++) {
        ipgui_coord_t v_x, v_y;
        switch (i) {
            case 0: v_x = -pivot->x;
                    v_y = -pivot->y;
                    break;
            case 1: v_x = img_data->w - 1 - pivot->x;
                    v_y = -pivot->y;
                    break;
            case 2: v_x = -pivot->x;
                    v_y = img_data->h - 1 - pivot->y;
                    break;
            case 3: v_x = img_data->w - 1 - pivot->x;
                    v_y = img_data->h - 1 - pivot->y;
                    break;
        }
        /* calculate and save it */
        p_vec[i].x = trans->a * v_x + trans->b * v_y;
        p_vec[i].y = trans->c * v_x + trans->d * v_y;
    }
    /* generate aabb(rel to pivot) */
    ipgui_aabb_generate_with_points(&aabb_trans, p_vec, 4);
    ipgui_aabb_expand_integer(&aabb_trans, &img_aabb);

    ipgui_coord_t img_w, img_h;
    img_w = ipgui_aabb_width(&img_aabb);
    img_h = ipgui_aabb_height(&img_aabb);

    /* get image aabb(absolute coordinate on surf) */
    img_aabb.start.x = anchor->x - pivot->x;
    img_aabb.start.y = anchor->y - pivot->y;
    img_aabb.end.x   = img_aabb.start.x + img_w - 1;
    img_aabb.end.y   = img_aabb.start.y + img_h - 1;

    /* clip image aabb */
    ipgui_aabb_t draw;
    if (0 != ipgui_aabb_overlap(&draw, &surf->surf, &img_aabb))
        return;
    if (clip) {
        if (0 != ipgui_aabb_overlap(&draw, &draw, clip))
            return;
    }

    /* draw draw area */
    /* allocate image buffer first */
    ipgui_coord_t res_h = 0;
    u8_t * img_buf;
    img_buf = ipgui_image_buf_acquire(
        (img_data->px_size + 1)/* "+1" means extra byte for mask */ * ipgui_aabb_width(&draw),
        ipgui_aabb_height(&draw),
        &res_h);
    if ((img_buf == (u8_t *)0) || (!res_h))
        return;
    /* get aabb relative to pivot */
    ipgui_aabb_t draw_rel;
    // draw_rel.start.x = 

    /* reverse mapping（反向映射）every pixel */
    


    /* free image buffer */
    ipgui_image_buf_free(img_buf);
}