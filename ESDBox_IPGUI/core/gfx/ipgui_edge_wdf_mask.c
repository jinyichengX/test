#include "ipgui_edge_wdf_mask.h"

/* 这个文件用于计算edge的wdf蒙版 
 * 暂时只支持整数端点edge！！！！！！
 */

/* 已知y，求直线对应的x坐标（仅对斜线有效） */
__IPGUI_STATIC__ void ipgui_line_x_at_y(
    ipgui_edge_wdf_param_t * param,
    ipgui_coord_t            y, 
    ipgui_coord_t          * x,   /* res */
    s32_t                  * frac /* res */)
{
    ipgui_coord_t temp, dy;

    dy = y - param->y1;
    temp = dy * param->dx;
    * x = temp / param->dy;
    * frac = temp - ((* x) * param->dy);
    if ((* frac) < 0) {
        * x -= 1;
        * frac += param->dy;
    }
    * x += param->x1;
}

__IPGUI_STATIC__ void ipgui_line_x_step(
    ipgui_xidx_t              * x_idx,
    ipgui_xstep_t               step,
    ipgui_edge_wdf_mask_dsc_t * dsc)
{
    x_idx->inte += step.inte;
    x_idx->frac += step.frac;

    if (dsc->p->dx > 0) {
        while (IPGUI_ABS(x_idx->frac) >= dsc->p->dy) {
            x_idx->inte += 1;
            x_idx->frac -= dsc->p->dy;
        }
    } else {
        while (IPGUI_ABS(x_idx->frac) >= dsc->p->dy) {
            x_idx->inte -= 1;
            x_idx->frac += dsc->p->dy; 
        }
        if (x_idx->frac < 0) {
            x_idx->inte -= 1;
            x_idx->frac += dsc->p->dy; 
        }
    }
}

__IPGUI_API__ ipgui_edge_wdf_param_t ipgui_edge_wdf_param_init(
    ipgui_coord_t x1,
    ipgui_coord_t y1,
    ipgui_coord_t x2,
    ipgui_coord_t y2)
{
    ipgui_edge_wdf_param_t param;
    
    param.a = y1 - y2;
    param.b = x2 - x1;
    param.c = x1 * y2 - x2 * y1;

    if (y1 > y2) {
        param.dy = y1 - y2;
        param.dx = x1 - x2;
        param.x1 = x2;
        param.y1 = y2;
    } else {
        param.dy = y2 - y1;
        param.dx = x2 - x1;
        param.x1 = x1;
        param.y1 = y1;
    }

    if (IPGUI_ABS(param.dx) >= param.dy) {
        param.flatten = 1;

        s64_t scaled_dy = (s64_t)param.dy << 16;
        s32_t half_dx   = param.dx / 2;

        if (param.dx > 0) {
            param.delta_y = (scaled_dy + half_dx) / IPGUI_ABS(param.dx);
        } else if (param.dx < 0) {
            param.delta_y = (scaled_dy - half_dx) / IPGUI_ABS(param.dx);
        }

        /* 计算修正因子索引 */ 
        s32_t abs_dx = IPGUI_ABS(param.dx);
        s32_t abs_dy = param.dy;
        param.correction_frac_index = (u8_t)((abs_dy * 126 + (abs_dx >> 1)) / abs_dx);
    } else {
        param.flatten = 0;
        
        /* 计算修正因子索引 */ 
        s32_t abs_dx = IPGUI_ABS(param.dx);
        s32_t abs_dy = param.dy;
        param.correction_frac_index = (u8_t)((abs_dx * 126 + (abs_dy >> 1)) / abs_dy);
    }

    return param;
}

/* 求横向跨度 */
__IPGUI_API__ ipgui_coord_t ipgui_edge_wdf_x_halfspan(
    ipgui_edge_wdf_param_t * param,
    ipgui_coord_t            width)
{
    /* 随便选取一个直线端点向右单点步进试探
     * 直到 d^2 <= (Ax0 + By0 + C)^2 / (A^2 + B^2) 
     * 附：点到直线距离公式d = |Ax0 + By0 + C| / sqrt(A^2 + B^2)
     */
    ipgui_coord_t x_step = param->x1 + 1;
    ipgui_coord_t x_half_span;
    
    ipgui_coord_t half_w2 = width >> 1;
    if (width & 1) half_w2 += 1;
    half_w2 *= half_w2;

    u32_t a2_plus_b2 = param->a * param->a 
                     + param->b * param->b;

    s64_t temp;
    while (1) {
        temp = param->a * x_step    + 
               param->b * param->y1 + 
               param->c;
        temp *= temp;
        if (temp >= ((s64_t)half_w2 * a2_plus_b2)) {
            x_step ++;
            break;
        }
        x_step ++;
    }

    x_half_span = x_step - param->x1;
    return x_half_span;
}

__IPGUI_API__ void ipgui_gen_edge_wdf_mask_dsc(
    ipgui_edge_wdf_mask_dsc_t * res,
    ipgui_edge_wdf_param_t    * param,
    ipgui_coord_t               start_y,
    ipgui_coord_t               width)
{
    /* gen x_half_span */
    res->x_half_span = ipgui_edge_wdf_x_halfspan(param, width);

    /* gen x_step */
    res->x_step.inte = param->dx / param->dy;
    res->x_step.frac = param->dx - res->x_step.inte * param->dy;

    /* gen half_width64 */
    res->half_width64 = width << 5;

    /* 计算起始点的x坐标和frac */
    ipgui_line_x_at_y(param, start_y, &res->x_idx.inte, &res->x_idx.frac);

    res->p = param;
}

