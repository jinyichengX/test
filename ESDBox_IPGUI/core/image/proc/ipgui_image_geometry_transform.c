#include "ipgui_image_geometry_transform.h"
#include "ipgui_memory.h"
#include "ipgui_debug.h"

/*   a image's coordinate
 * 
 *   · <--- it is image's pivot(it is a relative coordinate, related to image's (0,0))
 *    
 *    
 *    (0,0)                (w - 1, 0)
 * 
 * 
 * 
 * 
 *    (0, h - 1)           (w - 1, h - 1)
 *
 */
#define DONOT_USE_OPTIM 0/* 不使用优化，置0即可，保留优化前的代码是为了理解原始思路 */

/* some fixed macros */
#define IPGUI_FIXED_BITS 10
#define IPGUI_FIXED_PRECI (1 << IPGUI_FIXED_BITS) /* 像素精度1/(1 << 10) = 1/1024 ≈ 0.000976 */
#define IPGUI_FIXED_MASK ((~0U) << IPGUI_FIXED_BITS)
#define IPGUI_FIXED_HALF  (1 << (IPGUI_FIXED_BITS - 1))

__IPGUI_STATIC__ __IPGUI_INLINE__ ipgui_scoord_t fixed_mult(ipgui_scoord_t a, ipgui_scoord_t b)
{
    int sign = 1;
    ipgui_scoord_t c;
    
    /* 处理符号，转为正数计算 */
    if (a < 0) { a = -a; sign = -sign; }
    if (b < 0) { b = -b; sign = -sign; }
    
    /* 计算：c = (a * b + 0.5) >> IPGUI_FIXED_BITS */
    c = (ipgui_scoord_t)(((long long)a * b + IPGUI_FIXED_HALF) >> IPGUI_FIXED_BITS);
    
    return (sign > 0) ? c : -c;
}

__IPGUI_STATIC__ __IPGUI_INLINE__ ipgui_scoord_t fixed_div(ipgui_scoord_t a, ipgui_scoord_t b)
{
    int sign = 1;
    ipgui_scoord_t q;
    
    /* 处理符号，转为正数计算 */
    if (a < 0) { a = -a; sign = -sign; }
    if (b < 0) { b = -b; sign = -sign; }
    
    if (b == 0) return 0x7FFFFFFF;  /* 防除零 */
    
    /* 计算：q = ((a << IPGUI_FIXED_BITS) + b/2) / b */
    q = (ipgui_scoord_t)(((long long)a << IPGUI_FIXED_BITS) + (b >> 1)) / b;
    
    return (sign > 0) ? q : -q;
}

__IPGUI_API__ void ipgui_image_trans_init(ipgui_img_geo_trans_t * trans)
{
    trans->pivot.x = 0;
    trans->pivot.y = 0;
    trans->mat.a = IPGUI_FIXED_PRECI;
    trans->mat.b = 0;
    trans->mat.c = 0;
    trans->mat.d = IPGUI_FIXED_PRECI;
}

__IPGUI_API__ void ipgui_image_trans_pivot(ipgui_img_geo_trans_t * trans, ipgui_point_t pp)
{
    trans->pivot = pp;
}

/* vector P and transform matrix T1 and T2
 * P1 = T1·P
 * P2 = T2·P1
 * then P2 = T2·T1·P
*/

__IPGUI_API__ void ipgui_image_trans_scale(ipgui_img_geo_trans_t * trans, float sx, float sy)
{
    /* T = |sx  0 |
     *     |0   sy|
     */
    ipgui_scoord_t fsx, fsy;
    ipgui_scoord_t a, b, c, d;

    fsx = (ipgui_scoord_t)(sx * (float)IPGUI_FIXED_PRECI);
    fsy = (ipgui_scoord_t)(sy * (float)IPGUI_FIXED_PRECI);
    a = trans->mat.a;
    b = trans->mat.b;
    c = trans->mat.c;
    d = trans->mat.d;

    trans->mat.a = fixed_mult(a, fsx);
    trans->mat.b = fixed_mult(b, fsx);
    trans->mat.c = fixed_mult(c, fsy);
    trans->mat.d = fixed_mult(d, fsy);
}

