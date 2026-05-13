/*
 * MIT License
 *
 * Copyright (c) 2025 JinYiCheng
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* ipgui_prim.c讨论各个基元之间的关系，并提供简单的光栅化方法 */

#include "ipgui_prim.h"
#include "ipgui_utils.h"

__IPGUI_API__ void ipgui_aabb_rel_move(ipgui_aabb_t * aabb, ipgui_coord_t x, ipgui_coord_t y, ipgui_aabb_t * res)
{
    res->start.x = aabb->start.x + x;
    res->start.y = aabb->start.y + y;
    res->end.x = aabb->end.x + x;
    res->end.y = aabb->end.y + y;
}

/* if two rectangles intersect, if return 0, else return -1 */
__IPGUI_API__ int ipgui_aabb_intersect(ipgui_aabb_t * res, ipgui_aabb_t * aabb1, ipgui_aabb_t * aabb2)
{
    if(( !res ) || ( !aabb1 ) || ( !aabb2 ))
        return -2;
    
    res->start.x = IPGUI_MAX(aabb1->start.x, aabb2->start.x);
    res->start.y = IPGUI_MAX(aabb1->start.y, aabb2->start.y);
    res->end.x = IPGUI_MIN(aabb1->end.x, aabb2->end.x);
    res->end.y = IPGUI_MIN(aabb1->end.y, aabb2->end.y);

    if( (res->start.x > res->end.x) || (res->start.y > res->end.y) )
        return -1;
    else
        return 0;
}

__IPGUI_API__ int ipgui_aabb_overlap(ipgui_aabb_t * res, ipgui_aabb_t * aabb1, ipgui_aabb_t * aabb2)
{
    if(( !res ) || ( !aabb1 ) || ( !aabb2 ))
        return -2;
    
    res->start.x = IPGUI_MAX(aabb1->start.x, aabb2->start.x);
    res->start.y = IPGUI_MAX(aabb1->start.y, aabb2->start.y);
    res->end.x = IPGUI_MIN(aabb1->end.x, aabb2->end.x);
    res->end.y = IPGUI_MIN(aabb1->end.y, aabb2->end.y);

    if( (res->start.x > res->end.x) || (res->start.y > res->end.y) )
        return -1;
    else
        return 0;
}

/* if two rectangles intersect, if return 0, else return -1 */
__IPGUI_API__ int ipgui_rect_have_intersect(ipgui_rect_t * rect1, ipgui_rect_t * rect2)
{
    ipgui_coord_t min_x = 0, min_y = 0, max_x = 0, max_y = 0;

    min_x = IPGUI_MAX(rect1->start.x, rect2->start.x);
    min_y = IPGUI_MAX(rect1->start.y, rect2->start.y);
    max_x = IPGUI_MIN(rect1->end.x, rect2->end.x);
    max_y = IPGUI_MIN(rect1->end.y, rect2->end.y);

    if( (min_x > max_x) || (min_y > max_y) )
        return -1;
    else
        return 0;
}

/* generate AABB from points */
/* AABB is axis aligned bounding box(轴对齐边界框，也称轴对齐的包围盒，也就是四条边和x、y轴平行的矩形) */
__IPGUI_API__ int ipgui_aabb_generate_with_points(ipgui_aabb_t * ret_aabb, ipgui_point_t * points, int num)
{
    ipgui_coord_t min_x = 0, min_y = 0, max_x = 0, max_y = 0;

    min_x = points[0].x;
    min_y = points[0].y;
    max_x = points[0].x;
    max_y = points[0].y;
    for(int i = 1; i < num; i ++)
    {
        if(points[i].x < min_x)
            min_x = points[i].x;
        if(points[i].y < min_y)
            min_y = points[i].y;
        if(points[i].x > max_x)
            max_x = points[i].x;
        if(points[i].y > max_y)
            max_y = points[i].y;
    }
    ret_aabb->start.x = min_x;
    ret_aabb->start.y = min_y;
    ret_aabb->end.x   = max_x;
    ret_aabb->end.y   = max_y;

    return 0;
}

/* generate aabb from two rectangles */ /* 生成一个最小外接AABB包围这两个矩形 */
/* 最小外接就是刚好能包围的意思 */
__IPGUI_API__ int ipgui_aabb_generate_with_rect(ipgui_aabb_t * res, ipgui_rect_t * rect1, ipgui_rect_t * rect2)
{
    if( !res )
        return -1;
    if( !rect1 && !rect2 )
        return -1; 
    else if( !rect1 )
        * res = * rect2;
    else if( !rect2 )
        * res = * rect1;

    res->start.x = IPGUI_MIN(rect1->start.x, rect2->start.x);
    res->start.y = IPGUI_MIN(rect1->start.y, rect2->start.y);
    res->end.x   = IPGUI_MAX(rect1->end.x, rect2->end.x);
    res->end.y   = IPGUI_MAX(rect1->end.y, rect2->end.y);
}

