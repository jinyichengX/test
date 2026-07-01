#include "ipgui_draw_icon.h"

/* get icon aabb(absolute coordinate on surf) */
__IPGUI_API__ ipgui_aabb_t ipgui_locate_icon(
    ipgui_point_t * pivot,    /* 相对于图标的变换点 如果是子图标那么就是相对于子图标的 */
    ipgui_point_t * anchor,
    ipgui_coord_t   icon_w,
    ipgui_coord_t   icon_h)
{
    ipgui_aabb_t icon_aabb;
    icon_aabb.start.x = anchor->x - pivot->x;
    icon_aabb.start.y = anchor->y - pivot->y;
    icon_aabb.end.x   = icon_aabb.start.x + icon_w - 1;
    icon_aabb.end.y   = icon_aabb.start.y + icon_h - 1;

    return icon_aabb;
}

__IPGUI_API__ void ipgui_draw_icon(
    ipgui_surf_t             * surf,
    ipgui_aabb_t             * clip,
    ipgui_icon_data_t        * icon_data,
    ipgui_point_t            * pivot,    /* 相对于图标的变换点 如果是子图标那么就是相对于子图标的 */
    ipgui_point_t            * anchor,
    ipgui_trans_mat_t        * trans,
    ipgui_draw_icon_style_t  * style)
{
    if ((!surf) || (!icon_data) || (!pivot) || (!anchor) || (!style))
        return;

    if (style->opacity < 3)
        return;

    if (!trans) {
        ipgui_aabb_t icon_aabb;
        /* get icon aabb(absolute coordinate on surf) */
        icon_aabb = ipgui_locate_icon(
            pivot,
            anchor,
            icon_data->w,
            icon_data->h);

        ipgui_blend(
            surf,
            clip,
            &icon_aabb,
            &style->paint,
            style->opacity,
            icon_data->mask,
            &icon_aabb,
            style->blend_mode);

        return;
    }
}
