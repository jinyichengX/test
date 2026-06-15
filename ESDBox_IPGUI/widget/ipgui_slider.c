#include "ipgui_slider.h"
#include "ipgui_prim.h"
#include "ipgui_color.h"
#include "ipgui_widget_tree.h"

static void slider_render(ipgui_widget_t * w, ipgui_widget_render_ctx_t * ctx)
{
    ipgui_slider_t * sl = (ipgui_slider_t *)w;
    int track_h = IPGUI_MAX(4, w->h / 5);
    int track_y = (w->h - track_h) / 2;
    ipgui_box_style_t ts = {0};
    ts.left_top_radius = ts.right_top_radius =
        ts.left_bottom_radius = ts.right_bottom_radius = track_h / 2;

    /* track bg */
    {
        ipgui_aabb_t track = {{0, track_y}, {w->w - 1, track_y + track_h - 1}};
        ipgui_draw_box_background(ctx->surf, 0, &track, &ts, &sl->style.track_bg);
        ipgui_draw_box_border(ctx->surf, 0, &track, &ts, &sl->style.track_border);
    }

    /* fill */
    {
        int range = sl->style.max - sl->style.min;
        int fill_w = range > 0 ? (sl->style.value - sl->style.min) * w->w / range : 0;
        if (fill_w > 0) {
            ipgui_aabb_t fill = {{0, track_y}, {fill_w - 1, track_y + track_h - 1}};
            ipgui_draw_box_background(ctx->surf, 0, &fill, &ts, &sl->style.fill_bg);
        }
    }

    /* knob */
    {
        int range = sl->style.max - sl->style.min;
        int knob_x = range > 0 ? (sl->style.value - sl->style.min) * (w->w - w->h) / range : 0;
        int kw = w->h, kh = w->h;
        ipgui_aabb_t knob = {{knob_x, 0}, {knob_x + kw - 1, kh - 1}};
        ipgui_draw_box_background(ctx->surf, 0, &knob, &sl->style.knob_shape, &sl->style.knob_bg);
        ipgui_draw_box_border(ctx->surf, 0, &knob, &sl->style.knob_shape, &sl->style.knob_border);
    }
}

__IPGUI_API__ void ipgui_slider_style_init(ipgui_slider_style_t * s)
{
    if (!s) return;
    s->shape = (ipgui_box_style_t){0};
    s->shape.left_top_radius = s->shape.right_top_radius =
        s->shape.left_bottom_radius = s->shape.right_bottom_radius = 4;

    s->track_bg.paint.type = IPGUI_PAINT_COLOR;
    IPGUI_COLOR_SET(s->track_bg.paint.src.color, 255, 0xE8E8E8);
    s->track_bg.opacity = 255;
    s->track_bg.blend_mode = IPGUI_BLEND_NORMAL;

    s->track_border.opacity = 0;

    s->fill_bg.paint.type = IPGUI_PAINT_COLOR;
    IPGUI_COLOR_SET(s->fill_bg.paint.src.color, 255, 0x4080FF);
    s->fill_bg.opacity = 255;
    s->fill_bg.blend_mode = IPGUI_BLEND_NORMAL;

    s->knob_shape = (ipgui_box_style_t){0};
    s->knob_shape.left_top_radius = s->knob_shape.right_top_radius =
        s->knob_shape.left_bottom_radius = s->knob_shape.right_bottom_radius = 12;

    s->knob_bg.paint.type = IPGUI_PAINT_COLOR;
    IPGUI_COLOR_SET(s->knob_bg.paint.src.color, 255, 0xFFFFFF);
    s->knob_bg.opacity = 255;
    s->knob_bg.blend_mode = IPGUI_BLEND_NORMAL;

    s->knob_border.paint.type = IPGUI_PAINT_COLOR;
    IPGUI_COLOR_SET(s->knob_border.paint.src.color, 255, 0x4080FF);
    s->knob_border.opacity = 255;
    s->knob_border.width   = 2;
    s->knob_border.blend_mode = IPGUI_BLEND_NORMAL;

    s->min   = 0;
    s->max   = 100;
    s->value = 30;
}

__IPGUI_API__ ipgui_slider_t * ipgui_slider_create(ipgui_widget_t * parent)
{
    ipgui_slider_t * sl = (ipgui_slider_t *)ipgui_widget_create(parent);
    if (!sl) return (ipgui_slider_t *)0;
    ipgui_slider_style_init(&sl->style);
    ipgui_widget_set_render(&sl->base, slider_render);
    return sl;
}
