#include "ipgui_segmented.h"
#include "ipgui_prim.h"
#include "ipgui_color.h"
#include "ipgui_widget_tree.h"

static void seg_render(ipgui_widget_t * w, ipgui_widget_render_ctx_t * ctx)
{
    ipgui_segmented_t * sg = (ipgui_segmented_t *)w;
    int n = sg->style.segments;
    if (n < 2) n = 2;
    if (n > 8) n = 8;

    int seg_w = w->w / n;

    ipgui_aabb_t box;
    box.start.x = 0; box.start.y = 0;
    box.end.x = w->w - 1; box.end.y = w->h - 1;

    ipgui_draw_box_background(ctx->surf, 0, &box, &sg->style.shape, &sg->style.bg);
    ipgui_draw_box_border(ctx->surf, 0, &box, &sg->style.shape, &sg->style.border);

    /* selected segment highlight */
    {
        int sx = sg->style.selected * seg_w;
        ipgui_box_style_t ss = (ipgui_box_style_t){0};
        ss.left_top_radius = sg->style.shape.left_top_radius;
        ss.right_top_radius = sg->style.shape.right_top_radius;
        ss.left_bottom_radius = sg->style.shape.left_bottom_radius;
        ss.right_bottom_radius = sg->style.shape.right_bottom_radius;
        ipgui_aabb_t seg = {{sx + 1, 1}, {sx + seg_w - 2, w->h - 2}};
        ipgui_draw_box_background(ctx->surf, 0, &seg, &ss, &sg->style.sel_bg);
    }

    /* dividers between segments */
    int i;
    for (i = 1; i < n; i++) {
        int dx = i * seg_w;
        ipgui_aabb_t div = {{dx - 1, w->h / 5}, {dx, w->h * 4 / 5}};
        ipgui_box_style_t ds = {0};
        ipgui_box_bg_style_t db;
        db.paint.type = IPGUI_PAINT_COLOR;
        db.paint.src.color = sg->style.border.paint.src.color;
        db.paint.src.color.a = 80;
        db.opacity = 255;
        db.blend_mode = IPGUI_BLEND_NORMAL;
        ipgui_draw_box_background(ctx->surf, 0, &div, &ds, &db);
    }
}

__IPGUI_API__ void ipgui_segmented_style_init(ipgui_segmented_style_t * s)
{
    if (!s) return;
    s->shape = (ipgui_box_style_t){0};
    s->shape.left_top_radius = s->shape.right_top_radius =
        s->shape.left_bottom_radius = s->shape.right_bottom_radius = 6;

    s->bg.paint.type = IPGUI_PAINT_COLOR;
    IPGUI_COLOR_SET(s->bg.paint.src.color, 255, 0xEEEEEE);
    s->bg.opacity = 255;
    s->bg.blend_mode = IPGUI_BLEND_NORMAL;

    s->border.paint.type = IPGUI_PAINT_COLOR;
    IPGUI_COLOR_SET(s->border.paint.src.color, 255, 0xCCCCCC);
    s->border.opacity = 255;
    s->border.width = 1;
    s->border.blend_mode = IPGUI_BLEND_NORMAL;

    s->sel_bg.paint.type = IPGUI_PAINT_COLOR;
    IPGUI_COLOR_SET(s->sel_bg.paint.src.color, 255, 0xFFFFFF);
    s->sel_bg.opacity = 255;
    s->sel_bg.blend_mode = IPGUI_BLEND_NORMAL;

    s->selected = 0;
    s->segments = 3;
}

__IPGUI_API__ ipgui_segmented_t * ipgui_segmented_create(ipgui_widget_t * parent)
{
    ipgui_segmented_t * sg = (ipgui_segmented_t *)ipgui_widget_create(parent);
    if (!sg) return (ipgui_segmented_t *)0;
    ipgui_segmented_style_init(&sg->style);
    ipgui_widget_set_render(&sg->base, seg_render);
    return sg;
}
