#include "ipgui_curve.h"
#include "ipgui_screen.h"
#include "ipgui_debug.h"

typedef int ipgui_angle_t; //角度，0-360度

#define MAGIC_NUM 0.552284749830 /* 魔数，用于三次曲线控制点计算 */
#define IPGUI_CURVE_MAGIC_INT (int)(MAGIC_NUM * (float)(1 << IPGUI_PIXEL_BITS)) /* 整数部分 */
#define IPGUI_CURVE_MAGIC_FRACT (int)(((MAGIC_NUM * (float)(1 << IPGUI_PIXEL_BITS)) * 100) \
                                - (IPGUI_CURVE_MAGIC_INT * 100)) /* 小数部分，百分比表示 */
#define IPGUI_SUBDIV_PIXEL_AREA ((1 << IPGUI_PIXEL_BITS) * (1 << IPGUI_PIXEL_BITS)) /* 子像素细分后每个像素的面积 */

/* 2次曲线细分 */
__IPGUI_API__ void ipgui_split_curve2(ipgui_curve2_t * curve, ipgui_curve2_t * first, ipgui_curve2_t * second)
{
    ipgui_spoint_t p01,p12;
    p01.x = (curve->x0 + curve->x1) >> 1;
    p01.y = (curve->y0 + curve->y1) >> 1;
    p12.x = (curve->x1 + curve->x2) >> 1;
    p12.y = (curve->y1 + curve->y2) >> 1;

    first->x0 = curve->x0;
    first->x1 = p01.x;
    first->x2 = (p01.x + p12.x) >> 1;
    first->y0 = curve->y0;
    first->y1 = p01.y;
    first->y2 = (p01.y + p12.y) >> 1;

    second->x0 = first->x2;
    second->x1 = p12.x;
    second->x2 = curve->x2;
    second->y0 = first->y2;
    second->y1 = p12.y;
    second->y2 = curve->y2;
}

/* 3次曲线细分 */
__IPGUI_API__ void ipgui_split_curve3(ipgui_curve3_t * curve, ipgui_curve3_t * first, ipgui_curve3_t * second)
{
    ipgui_scoord_t c = (curve->x1 + curve->x2) >> 1;
	first->x1 = (curve->x0 + curve->x1) >> 1;
	second->x2 = (curve->x2 + curve->x3) >> 1;
	first->x0 = curve->x0;
	second->x3 = curve->x3;
	first->x2 = (first->x1 + c) >> 1;
	second->x1 = (second->x2 + c) >> 1;
	first->x3 = second->x0 = (first->x2 + second->x1) >> 1;

	c = (curve->y1 + curve->y2) >> 1;
	first->y1 = (curve->y0 + curve->y1) >> 1;
	second->y2 = (curve->y2 + curve->y3) >> 1;
	first->y0 = curve->y0;
	second->y3 = curve->y3;
	first->y2 = (first->y1 + c) >> 1;
	second->y1 = (second->y2 + c) >> 1;
	first->y3 = second->y0 = (first->y2 + second->y1) >> 1;
}

/* convert curve2 to curve3 */
__IPGUI_API__ void ipgui_curve22curve3(ipgui_curve2_t * curve2, ipgui_curve3_t * res)
{
    /* suppose Q0 Q1 Q2 need map to C0 C1 C2 C3, then the fomula is:
     * C0 = Q0
     * C1 = Q0 + (2/3) (Q1 - Q0)
     * C2 = Q2 + (2/3) (Q1 - Q2)
     * C3 = Q2
     */
    ipgui_spoint_t * p = curve2;
    ipgui_spoint_t * gen = res;

    /* first point */
    gen[0].x = p[0].x;
    gen[0].y = p[0].y;

    /* second control point */
    gen[1].x = 1 * p[0].x / 3 + 2 * p[1].x / 3;
    gen[1].y = 1 * p[0].y / 3 + 2 * p[1].y / 3;

    /* third control point */
    gen[2].x = 1 * p[2].x / 3 + 2 * p[1].x / 3;
    gen[2].y = 1 * p[2].y / 3 + 2 * p[1].y / 3;

    /* last point */
    gen[3].x = p[2].x;
    gen[3].y = p[2].y;
}

