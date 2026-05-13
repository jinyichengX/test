#include "ipgui_stroker.h"

/* this file has one mission: generate stroked polygon edge from path */


typedef struct {
    ipgui_scoord_t dx;
    ipgui_scoord_t dy;
}ipgui_slope_t;

/* 描边面/横截面 */
typedef struct {
    /*
    (middle)--------> ccw(vector)
        | 
        |
        |
        |
        | (line)
        |
        |
        |
        v
        point_ccw.x = middle + ccw.x;
        point_ccw.y = middle + ccw.y;
    */
    ipgui_svector_t ccw;
    ipgui_svector_t cw;
    ipgui_spoint_t middle; //原始线段的端点，也就是描边面的中点
    int endpoint; //0: the start surface of vector line, 1: end surface of vector line
}ipgui_cross_surface_t;

// void ipgui_stroker_cap_set(ipgui_stroker_dsc_t * stroker, ipgui_line_cap_t cap)
// {
//     stroker->cap = cap;
// }

void ipgui_stroker_join_set(ipgui_stroker_dsc_t * stroker, ipgui_line_join_t join)
{
    stroker->join = join;
}

static void ipgui_line_gen_expand_delta(
    ipgui_sline_dsc_t * line_dsc, ipgui_scoord_t width, 
    ipgui_scoord_t * dx, ipgui_scoord_t * dy)
{

}

/* add expanded sub egde from stroker parameter */
void ipgui_stroker_add_expanded_sub_edge(ipgui_stroker_dsc_t * stroker, ipgui_spoint_t from, ipgui_spoint_t to)
{

}

void ipgui_add_cap(void)
{

}

void ipgui_path_stroker(ipgui_stroker_dsc_t * stroker)
{

}

void ipgui_fill(void)
{

}

extern ipgui_scr_t * sdl_scr;

/* add line cap */
__IPGUI_API__ ipgui_err_t ipgui_stroker_add_cap(ipgui_stroker_dsc_t * stroker,
                ipgui_cross_surface_t * surface)//surface 和 line必须是对应的
{
    // switch (stroker->cap) {
    //     case IPGUI_LINE_CAP_SQUARE: {/* 方头 */
    //         /* 已知ccw 和 cw，再计算middle反向向外延长的vector */
    //         ipgui_svector_t v; //the square cap vector
    //         if (surface->endpoint == 0) /* 表明是线段向量的起始点 */
    //             ipgui_vector_rotate_screen(&surface->ccw, IPGUI_ANGLE_PI2, &v);
    //         else if (surface->endpoint == 1)
    //             ipgui_vector_rotate_screen(&surface->ccw, -IPGUI_ANGLE_PI2, &v);
    //     }
    //     break;
    //     case IPGUI_LINE_CAP_ROUND: {/* 圆头 */
            
    //         }
    //         break;
    //     case IPGUI_LINE_CAP_BUTT: /* 平头 */
    //     default: {
    //         // ipgui_polygon_add_edge(polygon, &surface->ccw, &surface->cw);
    //     }
    //         break;
    // }
}

__IPGUI_API__ ipgui_err_t ipgui_stroker_path_line_to(ipgui_stroker_dsc_t * stroker, ipgui_spoint_t from, ipgui_spoint_t to)
{
    ipgui_svector_t lv;
    lv.x = to.x - from.x; //dx
    lv.y = to.y - from.y; //dy

    ipgui_scoord_t half_width; 

    int mod_int, mod_frac, vec_mod;
    ipgui_int_sqrt(lv.x * lv.x + lv.y * lv.y, &mod_int, &mod_frac);
    vec_mod  = mod_int * 1000 + mod_frac;
    half_width = stroker->width * 500;
    
    /* 计算描边面 */
    ipgui_cross_surface_t sur_start;
    ipgui_cross_surface_t sur_end;
    int frac;
    ipgui_scoord_t dx, dy;
    dy = (long long)((long long)(-lv.x) * (long long)half_width) / vec_mod;
    frac = (long long)((long long)(-lv.x) * (long long)half_width) % vec_mod;
    if (IPGUI_ABS(frac) > (vec_mod >> 1)) {
        if (dy < 0) dy --;
        else if (dy > 0) dy ++;
    }
    dx = (long long)((long long)lv.y * (long long)half_width) / vec_mod;
    frac = (long long)((long long)lv.y * (long long)half_width) % vec_mod;
    if (IPGUI_ABS(frac) > (vec_mod >> 1)) {
        if (dx < 0) dx --;
        else if (dx > 0) dx ++;
    }
    sur_start.middle = from;
    sur_start.ccw.x = dx;
    sur_start.ccw.y = dy;

    sur_end.middle = to;
    sur_end.ccw.x = dx;
    sur_end.ccw.y = dy;

    dx = -dx; 
    dy = -dy;
    
    sur_start.cw.x = dx;
    sur_start.cw.y = dy;
    sur_end.cw.x = dx;
    sur_end.cw.y = dy;

    sur_start.endpoint = 0;
    sur_end.endpoint = 1;

    ipgui_sline_t sline;
    sline.start = from;
    sline.end = to;

    ipgui_stroker_add_cap(stroker, &sur_start);
    ipgui_stroker_add_cap(stroker, &sur_end);
    /* now we get two edges, add edges into polygon */
    /* or add point into polygon*/
    // if (ipgui_polygon_add_edge(polygon, &sur_start.ccw, &sur_end.ccw))
    //     return IPGUI_ERR_NOK;
    // if (ipgui_polygon_add_edge(polygon, &sur_start.ccw, &sur_end.ccw))
    //     return IPGUI_ERR_NOK;
}

__IPGUI_API__ ipgui_err_t ipgui_stroker_add_join(ipgui_stroker_dsc_t * stroker, ipgui_cross_surface_t * s1, ipgui_cross_surface_t * s2)
{
    if (!(s1->middle.x == s2->middle.x && s1->middle.y == s2->middle.y))//两个描边面的中点必须相同
        return IPGUI_ERR_NOK;

    switch (stroker->join) {
        case IPGUI_LINE_JOIN_BEVEL: //平角连接
        break;

        case IPGUI_LINE_JOIN_MITER: //尖角连接
        break;

        case IPGUI_LINE_JOIN_ROUND: //圆角连接
        break;
    }
}
