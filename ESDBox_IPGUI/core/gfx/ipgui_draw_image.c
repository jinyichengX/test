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
#ifndef IPGUI_FIXED_FLOOR
#define IPGUI_FIXED_FLOOR(fixed_val) ((fixed_val) >> IPGUI_FIXED_BITS)
#endif

/* 计算定点数后面的整数点（向正无穷方向） */
#ifndef IPGUI_FIXED_CEIL
#define IPGUI_FIXED_CEIL(fixed_val) ((fixed_val + IPGUI_FIXED_PRECI - 1) >> IPGUI_FIXED_BITS)
#endif

__IPGUI_STATIC__ __IPGUI_INLINE__ ipgui_scoord_t fixed_mult(ipgui_scoord_t a, ipgui_scoord_t b)
{
    s8_t sign = 1;
    ipgui_scoord_t c;
    
    /* 处理符号，转为正数计算 */
    if (a < 0) { a = -a; sign = -sign; }
    if (b < 0) { b = -b; sign = -sign; }
    
    /* 计算：c = (a * b + 0.5) >> IPGUI_FIXED_BITS */
    c = (ipgui_scoord_t)(((s64_t)a * b + IPGUI_FIXED_HALF) >> IPGUI_FIXED_BITS);
    
    return (sign > 0) ? c : -c;
}

__IPGUI_STATIC__ __IPGUI_INLINE__ ipgui_scoord_t fixed_div(ipgui_scoord_t a, ipgui_scoord_t b)
{
    s8_t sign = 1;
    ipgui_scoord_t q;
    
    /* 处理符号，转为正数计算 */
    if (a < 0) { a = -a; sign = -sign; }
    if (b < 0) { b = -b; sign = -sign; }
    
    if (b == 0) return 0x7fffffff;  /* 防除零 */
    
    /* 计算：q = ((a << IPGUI_FIXED_BITS) + b/2) / b */
    q = (ipgui_scoord_t)(((s64_t)a << IPGUI_FIXED_BITS) + (b >> 1)) / b;
    
    return (sign > 0) ? q : -q;
}


__IPGUI_STATIC__ void ipgui_image_trans_matrix_invert(
        ipgui_trans_mat_t * mat,
        ipgui_trans_mat_t * res)
{
    /* 2D matrix A inverted formula
     * if T = |a  b|
     *        |c  d|
     * then T(-1) = 1 / (a*d - b*c) * |d  -b|
     *                                |-c  a|
     */
    ipgui_trans_mat_t temp;
    ipgui_scoord_t det;

    ipgui_scoord_t a, b, c, d;
    a = mat->a;
    b = mat->b;
    c = mat->c;
    d = mat->d;

    det = fixed_mult(a, d) - fixed_mult(b, c);
    temp.a = fixed_div( d, det);
    temp.b = fixed_div(-b, det);
    temp.c = fixed_div(-c, det);
    temp.d = fixed_div( a, det);

    res->a = temp.a;
    res->b = temp.b;
    res->c = temp.c;
    res->d = temp.d;
}


/* 扩展为整数包围盒 */
__IPGUI_STATIC__ void ipgui_aabb_expand_integer(ipgui_saabb_t * in, ipgui_aabb_t * out)
{
    out->start.x = IPGUI_FIXED_FLOOR(in->start.x);
    out->start.y = IPGUI_FIXED_FLOOR(in->start.y);
    out->end.x   = IPGUI_FIXED_CEIL(in->end.x);
    out->end.y   = IPGUI_FIXED_CEIL(in->end.y);
}