/* update AABB with point 计算点和AABB的一个最小外接AABB */
__IPGUI_API__ void ipgui_aabb_update_with_point(ipgui_aabb_t * aabb, ipgui_point_t * point)
{
    if (aabb)
    {
        if(point->x < aabb->start.x)
            aabb->start.x = point->x;
        if(point->y < aabb->start.y)
            aabb->start.y = point->y;
        if(point->x > aabb->end.x)
            aabb->end.x = point->x;
        if(point->y > aabb->end.y)
            aabb->end.y = point->y;
    }
}

/* move rect with relative position */
__IPGUI_API__ void ipgui_rect_rel_move(ipgui_rect_t * rect, ipgui_coord_t x, ipgui_coord_t y)
{
    if( rect )
    {
        rect->start.x += x;
        rect->start.y += y;
        rect->end.x += x;
        rect->end.y += y;
    }
}

__IPGUI_API__ void ipgui_aabb_expand(ipgui_aabb_t * aabb, ipgui_coord_t e)
{
    aabb->start.x -= e;
    aabb->start.y -= e;
    aabb->end.x += e;
    aabb->end.y += e;
}



/* if point in rect, if return 1, else return 0 */
__IPGUI_API__ int ipgui_point_in_rect(ipgui_point_t * point, ipgui_rect_t * rect)
{
    if( (point->x >= rect->start.x) && (point->x <= rect->end.x) && \
        (point->y >= rect->start.y) && (point->y <= rect->end.y) )
        return 1;
    else
        return 0;
}

__IPGUI_API__ int ipgui_point_in_aabb(ipgui_coord_t x, ipgui_coord_t y, ipgui_aabb_t * aabb)
{
    if( (x >= aabb->start.x) && (x <= aabb->end.x) && \
        (y >= aabb->start.y) && (y <= aabb->end.y) )
        return 1;
    else
        return 0;
}

/* if point in aabb, if return 1, else return 0 */
__IPGUI_API__ int ipgui_point_in_aabb_radius(ipgui_point_t * point, ipgui_aabb_t * aabb)
{
    if (!((point->x >= aabb->start.x) && (point->x <= aabb->end.x) && \
        (point->y >= aabb->start.y) && (point->y <= aabb->end.y)))
        return 0;

    /* out of 4 radius */
    
}

/* 计算的是两点之间的距离平方 */
__IPGUI_STATIC__ ipgui_coord_t ipgui_vector_length(ipgui_vector_t * vector)
{
    return vector->x * vector->x + vector->y * vector->y;
}

/* calculate distance between two points */
/* 计算的是两点之间的距离平方 */
__IPGUI_STATIC__ ipgui_coord_t ipgui_two_point_distance(ipgui_point_t * point1, ipgui_point_t * point2)
{
    ipgui_vector_t vector;
    ipgui_vector_generate(&vector, point1, point2);
    return ipgui_vector_length(&vector);
}

/* if point in circle,if in return 1,else if 0 */
__IPGUI_API__ int ipgui_point_in_circle(ipgui_point_t * point, ipgui_circle_t * circle)
{
    return (ipgui_two_point_distance(point, &circle->center) <= circle->radius * circle->radius) ? 1 : 0;
}

/* conic bezier curve to cubic bezier curve *//* 二次贝塞尔曲线转三次贝塞尔曲线，点的轨迹一样，只是增加控制点数目 */
__IPGUI_API__ void ipgui_conic_bezier_to_cubic_bezier(ipgui_point_t * _3p, ipgui_point_t * _4p)
{
    /* suppose Q0 Q1 Q2 need map to C0 C1 C2 C3, then the fomula is:
     * C0 = Q0
     * C1 = Q0 + (2/3) (Q1 - Q0)
     * C2 = Q2 + (2/3) (Q1 - Q2)
     * C3 = Q2
     */
    ipgui_point_t * p = _3p;
    ipgui_point_t * gen = _4p;

    /* first control point */
    gen[0].x = p[0].x;
    gen[0].y = p[0].y;

    /* second control point */
    gen[1].x = 1 * p[0].x / 3 + 2 * p[1].x / 3;
    gen[1].y = 1 * p[0].y / 3 + 2 * p[1].y / 3;

    /* third control point */
    gen[2].x = 1 * p[2].x / 3 + 2 * p[1].x / 3;
    gen[2].y = 1 * p[2].y / 3 + 2 * p[1].y / 3;

    /* fourth control point */
    gen[3].x = p[2].x;
    gen[3].y = p[2].y;
}

/* clip line in aabb, Liang-Barsky algorithm */
__IPGUI_API__ int ipgui_aabb_clip_line_liang(ipgui_aabb_t * aabb, ipgui_line_t * line, ipgui_line_t * res)
{

}

/* if line and aabb intersect */
__IPGUI_API__ int ipgui_aabb_line_intersect(ipgui_aabb_t * aabb, ipgui_line_t * line)
{
    
}

/* 用cubic curve拟合圆：https://juejin.cn/post/7433428170471276544 */