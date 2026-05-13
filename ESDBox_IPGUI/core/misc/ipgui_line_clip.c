#include "ipgui_line_clip.h"
#include "ipgui_graphic2.h"

#define ipgui_scoord_up1(scoord)     ((scoord) + ((1 << IPGUI_PIXEL_BITS) - 1) & ((~0U) << IPGUI_PIXEL_BITS))

static void gen_axis_distance(ipgui_scoord_t from, 
                                    ipgui_scoord_t to, 
                                    int * dist)
{
    ipgui_coord_t min_up, max_up;
    unsigned int min_fract, max_fract;

    if (to > from) {
        min_up = ipgui_scoord_up1(from);
        min_fract = min_up - from;
        max_up = ipgui_scoord_up1(to);
        max_fract = max_up - to;

        * dist = max_up - min_up - max_fract + min_fract;
    } else {
        min_up = ipgui_scoord_up1(to);
        min_fract = min_up - to;
        max_up = ipgui_scoord_up1(from);
        max_fract = max_up - from;

        * dist = -(max_up - min_up - max_fract + min_fract);
    }
}

void ipgui_line_slope_gen(ipgui_spoint_t from, ipgui_spoint_t to, 
                        ipgui_line_slope_t * slope)
{
    gen_axis_distance(from.x, to.x, &slope->dx);
    gen_axis_distance(from.y, to.y, &slope->dy);
}

void ipgui_gen_line_dsc(ipgui_spoint_t from, ipgui_spoint_t to, ipgui_sline_dsc_t * line_dsc)
{
    ipgui_line_slope_gen(from, to, &line_dsc->line_slope);
    line_dsc->start = from;
}

#define pos_rel_code0 0x09U
#define pos_rel_code1 0x08U
#define pos_rel_code2 0x0AU
#define pos_rel_code3 0x01U
#define pos_rel_code4 0x00U
#define pos_rel_code5 0x02U
#define pos_rel_code6 0x05U
#define pos_rel_code7 0x04U
#define pos_rel_code8 0x06U

/* encode point for Cohen algorithm */
__IPGUI_STATIC__ __IPGUI_INLINE__ void ipgui_line_vertex_code_generate(ipgui_point_t * point, ipgui_aabb_t * aabb, unsigned char * code)
{
    unsigned char top, bottom, left, right;
    top = 0;
    bottom = 0;
    left = 0;
    right = 0;
    /* the postion relation between point and aabb
     * the middle grid represent aabb, code is 0b0000
     *            |           |
     *     1001   |    1000   |    1010          
     *            |           | 
     * -----------|-----------|-----------
     *            |           |
     *     0001   |    0000   |    0010
     *            |           |
     * -----------|-----------|-----------
     *            |           |
     *     0101   |    0100   |    0110
     *            |           |
     */
    if (point->x < aabb->start.x)
    {
        left |= pos_rel_code3;
    } else if (point->x > aabb->end.x)
    {
        right |= pos_rel_code5;
    }

    if (point->y < aabb->start.y)
    {
        top |= pos_rel_code1;
    } else if (point->y > aabb->end.y)
    {
        bottom |= pos_rel_code7;
    }
    * code = top | bottom | left | right;
}

/* clip line in aabb, Cohen algorithm */
/* return 1: all out of aabb */
/* return 0: instersect with aabb */
__IPGUI_API__ int ipgui_line_clip_cohen(ipgui_aabb_t * aabb, ipgui_line_t * line, ipgui_line_t * res)
{
    ipgui_point_t p1, p2;
    unsigned char sc, ec;
    unsigned char code;
    ipgui_point_t * p_tmp;

    p1 = line->start;
    p2 = line->end;
    while (1)
    {
        ipgui_line_vertex_code_generate(&p1, aabb, &sc);
        ipgui_line_vertex_code_generate(&p2, aabb, &ec);

        /* drop all and save all */
        if (!(sc | ec)) {
            res->start = p1;
            res->end = p2;
            return 0;
        } else if(sc & ec) {
            return 1;
        }
        
        /* clip line(calcute new p1 and new p2) 
         * accord to sequence: left -> right -> top -> bottom
         * actually whatever sequence you choose, the result is same.
         */
        code = sc ? sc : ec;
        p_tmp = (code == sc) ? &p1 : &p2;

        if (code & pos_rel_code3) {         /* left */
            p_tmp->y = p1.y + (p2.y - p1.y) * (aabb->start.x - p1.x) / (p2.x - p1.x);
            p_tmp->y = IPGUI_ROUND(p_tmp->y);
            p_tmp->x = aabb->start.x;
        } else if (code & pos_rel_code5) {   /* right */
            p_tmp->y = p1.y + (p2.y - p1.y) * (aabb->end.x - p1.x) / (p2.x - p1.x);
            p_tmp->y = IPGUI_ROUND(p_tmp->y);
            p_tmp->x = aabb->end.x;
        } else if (code & pos_rel_code7) {   /* bottom */
            p_tmp->x = p1.x + (p2.x - p1.x) * (aabb->end.y - p1.y) / (p2.y - p1.y);
            p_tmp->x = IPGUI_ROUND(p_tmp->x);
            p_tmp->y = aabb->end.y;
        } else if (code & pos_rel_code1) {   /* top */
            p_tmp->x = p1.x + (p2.x - p1.x) * (aabb->start.y - p1.y) / (p2.y - p1.y);
            p_tmp->x = IPGUI_ROUND(p_tmp->x);
            p_tmp->y = aabb->start.y;
        }
    }
}