__IPGUI_API__ void ipgui_image_trans_rotate_degree(ipgui_img_geo_trans_t * trans, int deg/* 0-360 is a period */)
{
    /* T = |cos(θ)  -sin(θ)|
     *     |sin(θ)   cos(θ)|
     */
    ipgui_scoord_t a, b, c, d;
    a = trans->mat.a;
    b = trans->mat.b;
    c = trans->mat.c;
    d = trans->mat.d;
    // deg = -deg; /* convert to screen coordinate */
    trans->mat.a = ((a * ipgui_cos(deg) - c * ipgui_sin(deg)) + 16384) / 32768;
    trans->mat.b = ((b * ipgui_cos(deg) - d * ipgui_sin(deg)) + 16384) / 32768;
    trans->mat.c = ((a * ipgui_sin(deg) + c * ipgui_cos(deg)) + 16384) / 32768;
    trans->mat.d = ((b * ipgui_sin(deg) + d * ipgui_cos(deg)) + 16384) / 32768;
}

/* 上下翻转 */
__IPGUI_API__ void ipgui_image_trans_reflect_y(ipgui_img_geo_trans_t * trans)
{
    /* T = |1  0 |
     *     |0  -1|
     */
    ipgui_scoord_t c, d;
    c = trans->mat.c;
    d = trans->mat.d;

    trans->mat.c = -c;
    trans->mat.d = -d;
}

/* 左右翻转 */
__IPGUI_API__ void ipgui_image_trans_reflect_x(ipgui_img_geo_trans_t * trans)
{
    /* T = |-1  0|
     *     |0   1|
     */
    ipgui_scoord_t a, b;
    a = trans->mat.a;
    b = trans->mat.b;

    trans->mat.a = -a;
    trans->mat.b = -b;
}

__IPGUI_API__ void ipgui_image_trans_shear(ipgui_img_geo_trans_t * trans, float sx, float sy)
{
    /* not support now */
}

