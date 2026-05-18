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
    ipgui_xidx_t * x_idx,
    ipgui_xstep_t  step,
    ipgui_coord_t  dx,
    ipgui_coord_t  dy)
{
    x_idx->inte += step.inte;
    x_idx->frac += step.frac;

    while (IPGUI_ABS(x_idx->frac) >= dy)
    {
        if (dx > 0) {
            x_idx->inte += 1;
            x_idx->frac -= dy;
        } else {
            x_idx->inte -= 1;
            x_idx->frac += dy; 
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

    return param;
}

/* 求横向跨度 */
__IPGUI_API__ ipgui_coord_t ipgui_calc_xspan(
    ipgui_edge_wdf_param_t * param,
    ipgui_coord_t width)
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

__IPGUI_API__ void ipgui_gen_edge_mask_dsc(
    ipgui_edge_wdf_mask_dsc_t * res,
    ipgui_edge_wdf_param_t    * param,
    ipgui_coord_t               width)
{
    /* gen x_half_span */
    res->x_half_span = ipgui_calc_xspan(param, width);

    /* gen x_step */
    res->x_step.inte = param->dx / param->dy;
    res->x_step.frac = param->dx - res->x_step.inte * param->dy;
}

__IPGUI_API__ void ipgui_edge_wdf_mask()
{

}
