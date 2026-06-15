#include "ipgui_radio.h"
#include "ipgui_prim.h"
#include "ipgui_color.h"
#include "ipgui_widget_tree.h"

static void radio_render(ipgui_widget_t * w, ipgui_widget_render_ctx_t * ctx)
{
    ipgui_radio_t * rb = (ipgui_radio_t *)w;
    ipgui_aabb_t box;
    box.start.x = 0; box.start.y = 0;
    box.end.x = w->w - 1; box.end.y = w->h - 1;

    ipgui_draw_box_background(ctx->surf, (ipgui_aabb_t *)0, &box, &rb->style.shape, &rb->style.bg);
    ipgui_draw_box_border(ctx->surf, (ipgui_aabb_t *)0, &box, &rb->style.shape, &rb->style.border);

    /* selected dot — 圆角小圆点 */
    if (rb->style.selected) {
        int cx = w->w / 2, cy = w->h / 2, r2 = w->w / 4;
        ipgui_box_style_t ds = {0};
        ds.left_top_radius = ds.right_top_radius =
            ds.left_bottom_radius = ds.right_bottom_radius = r2;
        ipgui_box_bg_style_t db;
        db.paint.type = IPGUI_PAINT_COLOR;
        db.paint.src.color = rb->style.dot_color;
        db.opacity = 255;
        db.blend_mode = IPGUI_BLEND_NORMAL;
        ipgui_aabb_t dot = {{cx - r2, cy - r2}, {cx + r2 - 1, cy + r2 - 1}};
        ipgui_draw_box_background(ctx->surf, 0, &dot, &ds, &db);
    }
}

__IPGUI_API__ void ipgui_radio_style_init(ipgui_radio_style_t * s)
{
    if (!s) return;
    s->shape = (ipgui_box_style_t){0};
    s->shape.left_top_radius = s->shape.right_top_radius =
        s->shape.left_bottom_radius = s->shape.right_bottom_radius =
        IPGUI_MAX(0, IPGUI_MIN(20, 20));
    s->bg.paint.type = IPGUI_PAINT_COLOR;
    IPGUI_COLOR_SET(s->bg.paint.src.color, 255, 0xFFFFFF);
    s->bg.opacity = 255;
    s->bg.blend_mode = IPGUI_BLEND_NORMAL;
    s->border.paint.type = IPGUI_PAINT_COLOR;
    IPGUI_COLOR_SET(s->border.paint.src.color, 255, 0xBBBBBB);
    s->border.opacity = 255;
    s->border.width   = 2;
    s->border.blend_mode = IPGUI_BLEND_NORMAL;
    IPGUI_COLOR_SET(s->dot_color, 255, 0x4080FF);
    s->selected = 0;
    s->text     = (const char *)0;
    IPGUI_COLOR_SET(s->text_color, 255, 0x333333);
}

__IPGUI_API__ ipgui_radio_t * ipgui_radio_create(ipgui_widget_t * parent)
{
    ipgui_radio_t * rb = (ipgui_radio_t *)ipgui_widget_create(parent);
    if (!rb) return (ipgui_radio_t *)0;
    ipgui_radio_style_init(&rb->style);
    ipgui_widget_set_render(&rb->base, radio_render);
    return rb;
}
