#ifndef IPGUI_LINE_CLIP_H
#define IPGUI_LINE_CLIP_H
#include "ipgui_prim.h"

typedef struct {
    ipgui_scoord_t dx;/* +- */
    ipgui_scoord_t dy;/* +- */
}ipgui_line_slope_t;

typedef struct
{
    ipgui_spoint_t start;
    ipgui_line_slope_t line_slope;
}ipgui_sline_dsc_t;
void ipgui_gen_line_dsc(ipgui_spoint_t from, ipgui_spoint_t to, ipgui_sline_dsc_t * line_dsc);
__IPGUI_API__ int ipgui_line_clip_cohen(ipgui_aabb_t * aabb, ipgui_line_t * line, ipgui_line_t * res);
#endif