/* 求图像变换后的包围盒（相对于pivot） */
__IPGUI_API__ ipgui_aabb_t ipgui_calc_image_transformed_aabb_rel_to_pivot(
    ipgui_point_t     * pivot,    /* 相对于图片(0,0)点的变换点 如果是子图那么就是相对于子图的 */
    ipgui_trans_mat_t * trans,
    ipgui_coord_t       img_w,
    ipgui_coord_t       img_h)
{
    ipgui_spoint_t p_vec[4]; /* the transformed image's 4 vertexes */
    /* 这四个点是以变换中心为原点 */
    /* screen coordinate
     * p_vec[0]     p_vec[1]
     *
     * p_vec[2]     p_vec[3]
     */
    ipgui_saabb_t aabb_trans;
    ipgui_aabb_t img_aabb;
    for (u8_t i = 0; i < 4; i ++) {
        ipgui_coord_t v_x, v_y;
        switch (i) {
            case 0: v_x = -pivot->x;
                    v_y = -pivot->y;
                    break;
            case 1: v_x = img_w - 1 - pivot->x;
                    v_y = -pivot->y;
                    break;
            case 2: v_x = -pivot->x;
                    v_y = img_h - 1 - pivot->y;
                    break;
            case 3: v_x = img_w - 1 - pivot->x;
                    v_y = img_h - 1 - pivot->y;
                    break;
        }
        /* calculate and save it */
        p_vec[i].x = trans->a * v_x + trans->b * v_y;
        p_vec[i].y = trans->c * v_x + trans->d * v_y;
    }
    /* generate aabb(rel to pivot) */
    ipgui_aabb_generate_with_points(&aabb_trans, p_vec, 4);
    ipgui_aabb_expand_integer(&aabb_trans, &img_aabb);

    return img_aabb;
}

/* get image aabb(absolute coordinate on surf) */
__IPGUI_API__ ipgui_aabb_t ipgui_locate_image(
    ipgui_point_t * pivot,    /* 相对于图片的变换点 如果是子图那么就是相对于子图的 */
    ipgui_point_t * anchor,
    ipgui_coord_t   img_w,
    ipgui_coord_t   img_h)
{
    ipgui_aabb_t img_aabb;
    img_aabb.start.x = anchor->x - pivot->x;
    img_aabb.start.y = anchor->y - pivot->y;
    img_aabb.end.x   = img_aabb.start.x + img_w - 1;
    img_aabb.end.y   = img_aabb.start.y + img_h - 1;

    return img_aabb;
}

/* 求图像变换后的包围盒（相对于pivot） */
__IPGUI_API__ ipgui_aabb_t ipgui_calc_image_transformed_aabb(
    ipgui_point_t     * pivot,    /* 相对于图片(0,0)点的变换点 如果是子图那么就是相对于子图的 */
    ipgui_point_t     * anchor,
    ipgui_trans_mat_t * trans,
    ipgui_coord_t       img_w,
    ipgui_coord_t       img_h)
{
    ipgui_aabb_t img_aabb;
    img_aabb = ipgui_calc_image_transformed_aabb_rel_to_pivot(
                        pivot,
                        trans,
                        img_w,
                        img_h);

    ipgui_coord_t w, h;
    w = ipgui_aabb_width (&img_aabb);
    h = ipgui_aabb_height(&img_aabb);

    /* get image aabb(absolute coordinate on surf) */
    return ipgui_locate_image(pivot, anchor, w, h);
}

__IPGUI_STATIC__ __IPGUI_INLINE__ u8_t * image_pixmap_get(ipgui_image_data_t * img_data, ipgui_coord_t x/* rel to image's(0,0) */, ipgui_coord_t y)
{
    return img_data->pixmap + (img_data->stride * y) + img_data->fmt * x;
}

__IPGUI_STATIC__ __IPGUI_INLINE__ void ipgui_biliner_core_4ch(
    u8_t * a, u8_t * b, u8_t * c, u8_t * d,
    u8_t hd, u8_t vd,
    u8_t * out)  // out[0]=R, out[1]=G, out[2]=B, out[3]=A
{
    u32_t w1 = (255 - hd) * (255 - vd);
    u32_t w2 = hd * (255 - vd);
    u32_t w3 = vd * (255 - hd);
    u32_t w4 = hd * vd;
    
    for (u8_t i = 0; i < 4; i++) {
        out[i] = (a[i] * w1 + b[i] * w2 + c[i] * w3 + d[i] * w4 + 32768) >> 16;
    }
}