__IPGUI_API__ void ipgui_edeg_wdf_mask_dsc_next_y(
    ipgui_edge_wdf_mask_dsc_t * dsc)
{
    ipgui_line_x_step(&dsc->x_idx, dsc->x_step, dsc);
}

#define correct_d_and_mask(distance, half_width)\
    distance = distance * correction_frac[dsc->p->correction_frac_index] >> 8;\
    if (distance >= (half_width + 64)) mask = 0;\
    else if (distance <= half_width) mask = 255;\
    else mask = (64 - (distance - half_width)) << 2;\

extern const u8_t correction_frac[127];

/* 单行（批量）mask生成，不混合只填充 */
__IPGUI_API__ void ipgui_edge_wdf_mask(
    ipgui_edge_wdf_mask_dsc_t * dsc,
    ipgui_coord_t               sx, /* start coordinate of x */
    u8_t                      * mask_buf,
    ipgui_coord_t               len /* length of mask buffer */)
{
    ipgui_coord_t x_left, x_right;
    u8_t lfrac64 = 0, rfrac64 = 0;

    if (dsc->x_idx.frac) {
        x_left  = dsc->x_idx.inte - dsc->x_half_span;
        x_right = dsc->x_idx.inte + 1 + dsc->x_half_span;
        lfrac64 = ((s64_t)dsc->x_idx.frac << 6) / dsc->p->dy;/* x_idx.frac scale to 0 - 64 */
        rfrac64 = 64 - lfrac64;
    } else {
        x_left  = dsc->x_idx.inte - dsc->x_half_span;
        x_right = dsc->x_idx.inte + dsc->x_half_span;
    }

    /* generate edge wdf mask */
    u8_t mask;
    u32_t dist64, d;

    dist64 = (dsc->x_idx.inte - x_left) << 6;
    dist64 += lfrac64;
    while (1) {
        if (dsc->p->flatten) {
            d = ((s64_t)dist64 * dsc->p->delta_y + 32768) >> 16;/* 转化成到直线的轴向距离 */
        } else {
            d = dist64;
        }
        correct_d_and_mask(d, dsc->half_width64);
        if (mask == 255) {
            break;
        }
        x_left ++;
        dist64 -= 64;
    }

    dist64 = (x_right - (dsc->x_idx.inte + 1)) << 6;
    dist64 += rfrac64;
    while (1) {
        if (dsc->p->flatten) {
            d = ((s64_t)dist64 * dsc->p->delta_y + 32768) >> 16;/* 转化成到直线的轴向距离 */
        } else {
            d = dist64;
        }
        correct_d_and_mask(d, dsc->half_width64);
        if (mask == 255) {
            break;
        }
        x_right --;
        dist64 -= 64;
    }
}

/* 单点mask生成 */
__IPGUI_API__ u8_t ipgui_edge_wdf_mask_point(
    ipgui_edge_wdf_mask_dsc_t * dsc,
    ipgui_coord_t               x)
{
    u8_t mask;
    u32_t dist64, d;
    u8_t lfrac64 = 0;
    lfrac64 = ((s64_t)dsc->x_idx.frac << 6) / dsc->p->dy;/* x_idx.frac scale to 0 - 64 */
    if (dsc->x_idx.frac) {
        if (x <= dsc->x_idx.inte) {
            dist64 = ((dsc->x_idx.inte - x) << 6) + lfrac64;
        } else {
            dist64 = ((x - dsc->x_idx.inte) << 6) - lfrac64;
        }
    } else {
        dist64 = (IPGUI_ABS(x - dsc->x_idx.inte)) << 6;
    }
    if (dsc->p->flatten) {
        d = ((s64_t)dist64 * dsc->p->delta_y + 32768) >> 16;/* 转化成轴向距离 */
    } else {
        d = dist64;
    }
    correct_d_and_mask(d, dsc->half_width64);
    return mask;
}

#if 1
/* 测试，逐点遍历效率很低 */
#include "ipgui_pattle.h"
void test_first_octant_wdf(ipgui_surf_t * surf)
{
    ipgui_color_t g_color;
    IPGUI_COLOR_SET(g_color, 255, 0x2196f3);

    static ipgui_coord_t y_step = 101;

    ipgui_edge_wdf_param_t edge_param = ipgui_edge_wdf_param_init(
        0,   100,   /* 自己定义起点 */
        200, y_step ++     /* 自己定义终点 */
    );

    ipgui_edge_wdf_mask_dsc_t dsc;
    ipgui_gen_edge_wdf_mask_dsc(&dsc, &edge_param, 0, 100);


    for (ipgui_coord_t y = 0; y < 480; y ++) {
#if 0 /* 单点 */ /* 这个分支测试完成 */
        for (ipgui_coord_t x = 0; x < 800; x ++) {
            u8_t mask = ipgui_edge_wdf_mask_point(&dsc, x);
            if (mask > 0) {
                ipgui_draw_pixel(
                    surf,
                    NULL,
                    x, y,
                    g_color,
                    mask,
                    255,
                    0
                );
            }
        }
#else /* 批量 */ /* 这个分支未测试 */
        static u8_t mask_buf[800];
        ipgui_edge_wdf_mask(&dsc, 0, mask_buf, 800);
        for (ipgui_coord_t x = 0; x < 800; x ++) {
            u8_t mask = mask_buf[x];
            if (mask > 0) {
                ipgui_draw_pixel(
                    surf,
                    NULL,
                    x, y,
                    g_color,
                    mask,
                    255,
                    0
                );
            }
        }
#endif
        ipgui_edeg_wdf_mask_dsc_next_y(&dsc);
    }
}
#endif