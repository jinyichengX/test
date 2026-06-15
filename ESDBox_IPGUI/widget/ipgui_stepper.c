#include "ipgui_stepper.h"
#include "ipgui_prim.h"
#include "ipgui_color.h"
#include "ipgui_widget_tree.h"

static void st_render(ipgui_widget_t * w, ipgui_widget_render_ctx_t * ctx)
{
    ipgui_stepper_t * st = (ipgui_stepper_t *)w;
    int btn_w = w->h;  /* 方形按钮 */

    ipgui_aabb_t box;
    box.start.x = 0; box.start.y = 0;
    box.end.x = w->w - 1; box.end.y = w->h - 1;

    /* bg */
    ipgui_draw_box_background(ctx->surf, 0, &box, &st->style.shape, &st->style.bg);
    ipgui_draw_box_border(ctx->surf, 0, &box, &st->style.shape, &st->style.border);

    /* minus button */
    {
        ipgui_aabb_t mb = {{1, 1}, {btn_w - 1, w->h - 2}};
        ipgui_box_bg_style_t bbg;
        bbg.paint.type = IPGUI_PAINT_COLOR;
        bbg.paint.src.color = st->style.btn_color;
        bbg.opacity = 255;
        bbg.blend_mode = IPGUI_BLEND_NORMAL;
        ipgui_draw_box_background(ctx->surf, 0, &mb, &st->style.shape, &bbg);

        /* minus bar */
        int my = w->h / 2, ml = btn_w / 4, mr = btn_w * 3 / 4;
        ipgui_aabb_t bar = {{ml, my - 1}, {mr, my + 1}};
        ipgui_blend_color(ctx->surf, 0, &bar, st->style.btn_text, 255, 0, 0, IPGUI_BLEND_NORMAL);
    }

    /* plus button */
    {
        ipgui_aabb_t pb = {{w->w - btn_w, 1}, {w->w - 2, w->h - 2}};
        ipgui_box_bg_style_t bbg;
        bbg.paint.type = IPGUI_PAINT_COLOR;
        bbg.paint.src.color = st->style.btn_color;
        bbg.opacity = 255;
        bbg.blend_mode = IPGUI_BLEND_NORMAL;
        ipgui_draw_box_background(ctx->surf, 0, &pb, &st->style.shape, &bbg);

        /* + horizontal */
        int px = w->w - btn_w / 2, py = w->h / 2;
        int bl = btn_w / 4, br = btn_w * 3 / 4;
        ipgui_aabb_t bar_h = {{px - br + btn_w / 2, py - 1}, {px - bl + btn_w / 2, py + 1}};
        ipgui_blend_color(ctx->surf, 0, &bar_h, st->style.btn_text, 255, 0, 0, IPGUI_BLEND_NORMAL);
        ipgui_aabb_t bar_v = {{px - 1, py - br + btn_w / 2}, {px + 1, py - bl + btn_w / 2}};
        ipgui_blend_color(ctx->surf, 0, &bar_v, st->style.btn_text, 255, 0, 0, IPGUI_BLEND_NORMAL);
    }
}

__IPGUI_API__ void ipgui_stepper_style_init(ipgui_stepper_style_t * s)
{
    if (!s) return;
    s->shape = (ipgui_box_style_t){0};
    s->shape.left_top_radius = s->shape.right_top_radius =
        s->shape.left_bottom_radius = s->shape.right_bottom_radius = 6;

    s->bg.paint.type = IPGUI_PAINT_COLOR;
    IPGUI_COLOR_SET(s->bg.paint.src.color, 255, 0xFFFFFF);
    s->bg.opacity = 255;
    s->bg.blend_mode = IPGUI_BLEND_NORMAL;

    s->border.paint.type = IPGUI_PAINT_COLOR;
    IPGUI_COLOR_SET(s->border.paint.src.color, 255, 0xCCCCCC);
    s->border.opacity = 255;
    s->border.width = 1;
    s->border.blend_mode = IPGUI_BLEND_NORMAL;

    IPGUI_COLOR_SET(s->btn_color, 255, 0xF0F0F0);
    IPGUI_COLOR_SET(s->btn_text, 255, 0x333333);
    s->value = 0;
    s->min = 0;
    s->max = 99;
    s->step = 1;
}

__IPGUI_API__ ipgui_stepper_t * ipgui_stepper_create(ipgui_widget_t * parent)
{
    ipgui_stepper_t * st = (ipgui_stepper_t *)ipgui_widget_create(parent);
    if (!st) return (ipgui_stepper_t *)0;
    ipgui_stepper_style_init(&st->style);
    ipgui_widget_set_render(&st->base, st_render);
    return st;
}
