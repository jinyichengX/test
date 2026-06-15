#include "ipgui_progressbar.h"
#include "ipgui_prim.h"
#include "ipgui_color.h"
#include "ipgui_widget_tree.h"

static void pb_render(ipgui_widget_t * w, ipgui_widget_render_ctx_t * ctx)
{
    ipgui_progressbar_t * pb = (ipgui_progressbar_t *)w;
    ipgui_aabb_t box;
    box.start.x = 0; box.start.y = 0;
    box.end.x = w->w - 1; box.end.y = w->h - 1;

    /* track 使用圆角 */
    ipgui_draw_box_background(ctx->surf, (ipgui_aabb_t *)0, &box, &pb->style.shape, &pb->style.track_bg);
    ipgui_draw_box_border(ctx->surf, (ipgui_aabb_t *)0, &box, &pb->style.shape, &pb->style.track_border);

    /* fill */
    if (pb->style.value > 0 && pb->style.max > 0) {
        int fill_w = pb->style.value * w->w / pb->style.max;
        if (fill_w > 0) {
            ipgui_aabb_t fill = {{0, 0}, {fill_w - 1, w->h - 1}};
            ipgui_draw_box_background(ctx->surf, (ipgui_aabb_t *)0, &fill, &pb->style.shape, &pb->style.fill_bg);
        }
    }
}

__IPGUI_API__ void ipgui_progressbar_style_init(ipgui_progressbar_style_t * s)
{
    if (!s) return;
    s->shape = (ipgui_box_style_t){0};
    s->shape.left_top_radius = s->shape.right_top_radius =
        s->shape.left_bottom_radius = s->shape.right_bottom_radius = 6;

    s->track_bg.paint.type = IPGUI_PAINT_COLOR;
    IPGUI_COLOR_SET(s->track_bg.paint.src.color, 255, 0xEBEBEB);
    s->track_bg.opacity = 255;
    s->track_bg.blend_mode = IPGUI_BLEND_NORMAL;

    s->track_border.paint.type = IPGUI_PAINT_COLOR;
    IPGUI_COLOR_SET(s->track_border.paint.src.color, 255, 0xD5D5D5);
    s->track_border.opacity = 255;
    s->track_border.width   = 1;
    s->track_border.blend_mode = IPGUI_BLEND_NORMAL;

    s->fill_bg.paint.type = IPGUI_PAINT_COLOR;
    IPGUI_COLOR_SET(s->fill_bg.paint.src.color, 255, 0x4080FF);
    s->fill_bg.opacity = 255;
    s->fill_bg.blend_mode = IPGUI_BLEND_NORMAL;

    s->fill_border.opacity = 0;
    s->value = 40;
    s->max   = 100;
}

__IPGUI_API__ ipgui_progressbar_t * ipgui_progressbar_create(ipgui_widget_t * parent)
{
    ipgui_progressbar_t * pb = (ipgui_progressbar_t *)ipgui_widget_create(parent);
    if (!pb) return (ipgui_progressbar_t *)0;
    ipgui_progressbar_style_init(&pb->style);
    ipgui_widget_set_render(&pb->base, pb_render);
    return pb;
}
