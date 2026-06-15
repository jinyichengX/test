#include "ipgui_textbox.h"
#include "ipgui_prim.h"
#include "ipgui_color.h"
#include "ipgui_widget_tree.h"

static void textbox_render(ipgui_widget_t * w, ipgui_widget_render_ctx_t * ctx)
{
    ipgui_textbox_t * tb = (ipgui_textbox_t *)w;
    ipgui_aabb_t box;
    box.start.x = 0; box.start.y = 0;
    box.end.x = w->w - 1; box.end.y = w->h - 1;

    ipgui_draw_box_background(ctx->surf, (ipgui_aabb_t *)0, &box, &tb->style.shape, &tb->style.bg);
    ipgui_draw_box_border(ctx->surf, (ipgui_aabb_t *)0, &box, &tb->style.shape, &tb->style.border);

    /* placeholder indicator — 圆角灰色条模拟文字占位 */
    if (!tb->style.text && tb->style.placeholder) {
        int px = 8, pw = w->w / 2, ph = IPGUI_MAX(3, w->h / 10);
        int py = w->h / 2 - ph / 2;
        ipgui_box_style_t ps = {0};
        ps.left_top_radius = ps.right_top_radius =
            ps.left_bottom_radius = ps.right_bottom_radius = ph / 2;
        ipgui_box_bg_style_t pb;
        pb.paint.type = IPGUI_PAINT_COLOR;
        pb.paint.src.color = tb->style.placeholder_color;
        pb.opacity = 150;
        pb.blend_mode = IPGUI_BLEND_NORMAL;
        ipgui_aabb_t bar = {{px, py}, {px + pw - 1, py + ph - 1}};
        ipgui_draw_box_background(ctx->surf, 0, &bar, &ps, &pb);
    } else if (tb->style.text) {
        int px = 8, pw = w->w - 30, ph = IPGUI_MAX(3, w->h / 10);
        int py = w->h / 2 - ph / 2;
        ipgui_box_style_t ps = {0};
        ps.left_top_radius = ps.right_top_radius =
            ps.left_bottom_radius = ps.right_bottom_radius = ph / 2;
        ipgui_box_bg_style_t pb;
        pb.paint.type = IPGUI_PAINT_COLOR;
        pb.paint.src.color = tb->style.text_color;
        pb.opacity = 200;
        pb.blend_mode = IPGUI_BLEND_NORMAL;
        ipgui_aabb_t bar = {{px, py}, {px + pw - 1, py + ph - 1}};
        ipgui_draw_box_background(ctx->surf, 0, &bar, &ps, &pb);
    }
}

__IPGUI_API__ void ipgui_textbox_style_init(ipgui_textbox_style_t * s)
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
    s->border.width   = 1;
    s->border.blend_mode = IPGUI_BLEND_NORMAL;

    s->text = (const char *)0;
    IPGUI_COLOR_SET(s->text_color, 255, 0x333333);
    s->placeholder = (const char *)0;
    IPGUI_COLOR_SET(s->placeholder_color, 255, 0xAAAAAA);
}

__IPGUI_API__ ipgui_textbox_t * ipgui_textbox_create(ipgui_widget_t * parent)
{
    ipgui_textbox_t * tb = (ipgui_textbox_t *)ipgui_widget_create(parent);
    if (!tb) return (ipgui_textbox_t *)0;
    ipgui_textbox_style_init(&tb->style);
    ipgui_widget_set_render(&tb->base, textbox_render);
    return tb;
}