/* linear interpolation algorithm */
__IPGUI_STATIC__ __IPGUI_INLINE__ u8_t ipgui_liner_core(u8_t a, u8_t b, u8_t d) /* d is scaled 255 (0.0 ~ 1.0 ---> 0 ~ 255)*/
{
    /* the five points p,a,b position like below
     * a--d--p       b
     */
    return (u8_t)(((255 - d) * a + b * d + 128) >> 8);
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
        /* get image aabb(absolute coordinate on surf) */
        img_aabb = ipgui_locate_image(
            pivot,
            anchor,
            img_data->w,
            img_data->h);

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
        ipgui_dbg_warning("warning: the transfrom matrix err, because it is 不可逆的\r\n");
        return;
    }

    ipgui_aabb_t img_rel_pivot;
    ipgui_aabb_t img_in_surf;
    ipgui_coord_t img_w, img_h;
    img_rel_pivot = ipgui_calc_image_transformed_aabb_rel_to_pivot(pivot, trans, img_data->w, img_data->h);
    img_w = ipgui_aabb_width (&img_rel_pivot);
    img_h = ipgui_aabb_height(&img_rel_pivot);

    /* get image aabb(absolute coordinate on surf) */
    img_in_surf = ipgui_locate_image(pivot, anchor, img_w, img_h);

    /* calc the aabb need to be drawn */
    ipgui_aabb_t  draw_in_surf;
    ipgui_coord_t draw_w, draw_h;
    if (0 != ipgui_aabb_overlap(&draw_in_surf, &surf->surf, &img_in_surf))
        return;
    if (clip) {
        if (0 != ipgui_aabb_overlap(&draw_in_surf, &draw_in_surf, clip))
            return;
    }
    draw_w = ipgui_aabb_width (&draw_in_surf);
    draw_h = ipgui_aabb_height(&draw_in_surf);

    /* draw draw area */
    /* allocate image buffer first */
    ipgui_coord_t res_h = 0;
    u8_t * pixmap;
    u8_t * mask_buf;
    pixmap = ipgui_image_buf_acquire(
        (img_data->px_size + 1)/* "+1" means extra byte for mask */ * draw_w,
        1,/* render line by line */
        &res_h);
    if ((pixmap == (u8_t *)0) || (!res_h))
        return;
    mask_buf = pixmap + img_data->px_size * draw_w * res_h; /* mask buffer is after pixel buffer */
    
    /* get aabb relative to pivot */
    ipgui_aabb_t draw_rel_pivot;
    draw_rel_pivot.start.x = img_rel_pivot.start.x + (draw_in_surf.start.x - img_in_surf.start.x);
    draw_rel_pivot.start.y = img_rel_pivot.start.y + (draw_in_surf.start.y - img_in_surf.start.y);
    draw_rel_pivot.end.x   = draw_rel_pivot.start.x + draw_w - 1;
    draw_rel_pivot.end.y   = draw_rel_pivot.start.y + draw_h - 1;

    u8_t * cr_a, * cr_b, * cr_c, * cr_d;/* 插值点周围4点颜色值索引 */
    ipgui_coord_t temp_x, temp_y;
    s32_t alpha, d;/* for edge mask */
    u8_t a, r, g, b;
    u8_t cr[4];
    u8_t hd, vd;
    s32_t idx = 0, shift = 8 - IPGUI_FIXED_BITS;

    u32_t px_sz  = img_data->px_size; /* per pixel size */
    u32_t stride = img_data->stride;

    ipgui_scoord_t ori_dx, ori_dy;
    ori_dx = pivot->x * IPGUI_FIXED_PRECI;
    ori_dy = pivot->y * IPGUI_FIXED_PRECI;
    ipgui_coord_t x, y;
    ipgui_scoord_t xo, yo; /* the transformed xy and orig's xy */

    ipgui_scoord_t border_x     = (img_data->w  - 1) * IPGUI_FIXED_PRECI;
    ipgui_scoord_t border_y     = (img_data->h  - 1) * IPGUI_FIXED_PRECI;
    ipgui_scoord_t max_border_x = border_x + IPGUI_FIXED_PRECI;
    ipgui_scoord_t max_border_y = border_y + IPGUI_FIXED_PRECI;

    ipgui_scoord_t xo_start_row, yo_start_row; /* the transformed xy and orig's xy */
    ipgui_scoord_t next_pix_incx/* inv.a */, next_pix_incy/* inv.c */;
    ipgui_scoord_t next_row_incx/* inv.b */, next_row_incy/* inv.d */;
    ipgui_trans_mat_t inv;
    ipgui_image_trans_matrix_invert(trans, &inv); 
    next_pix_incx = inv.a;
    next_pix_incy = inv.c;
    next_row_incx = inv.b;
    next_row_incy = inv.d;
    x = draw_rel_pivot.start.x;
    y = draw_rel_pivot.start.y;

    xo_start_row = inv.a * x + inv.b * y + ori_dx;
    yo_start_row = inv.c * x + inv.d * y + ori_dy;

    /* reverse mapping（反向映射）every pixel */
    for (; y <= draw_rel_pivot.end.y; y ++) { /* xy是变换后的图像点相对于变换点的deltax和deltay */
        x = draw_rel_pivot.start.x;
        xo = xo_start_row;
        yo = yo_start_row;
        for (; x <= draw_rel_pivot.end.x; x ++) {
            /* check if in the original image's aabb */
            if ((xo <= -IPGUI_FIXED_PRECI) || (xo >= max_border_x)) {
                mask_buf[idx ++] = 0;
                goto _next_pix_inc;
            }
            if ((yo <= -IPGUI_FIXED_PRECI) || (yo >= max_border_y)) {
                mask_buf[idx ++] = 0;
                goto _next_pix_inc;
            }

            if (((xo >= 0) && (xo <= border_x)) \
             && ((yo >= 0) && (yo <= border_y))) {
                hd = xo & (~IPGUI_FIXED_MASK);
                vd = yo & (~IPGUI_FIXED_MASK);
                /* scale to 0-255 */
                if (shift > 0) {
                    hd = hd << shift;
                    vd = vd << shift;
                } else if (shift < 0) {
                    hd = hd >> (-shift);
                    vd = vd >> (-shift);
                }

                /* left top point(a) coordinate */
                temp_x = IPGUI_FIXED_FLOOR(xo);
                temp_y = IPGUI_FIXED_FLOOR(yo);

                /* src color */
                cr_a = image_pixmap_get(img_data, temp_x, temp_y); /* the top left point */
                cr_b = cr_a + px_sz;
                cr_c = cr_a + stride;
                cr_d = cr_c + px_sz;

                /* dst color */
                // a = ipgui_biliner_core(cr_a[3], cr_b[3], cr_c[3], cr_d[3], hd, vd);
                // b = ipgui_biliner_core(cr_a[0], cr_b[0], cr_c[0], cr_d[0], hd, vd);
                // g = ipgui_biliner_core(cr_a[1], cr_b[1], cr_c[1], cr_d[1], hd, vd);
                // r = ipgui_biliner_core(cr_a[2], cr_b[2], cr_c[2], cr_d[2], hd, vd);
                ipgui_biliner_core_4ch(cr_a, cr_b, cr_c, cr_d, hd, vd, cr);

                /* write to pixel buffer */
                pixmap[idx * px_sz] = cr[2]; /* need to modify code here like set_image_pix(color_t color, coord_t index) */
                pixmap[idx * px_sz + 1] = cr[1];
                pixmap[idx * px_sz + 2] = cr[0];
                mask_buf[idx ++] = 255;
            } else {                /* generate edge mask */
                if (xo < 0) { /* left edge */
                    if (yo < 0) {
                        d = 0;
                        alpha = IPGUI_MIN((0 - xo), (0 - yo));
                        cr_a = cr_b = image_pixmap_get(img_data, 0, 0);
                    } else if (yo > border_y) {
                        d = 0;
                        alpha = IPGUI_MIN((0 - xo), (yo & (~IPGUI_FIXED_MASK)));
                        cr_a = cr_b = image_pixmap_get(img_data, 0, img_data->h - 1);
                    } else {
                        alpha = 0 - xo;
                        temp_x = IPGUI_FIXED_CEIL(xo);
                        temp_y = IPGUI_FIXED_FLOOR(yo);
                        /* get src color */
                        cr_a = image_pixmap_get(img_data, temp_x, temp_y);
                        cr_b = cr_a + stride;
                        d = (yo - (yo & IPGUI_FIXED_MASK));
                    }
                } else if (xo > border_x) { /* right edge */
                    if (yo < 0) {
                        d = 0;
                        alpha = IPGUI_MIN((xo & (~IPGUI_FIXED_MASK)), (0 - yo));
                        cr_a = cr_b = image_pixmap_get(img_data, img_data->w - 1, 0);
                    } else if (yo > border_y) {
                        d = 0;
                        alpha = IPGUI_MIN((xo & (~IPGUI_FIXED_MASK)), (yo & (~IPGUI_FIXED_MASK)));
                        cr_a = cr_b = image_pixmap_get(img_data, img_data->w - 1, img_data->h - 1);
                    } else {
                        alpha = xo - border_x;
                        temp_x = IPGUI_FIXED_FLOOR(xo);
                        temp_y = IPGUI_FIXED_FLOOR(yo);
                        /* get src color */
                        cr_a = image_pixmap_get(img_data, temp_x, temp_y);
                        cr_b = cr_a + stride;
                        d = (yo - (yo & IPGUI_FIXED_MASK));
                    }
                } else if (yo < 0) { /* top edge */
                    alpha = 0 - yo;
                    temp_x = IPGUI_FIXED_FLOOR(xo);
                    temp_y = IPGUI_FIXED_CEIL(yo);
                    /* get src color */
                    cr_a = image_pixmap_get(img_data, temp_x, temp_y);
                    cr_b = cr_a + px_sz;
                    d = (xo - (xo & IPGUI_FIXED_MASK));
                } else if (yo > border_y) { /* bottom edge */
                    alpha = yo - border_y;
                    temp_x = IPGUI_FIXED_FLOOR(xo);
                    temp_y = IPGUI_FIXED_FLOOR(yo);
                    /* get src color */
                    cr_a = image_pixmap_get(img_data, temp_x, temp_y);
                    cr_b = cr_a + px_sz;
                    d = (xo - (xo & IPGUI_FIXED_MASK));
                }

                /* pre-handle alpha and d */
                alpha = IPGUI_FIXED_PRECI - alpha;
                if (shift > 0) {
                    alpha = alpha << shift;
                    d = d << shift;
                } else if (shift < 0) {
                    alpha = alpha >> (-shift);
                    d = d >> (-shift);
                }

                /* dst color */
                a = ipgui_liner_core(cr_a[3], cr_b[3], d);
                b = ipgui_liner_core(cr_a[0], cr_b[0], d);
                g = ipgui_liner_core(cr_a[1], cr_b[1], d);
                r = ipgui_liner_core(cr_a[2], cr_b[2], d);

                /* write to pixel buffer */
                pixmap[idx * px_sz] = r; /* need to modify code here like set_image_pix(color_t color, coord_t index) */
                pixmap[idx * px_sz + 1] = g;
                pixmap[idx * px_sz + 2] = b;

                mask_buf[idx ++] = alpha;
            }

_next_pix_inc:
            xo += next_pix_incx;/* inv.a */
            yo += next_pix_incy;/* inv.c */
        }
        xo_start_row += next_row_incx;/* inv.b */
        yo_start_row += next_row_incy;/* inv.d */
    }


    /* free image buffer */
    ipgui_image_buf_free(pixmap);
}