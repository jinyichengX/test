#include "ipgui_draw_image_api.h"
#include "ipgui_widget.h"
#include "ipgui_blend.h"
#include "ipgui_pattle.h"
#include "ipgui_image_geometry_transform.h"
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

    // ipgui_draw_image_in_rect(
    //     ctx->surf,
    //     &wave_img,
    //     &target,
    //     IPGUI_IMG_ALIGN_CENTER,
    //     IPGUI_IMG_FIT_STRETCH,
    //     &style
    // );

    ipgui_img_geo_trans_t trans1;

    ipgui_point_t pivot2;
    pivot2.x = wave_img.w / 2;
    pivot2.y = wave_img.h / 2;
    ipgui_image_trans_init(&trans1);
    ipgui_image_trans_pivot(&trans1, pivot2);
    // ipgui_image_trans_scale(&trans1, sx, sy);
    // ipgui_image_trans_rotate_degree(&trans1, degree1);
    ipgui_point_t anchor2;
    anchor2.x = 400;
    anchor2.y = 240;
    ipgui_draw_image(
        ctx->surf,
        (ipgui_aabb_t *)0,
        &wave_img,
        &trans1.pivot,
        &anchor2,
        (ipgui_trans_mat_t *)0,
        &style
    );
}