/* 生成圆弧的曲线控制点 */
__IPGUI_API__ void ipgui_generate_curve3_for_arc(ipgui_spoint_t center, ipgui_scoord_t radius, 
                                    ipgui_angle_t start, ipgui_angle_t end, ipgui_curve3_t * ret)
{

}

__IPGUI_API__ void ipgui_generate_quad_circle_control(ipgui_scoord_t radius,
                    ipgui_svector_t * v0, ipgui_svector_t * v1,
                    ipgui_svector_t * v2, ipgui_svector_t * v3)
{
    /*
            delta
         _____^_____
        /           \
        v0          v1  
        ^
        |               
        |                      
        |                         v2  \
        |                             |
        |                              > delta
        |                             |                       
        ori ------------------->  v3  /
        
        generate the relative coordinates, relative to ori(0,0)
    */

    if (radius <= 0) return;

    ipgui_scoord_t delta, tmp;
    ipgui_scoord_t coeff = 100 *  (1 << IPGUI_PIXEL_BITS);
    tmp = (IPGUI_CURVE_MAGIC_INT * 100) + IPGUI_CURVE_MAGIC_FRACT;
    tmp *= radius;
    delta = tmp / coeff;
    if (tmp % coeff > (coeff >> 1)) delta ++;

    v0->x = v3->y = 0;
    v0->y = v1->y = v3->x = v2->x = radius;
    v1->x = v2->y = delta;
}

__IPGUI_API__ void ipgui_generate_quad_ellipse_control(
                    ipgui_scoord_t radius_x, ipgui_scoord_t radius_y,
                    ipgui_svector_t * v0, ipgui_svector_t * v1,
                    ipgui_svector_t * v2, ipgui_svector_t * v3)
{
    /*
            deltax
         _____^_____
        /           \
        v0          v1  
        ^    
        |                            v2  \
        |                                |
        |                                 > deltay
        |                                |                       
        ori ---------------------->  v3  /
        
        generate the relative coordinates, relative to ori(0,0)
    */
    if ((radius_x <= 0) || (radius_y <= 0)) return;

    ipgui_scoord_t deltax, deltay, tmp;
    ipgui_scoord_t coeff = 100 *  (1 << IPGUI_PIXEL_BITS);

    tmp = (IPGUI_CURVE_MAGIC_INT * 100) + IPGUI_CURVE_MAGIC_FRACT;
    tmp *= radius_x;
    deltax = tmp / coeff;
    if (tmp % coeff > (coeff >> 1)) deltax ++;

    tmp = (IPGUI_CURVE_MAGIC_INT * 100) + IPGUI_CURVE_MAGIC_FRACT;
    tmp *= radius_y;
    deltay = tmp / coeff;
    if (tmp % coeff > (coeff >> 1)) deltay ++;

    v0->x = v3->y = 0;
    v0->y = v1->y = radius_y;
    v1->x = deltax;
    v3->x = v2->x = radius_x;
    v2->y = deltay;
}

__IPGUI_API__ char ipgui_curve3_is_sufficently_flat(ipgui_curve3_t * curve) 
{
    /*  pseudo-code from https://hcklbrrfnn.wordpress.com/wp-content/uploads/2012/08/bez.pdf
        double ux = 3.0*curve->x1- 2.0*curve->x0- curve->x3; ux *= ux;
        double uy = 3.0*curve->y1- 2.0*curve->y0- curve->y3; uy *= uy;
        double vx = 3.0*curve->x2- 2.0*curve->x3- curve->x0; vx *= vx;
        double vy = 3.0*curve->y2- 2.0*curve->y3- curve->y0; vy *= vy;
    */

    ipgui_scoord_t tolerance;
    ipgui_scoord_t ux, uy, vx, vy;

    tolerance = 16 * (IPGUI_SUBDIV_PIXEL_AREA / (4 * 4));
    ux = 3 * curve->x1 - 2 * curve->x0 - curve->x3; ux *= ux;
    uy = 3 * curve->y1 - 2 * curve->y0 - curve->y3; uy *= uy;
    vx = 3 * curve->x2 - 2 * curve->x3 - curve->x0; vx *= vx;
    vy = 3 * curve->y2 - 2 * curve->y3 - curve->y0; vy *= vy;
    
    if (ux < vx) ux = vx;
    if (uy < vy) uy = vy;
    return (ux + uy <= tolerance); /* tolerance is 16*tol^2 tol可以取0.25或0.5 */
}
