#ifndef IPGUI_PRIM_H
#define IPGUI_PRIM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ipgui_coord.h"
#include "ipgui_utils.h"

typedef s32_t ipgui_scoord_t; //it means sub pixel coordinate, 26.6

typedef struct {
    ipgui_scoord_t x; /* x coordinate, 26.6 */
    ipgui_scoord_t y; /* y coordinate, 26.6 */
}ipgui_spoint_t;

typedef struct {
    ipgui_scoord_t x; /* x coordinate, 26.6 */
    ipgui_scoord_t y; /* y coordinate, 26.6 */
}ipgui_svector_t;

typedef struct {
    ipgui_spoint_t start;
    ipgui_spoint_t end;
}ipgui_sline_t;

typedef struct
{
    ipgui_coord_t x;
    ipgui_coord_t y;
}ipgui_point_t;

typedef struct
{
    ipgui_coordf_t x;
    ipgui_coordf_t y;
}ipgui_pointf_t;

typedef ipgui_point_t ipgui_vector_t;
typedef ipgui_pointf_t ipgui_vectorf_t;

typedef struct
{
    ipgui_point_t start;
    ipgui_point_t end;
}ipgui_line_t;

typedef struct
{
    ipgui_point_t start;/* start x and y must be smaller than end x and y */
    ipgui_point_t end; /* end x and y must be larger than start x and y */
}ipgui_rect_t;

typedef ipgui_rect_t ipgui_aabb_t;
typedef ipgui_rect_t ipgui_saabb_t;

typedef struct
{
    ipgui_point_t center;
    ipgui_coord_t radius;
}ipgui_circle_t;

__IPGUI_STATIC__ __IPGUI_INLINE__ void ipgui_aabb_set_sx(ipgui_aabb_t * aabb, ipgui_coord_t sx)
{
    aabb->start.x = sx;
}

__IPGUI_STATIC__ __IPGUI_INLINE__ void ipgui_aabb_set_sy(ipgui_aabb_t * aabb, ipgui_coord_t sy)
{
    aabb->start.y = sy;
}

__IPGUI_STATIC__ __IPGUI_INLINE__ void ipgui_aabb_set_ex(ipgui_aabb_t * aabb, ipgui_coord_t ex)
{
    aabb->end.x = ex;
}

__IPGUI_STATIC__ __IPGUI_INLINE__ void ipgui_aabb_set_ey(ipgui_aabb_t * aabb, ipgui_coord_t ey)
{
    aabb->end.y = ey;
}

__IPGUI_STATIC__ __IPGUI_INLINE__ ipgui_coord_t ipgui_aabb_width(ipgui_aabb_t * aabb)
{
    return aabb->end.x - aabb->start.x + 1;
}

__IPGUI_STATIC__ __IPGUI_INLINE__ ipgui_coord_t ipgui_aabb_height(ipgui_aabb_t * aabb)
{
    return aabb->end.y - aabb->start.y + 1;
}

__IPGUI_STATIC__ __IPGUI_INLINE__ int ipgui_aabb_empty(ipgui_aabb_t * aabb)
{   
    return ((aabb->start.x > aabb->end.x) || (aabb->start.y > aabb->end.y));
}

/* generate vector from start to end */
__IPGUI_STATIC__ __IPGUI_INLINE__ void ipgui_vector_generate(ipgui_vector_t * ret_vector, ipgui_point_t * start, ipgui_point_t * end)
{
    ret_vector->x = end->x - start->x;
    ret_vector->y = end->y - start->y;
}


extern __IPGUI_API__ int ipgui_aabb_intersect(ipgui_aabb_t * res, ipgui_aabb_t * aabb1, ipgui_aabb_t * aabb2);
extern __IPGUI_API__ int ipgui_aabb_overlap(ipgui_aabb_t * res, ipgui_aabb_t * aabb1, ipgui_aabb_t * aabb2);
extern __IPGUI_API__ void ipgui_aabb_rel_move(ipgui_aabb_t * aabb, ipgui_coord_t x, ipgui_coord_t y, ipgui_aabb_t * res);
extern __IPGUI_API__ int ipgui_aabb_generate(ipgui_rect_t * ret_aabb, ipgui_point_t points[], int num);
extern __IPGUI_API__ int ipgui_point_in_rect(ipgui_point_t * point, ipgui_rect_t * rect);
extern __IPGUI_API__ int ipgui_point_in_aabb(ipgui_coord_t x, ipgui_coord_t y, ipgui_aabb_t * aabb);
extern __IPGUI_API__ int ipgui_point_in_circle(ipgui_point_t * point, ipgui_circle_t * circle);
extern __IPGUI_API__ void ipgui_aabb_expand(ipgui_aabb_t * aabb, ipgui_coord_t e);
extern __IPGUI_API__ int ipgui_aabb_generate_with_rect(ipgui_aabb_t * res, ipgui_rect_t * rect1, ipgui_rect_t * rect2);
extern __IPGUI_API__ void ipgui_aabb_update_with_point(ipgui_aabb_t * aabb, ipgui_point_t * point);
extern __IPGUI_API__ int ipgui_aabb_generate_with_points(ipgui_aabb_t * ret_aabb, ipgui_point_t * points, int num);

#ifdef __cplusplus
}
#endif

#endif