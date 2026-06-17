#include "ipgui_draw_filled_circle.h"

__IPGUI_API__ void ipgui_draw_filled_circle(
    ipgui_surf_t                * surf,
    ipgui_aabb_t                * clip,
    ipgui_coord_t                 cx, 
    ipgui_coord_t                 cy, 
    ipgui_coord_t                 r,
    ipgui_filled_circle_style_t * style)
{
    ipgui_arc_t arc;
    arc.cx = cx;
    arc.cy = cy;
    arc.er = r;
    arc.ir = 0;
    arc.start = 0;
    arc.angle = 360;
    arc.dir = IPGUI_ARC_DRAW_DIR_CW;

    ipgui_arc_style_t arc_style;
    arc_style.paint      = style->paint;
    arc_style.opacity    = style->opacity;
    arc_style.blend_mode = style->blend_mode;

    ipgui_draw_arc(
        surf,
        clip,
        &arc,
        &arc_style);
}