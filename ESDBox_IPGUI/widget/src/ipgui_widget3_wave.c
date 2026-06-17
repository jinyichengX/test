#include "ipgui_draw_image_api.h"
#include "ipgui_widget.h"
#include "ipgui_blend.h"
#include "ipgui_pattle.h"
#include "ipgui_image_geometry_transform.h"
#include "ipgui_draw_arc.h"
extern ipgui_image_data_t wave_img;

void widget3_render(struct ipgui_widget * widget, ipgui_widget_render_ctx_t * ctx)
{
    ipgui_image_draw_style_t style;
    style.blend_mode = IPGUI_BLEND_NORMAL;
    style.opacity = 200;

    ipgui_aabb_t target;
    target.start.x = 0;
    target.start.y = 250;
    target.end.x = 1600;
    target.end.y = 400;

    ipgui_draw_image_in_rect(
        ctx->surf,
        &wave_img,
        &target,
        IPGUI_IMG_ALIGN_CENTER,
        IPGUI_IMG_FIT_STRETCH,
        &style
    );

}

extern ipgui_image_data_t power_img;
void power_render(struct ipgui_widget * widget, ipgui_widget_render_ctx_t * ctx)
{
    ipgui_image_draw_style_t style;
    style.blend_mode = IPGUI_BLEND_NORMAL;
    style.opacity = 90;

    ipgui_draw_image_at(
    ctx->surf,
    &power_img,
    16,
    18,
    &style);

    ipgui_arc_t arc;
    arc.cx = 49;
    arc.cy = 49;
    arc.er = 45;
    arc.ir = arc.er - 2;
    arc.start = 0;
    arc.angle = 360;
    arc.dir = IPGUI_ARC_DRAW_DIR_CW;

    ipgui_arc_style_t arc_style;
    arc_style.paint.type = IPGUI_PAINT_COLOR;
    IPGUI_COLOR_SET(arc_style.paint.src.color, 255, 0xff4500);
    arc_style.opacity    = style.opacity;
    arc_style.blend_mode = style.blend_mode;

    ipgui_draw_arc(
        ctx->surf,
        (ipgui_aabb_t *) 0,
        &arc,
        &arc_style);
}