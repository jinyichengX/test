#include "ipgui_draw_image_api.h"
#include "ipgui_widget.h"
#include "ipgui_blend.h"
#include "ipgui_pattle.h"
#include "ipgui_draw_filled_circle.h"
#include "ipgui_image_geometry_transform.h"
extern ipgui_image_data_t main_bg_img;

void widget4_render(struct ipgui_widget * widget, ipgui_widget_render_ctx_t * ctx)
{
    ipgui_image_draw_style_t style;
    style.blend_mode = IPGUI_BLEND_NORMAL;
    style.opacity = 170;

    ipgui_aabb_t target;
    target.start.x = 0;
    target.start.y = 250;
    target.end.x = 1600;
    target.end.y = 400;

    ipgui_draw_image_at(
        ctx->surf,
        &main_bg_img,
        0,
        0,
        &style
    );

    static ipgui_coord_t knob_cx = 73;
    static ipgui_coord_t knob_cy = 115;
    static ipgui_coord_t knob_r = 12;
    static ipgui_filled_circle_style_t knob_style;

    knob_style.opacity = 200;
    knob_style.blend_mode = IPGUI_BLEND_NORMAL;
    knob_style.paint.type = IPGUI_PAINT_COLOR;
    IPGUI_COLOR_SET(knob_style.paint.src.color, 255, IPGUI_COLOR_WHITE);
    ipgui_draw_filled_circle(
        ctx->surf,
        (ipgui_aabb_t *) 0,
        knob_cx,
        knob_cy,
        knob_r,
        &knob_style
    );

    // ipgui_point_t p;
    // p.x = 0;
    // p.y = 0;

    // ipgui_draw_image(
    //     ctx->surf,
    //     (ipgui_aabb_t *) 0,
    //     &main_bg_img,
    //     &p,&p,
    //     (ipgui_trans_mat_t *) 0,
    //     &style
    // );

}