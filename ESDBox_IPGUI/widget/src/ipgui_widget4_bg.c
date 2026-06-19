#include "ipgui_draw_image_api.h"
#include "ipgui_widget.h"
#include "ipgui_blend.h"
#include "ipgui_pattle.h"
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