__IPGUI_STATIC__ void ipgui_image_trans_matrix_invert(
        ipgui_img_geo_trans_mat_t * mat,
        ipgui_img_geo_trans_mat_t * res)
{
    /* 2D matrix A inverted formula
     * if T = |a  b|
     *        |c  d|
     * then T(-1) = 1 / (a*d - b*c) * |d  -b|
     *                                |-c  a|
     */
    ipgui_img_geo_trans_mat_t temp;
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

/* bilinear interpolation algorithm */
__IPGUI_STATIC__ __IPGUI_INLINE__ unsigned char ipgui_biliner_core(unsigned char a, unsigned char b, unsigned char c, unsigned char d, 
    unsigned char hd, unsigned char vd) /* hd and vd is scaled 255 (0.0 ~ 1.0 ---> 0 ~ 255)*/
{
    /* the five points p,a,b,c,d position like below
     * a--hd--     b
     * |      |
     * vd     |
     * |      |
     *  ----- p
     * 
     * c           d
     */
    return (unsigned char)(((a * (255 - hd) * (255 - vd) + b * hd * (255 - vd) +
			 c * vd * (255 - hd) + d * hd * vd) + 32768) >> 16);
}

__IPGUI_STATIC__ __IPGUI_INLINE__ void ipgui_biliner_core_4ch(
    unsigned char * a, unsigned char * b, unsigned char * c, unsigned char * d,
    unsigned char hd, unsigned char vd,
    unsigned char * out)  // out[0]=R, out[1]=G, out[2]=B, out[3]=A
{
    unsigned int w1 = (255 - hd) * (255 - vd);
    unsigned int w2 = hd * (255 - vd);
    unsigned int w3 = vd * (255 - hd);
    unsigned int w4 = hd * vd;
    
    for (int i = 0; i < 4; i++) {
        out[i] = (a[i] * w1 + b[i] * w2 + c[i] * w3 + d[i] * w4 + 32768) >> 16;
    }
}

/* linear interpolation algorithm */
__IPGUI_STATIC__ __IPGUI_INLINE__ unsigned char ipgui_liner_core(unsigned char a, unsigned char b, unsigned char d) /* d is scaled 255 (0.0 ~ 1.0 ---> 0 ~ 255)*/
{
    /* the five points p,a,b position like below
     * a--d--p       b
     */
    return (unsigned char)(((255 - d) * a + b * d + 128) >> 8);
}

/* 计算定点数前面的整数点（向负无穷方向） */
#define IPGUI_FIXED_FLOOR(fixed_val) ((fixed_val) >> IPGUI_FIXED_BITS)

/* 计算定点数后面的整数点（向正无穷方向） */
#define IPGUI_FIXED_CEIL(fixed_val) ((fixed_val + IPGUI_FIXED_PRECI - 1) >> IPGUI_FIXED_BITS )

/* 扩展为整数包围盒 */
__IPGUI_STATIC__ void ipgui_aabb_expand_integer(ipgui_saabb_t * in, ipgui_aabb_t * out)
{
    out->start.x = IPGUI_FIXED_FLOOR(in->start.x);
    out->start.y = IPGUI_FIXED_FLOOR(in->start.y);
    out->end.x = IPGUI_FIXED_CEIL(in->end.x);
    out->end.y = IPGUI_FIXED_CEIL(in->end.y);
}

__IPGUI_STATIC__ __IPGUI_INLINE__ unsigned char * image_pixmap_get(ipgui_img_dsc_t * image, ipgui_coord_t x/* rel to image's(0,0) */, ipgui_coord_t y)
{
    return image->pixmap + (image->stride * y) + image->fmt * x;
}

/* 此API是将图像变换到一块离屏缓冲上（再由光栅化器渲染出来） */
__IPGUI_API__ ipgui_img_dsc_t * ipgui_image_geo_transform(
    ipgui_img_dsc_t * image,
    ipgui_img_geo_trans_t * trans,
    ipgui_lerp_method_t lerp)
{
    /* check if matrix is invertible */
    if (trans->mat.a * trans->mat.d == trans->mat.b * trans->mat.c) {
        ipgui_dbg_warning("warning: the transfrom matrix err\r\n");
        return (ipgui_img_dsc_t *)0;
    }
    
    ipgui_scoord_t ori_dx, ori_dy;
    ori_dx = trans->pivot.x * IPGUI_FIXED_PRECI;
    ori_dy = trans->pivot.y * IPGUI_FIXED_PRECI;
    ipgui_coord_t x, y;
    ipgui_scoord_t xo, yo; /* the transformed xy and orig's xy */
    ipgui_coord_t w_trans, h_trans; /* transformed image's width and height and orig's w and h */
    int px_sz = (int)image->fmt; /* per pixel size */
    int stride = image->stride;
    ipgui_spoint_t p_vec[4]; /* the transformed image's 4 vertexes */
    /* 这四个点是以变换中心为原点 */
    /* screen coordinate
     * p_vec[0]     p_vec[1]
     *
     * p_vec[2]     p_vec[3]
     */
    ipgui_saabb_t aabb_trans;
    ipgui_aabb_t aabb;
    for (int i = 0; i < 4; i ++) {
        ipgui_coord_t v_x, v_y;
        switch (i) {
            case 0: v_x = -trans->pivot.x;
                    v_y = -trans->pivot.y;
                    break;
            case 1: v_x = image->w - 1 - trans->pivot.x;
                    v_y = -trans->pivot.y;
                    break;
            case 2: v_x = -trans->pivot.x;
                    v_y = image->h - 1 - trans->pivot.y;
                    break;
            case 3: v_x = image->w - 1 - trans->pivot.x;
                    v_y = image->h - 1 - trans->pivot.y;
                    break;
        }
        /* calculate and save it */
        p_vec[i].x = trans->mat.a * v_x + trans->mat.b * v_y;
        p_vec[i].y = trans->mat.c * v_x + trans->mat.d * v_y;
    }
    /* generate aabb(rel to pivot) */
    ipgui_aabb_generate_with_points(&aabb_trans, p_vec, 4);
    ipgui_aabb_expand_integer(&aabb_trans, &aabb);

    w_trans = ipgui_aabb_width(&aabb);
    h_trans = ipgui_aabb_height(&aabb);

    /* 创建一块离屏缓冲区 */
    ipgui_img_dsc_t * r_img;
    r_img = ipgui_mem_alloc_def(sizeof(ipgui_img_dsc_t) + (w_trans * h_trans) * (px_sz + 1));/* 加1是分配mask */
    if (!r_img) return r_img;

    /* de-transform */
    unsigned char a, r, g, b;
    unsigned char cr[4];
    unsigned char hd, vd;

    int idx = 0, shift = 8 - IPGUI_FIXED_BITS;
    ipgui_scoord_t border_x = (image->w  - 1) * IPGUI_FIXED_PRECI;
    ipgui_scoord_t border_y = (image->h  - 1) * IPGUI_FIXED_PRECI;
    ipgui_scoord_t max_border_x = border_x + IPGUI_FIXED_PRECI;
    ipgui_scoord_t max_border_y = border_y + IPGUI_FIXED_PRECI;

    unsigned char * pixmap = (unsigned char *)r_img + sizeof(ipgui_img_dsc_t); /* 指向pixmap */
    unsigned char * mask_buf = pixmap + (w_trans * h_trans) * px_sz; /* 指向mask */
    unsigned char * cr_a, * cr_b, * cr_c, * cr_d;/* 插值点周围4点颜色值索引 */
    ipgui_coord_t temp_x, temp_y;
    int alpha, d;/* for edge mask */
#if DONOT_USE_OPTIM == 1 
    /* 原理版本，这里算逆矩阵用来逆变换会与上面p_vec的计算数学关系不一致（保留这部分，原理版本可用于对比参考，以便理解优化版本） */
    ipgui_img_geo_trans_mat_t inv;
    ipgui_image_trans_matrix_invert(&trans->mat, &inv); 
    y = aabb.start.y;
    x = aabb.start.x;
#else
    /* 优化版本 */
    ipgui_scoord_t xo_start_row, yo_start_row; /* the transformed xy and orig's xy */
    ipgui_scoord_t next_pix_incx/* inv.a */, next_pix_incy/* inv.c */;
    ipgui_scoord_t next_row_incx/* inv.b */, next_row_incy/* inv.d */;

    y = aabb.start.y;
    x = aabb.start.x;

#if 1
    ipgui_img_geo_trans_mat_t inv;
    ipgui_image_trans_matrix_invert(&trans->mat, &inv); 
    next_pix_incx = inv.a;
    next_pix_incy = inv.c;
    next_row_incx = inv.b;
    next_row_incy = inv.d;
    xo_start_row = inv.a * x + inv.b * y + ori_dx;
    yo_start_row = inv.c * x + inv.d * y + ori_dy;
#else
    int frac_a, frac_b, frac_c, frac_d;
    int det, temp, _int, _frac;
    det = trans->mat.a * trans->mat.d - trans->mat.b * trans->mat.c;

    /* xo_start_row = inv.a * x + inv.b * y + ori_dx;分解为下面三步 */
    temp = trans->mat.d * IPGUI_FIXED_PRECI; /* 分子 */
    _int = temp / det;/* inv.a的整数部分 */
    _frac = temp % det;/* inv.a的小数部分 */
    next_pix_incx = _int * IPGUI_FIXED_PRECI + (int)((long long)((long long)_frac * IPGUI_FIXED_PRECI) / det); /* this is inv.a */
    frac_a = (int)((long long)((long long)_frac * IPGUI_FIXED_PRECI) % det);
    xo_start_row = _int * x * IPGUI_FIXED_PRECI + (int)((long long)((long long)_frac * x * IPGUI_FIXED_PRECI) / det);/* + inv.a * x */

    temp = -trans->mat.b * IPGUI_FIXED_PRECI; /* 分子 */
    _int = temp / det;/* inv.b的整数部分 */
    _frac = temp % det;/* inv.b的小数部分 */
    next_row_incx = _int * IPGUI_FIXED_PRECI + (int)((long long)((long long)_frac * IPGUI_FIXED_PRECI) / det); /* this is inv.b */
    frac_b = (int)((long long)((long long)_frac * IPGUI_FIXED_PRECI) % det);
    xo_start_row += (_int * y * IPGUI_FIXED_PRECI + (int)((long long)((long long)_frac * y * IPGUI_FIXED_PRECI) / det));/* + inv.b * y */

    xo_start_row += ori_dx;

    /* yo_start_row = inv.c * x + inv.d * y + ori_dy;分解为下面三步 */
    temp = -trans->mat.c * IPGUI_FIXED_PRECI; /* 分子 */
    _int = temp / det;/* inv.c的整数部分 */
    _frac = temp % det;/* inv.c的小数部分 */
    next_pix_incy = _int * IPGUI_FIXED_PRECI + (int)((long long)((long long)_frac * IPGUI_FIXED_PRECI) / det); /* this is inv.c */
    frac_c = (int)((long long)((long long)_frac * IPGUI_FIXED_PRECI) % det);
    yo_start_row = _int * x * IPGUI_FIXED_PRECI + (int)((long long)((long long)_frac * x * IPGUI_FIXED_PRECI) / det);/* + inv.c * x */

    temp = trans->mat.a * IPGUI_FIXED_PRECI; /* 分子 */
    _int = temp / det;/* inv.d的整数部分 */
    _frac = temp % det;/* inv.d的小数部分 */
    next_row_incy = _int * IPGUI_FIXED_PRECI + (int)((long long)((long long)_frac * IPGUI_FIXED_PRECI) / det); /* this is inv.d */
    frac_d = (int)((long long)((long long)_frac * IPGUI_FIXED_PRECI) % det);
    yo_start_row += (_int * y * IPGUI_FIXED_PRECI + (int)((long long)((long long)_frac * y * IPGUI_FIXED_PRECI) / det));/* + inv.d * y */
    
    yo_start_row += ori_dy;
#endif
#endif
    for (; y <= aabb.end.y; y ++) { /* xy是变换后的图像点相对于变换点的deltax和deltay */
        x = aabb.start.x;
#if DONOT_USE_OPTIM == 0
        xo = xo_start_row;
        yo = yo_start_row;
#endif
        for (; x <= aabb.end.x; x ++) {
#if DONOT_USE_OPTIM == 1   
            /* 原理版本（保留这部分，原理版本可用于对比参考，以便理解优化版本） */
            xo = inv.a * x + inv.b * y;
            yo = inv.c * x + inv.d * y;

            xo += ori_dx;
            yo += ori_dy;
#endif
            /* check if in the original image's aabb */
            if ((xo <= -IPGUI_FIXED_PRECI) || (xo >= max_border_x)) {
                mask_buf[idx ++] = 0;
#if DONOT_USE_OPTIM == 0
                goto _next_pix_inc;
#else
                continue;
#endif
            }
            if ((yo <= -IPGUI_FIXED_PRECI) || (yo >= max_border_y)) {
                mask_buf[idx ++] = 0;
#if DONOT_USE_OPTIM == 0
                goto _next_pix_inc;
#else
                continue;
#endif
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
                cr_a = image_pixmap_get(image, temp_x, temp_y); /* the top left point */
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
#if DONOT_USE_OPTIM == 0
                        d = 0;
                        alpha = IPGUI_MIN((0 - xo), (0 - yo));
                        cr_a = cr_b = image_pixmap_get(image, 0, 0);
#else
                        mask_buf[idx ++] = 0;
                        continue;
#endif
                    } else if (yo > border_y) {
#if DONOT_USE_OPTIM == 0
                        d = 0;
                        alpha = IPGUI_MIN((0 - xo), (yo & (~IPGUI_FIXED_MASK)));
                        cr_a = cr_b = image_pixmap_get(image, 0, image->h - 1);
#else
                        mask_buf[idx ++] = 0;
                        continue;
#endif     
                    } else {
                        alpha = 0 - xo;
                        temp_x = IPGUI_FIXED_CEIL(xo);
                        temp_y = IPGUI_FIXED_FLOOR(yo);
                        /* get src color */
                        cr_a = image_pixmap_get(image, temp_x, temp_y);
                        cr_b = cr_a + stride;
                        d = (yo - (yo & IPGUI_FIXED_MASK));
                    }
                } else if (xo > border_x) { /* right edge */
                    if (yo < 0) {
#if DONOT_USE_OPTIM == 0
                        d = 0;
                        alpha = IPGUI_MIN((xo & (~IPGUI_FIXED_MASK)), (0 - yo));
                        cr_a = cr_b = image_pixmap_get(image, image->w - 1, 0);
#else
                        mask_buf[idx ++] = 0;
                        continue;
#endif 
                    } else if (yo > border_y) {
#if DONOT_USE_OPTIM == 0
                        d = 0;
                        alpha = IPGUI_MIN((xo & (~IPGUI_FIXED_MASK)), (yo & (~IPGUI_FIXED_MASK)));
                        cr_a = cr_b = image_pixmap_get(image, image->w - 1, image->h - 1);
#else
                        mask_buf[idx ++] = 0;
                        continue;
#endif 
                    } else {
                        alpha = xo - border_x;
                        temp_x = IPGUI_FIXED_FLOOR(xo);
                        temp_y = IPGUI_FIXED_FLOOR(yo);
                        /* get src color */
                        cr_a = image_pixmap_get(image, temp_x, temp_y);
                        cr_b = cr_a + stride;
                        d = (yo - (yo & IPGUI_FIXED_MASK));
                    }
                } else if (yo < 0) { /* top edge */
                    alpha = 0 - yo;
                    temp_x = IPGUI_FIXED_FLOOR(xo);
                    temp_y = IPGUI_FIXED_CEIL(yo);
                    /* get src color */
                    cr_a = image_pixmap_get(image, temp_x, temp_y);
                    cr_b = cr_a + px_sz;
                    d = (xo - (xo & IPGUI_FIXED_MASK));
                } else if (yo > border_y) { /* bottom edge */
                    alpha = yo - border_y;
                    temp_x = IPGUI_FIXED_FLOOR(xo);
                    temp_y = IPGUI_FIXED_FLOOR(yo);
                    /* get src color */
                    cr_a = image_pixmap_get(image, temp_x, temp_y);
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
#if DONOT_USE_OPTIM == 0
_next_pix_inc:
            xo += next_pix_incx;/* inv.a */
            yo += next_pix_incy;/* inv.c */
#endif
        }
#if DONOT_USE_OPTIM == 0
        xo_start_row += next_row_incx;/* inv.b */
        yo_start_row += next_row_incy;/* inv.d */
#endif
    }
    r_img->w = w_trans;
    r_img->h = h_trans;
    r_img->pixmap = pixmap;
    r_img->fmt = image->fmt;
    r_img->stride = px_sz * w_trans;
    r_img->mask = mask_buf;

    return r_img;
}
