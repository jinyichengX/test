#include "ipgui_checkbox.h"
#include "ipgui_prim.h"
#include "ipgui_color.h"
#include "ipgui_widget_tree.h"

static void cb_render(ipgui_widget_t * w, ipgui_widget_render_ctx_t * ctx)
{
    ipgui_checkbox_t * cb = (ipgui_checkbox_t *)w;
    ipgui_aabb_t box;
    box.start.x = 0; box.start.y = 0;
    box.end.x = w->w - 1; box.end.y = w->h - 1;

    ipgui_draw_box_background(ctx->surf, (ipgui_aabb_t *)0, &box, &cb->style.shape, &cb->style.bg);
    ipgui_draw_box_border(ctx->surf, (ipgui_aabb_t *)0, &box, &cb->style.shape, &cb->style.border);

    /* checkmark: 用像素点拼出 ✓ */
    if (cb->style.checked) {
        ipgui_box_style_t ps = {0};
        ps.left_top_radius = ps.right_top_radius = ps.left_bottom_radius = ps.right_bottom_radius = 1;
        ipgui_box_bg_style_t pb;
        pb.paint.type = IPGUI_PAINT_COLOR;
        pb.paint.src.color = cb->style.check_color;
        pb.opacity = 255;
        pb.blend_mode = IPGUI_BLEND_NORMAL;
        int sw = IPGUI_MAX(2, w->w / 9);
        int cw = w->w, ch = w->h;
        int i;
        for (i = 0; i < sw + 3; i++) {
            int dy = i * sw / 3;
            /* 左斜线 */
            {int x1 = cw / 4 + i - dy / 2, y1 = ch / 2 + dy - i / 2;
             ipgui_aabb_t r = {{x1, y1}, {x1 + 1, y1 + 1}};
             ipgui_draw_box_background(ctx->surf, 0, &r, &ps, &pb);}
            /* 右斜线 */
            {int x2 = cw / 2 + i + dy / 2, y2 = ch * 3 / 4 - dy + i / 2;
             ipgui_aabb_t r = {{x2, y2}, {x2 + 1, y2 + 1}};
             ipgui_draw_box_background(ctx->surf, 0, &r, &ps, &pb);}
            /* 中点 */
            {int x3 = cw / 2 + i, y3 = ch / 2 + i / 2;
             ipgui_aabb_t r = {{x3, y3}, {x3 + 1, y3 + 1}};
             ipgui_draw_box_background(ctx->surf, 0, &r, &ps, &pb);}
        }
    }
}

__IPGUI_API__ void ipgui_checkbox_style_init(ipgui_checkbox_style_t * s)
{
    if (!s) return;
    s->shape = (ipgui_box_style_t){0};
    s->shape.left_top_radius = s->shape.right_top_radius =
        s->shape.left_bottom_radius = s->shape.right_bottom_radius = 4;
    s->bg.paint.type = IPGUI_PAINT_COLOR;
    IPGUI_COLOR_SET(s->bg.paint.src.color, 255, 0xFFFFFF);
    s->bg.opacity = 255;
    s->bg.blend_mode = IPGUI_BLEND_NORMAL;
    s->border.paint.type = IPGUI_PAINT_COLOR;
    IPGUI_COLOR_SET(s->border.paint.src.color, 255, 0xBBBBBB);
    s->border.opacity = 255;
    s->border.width   = 2;
    s->border.blend_mode = IPGUI_BLEND_NORMAL;
    IPGUI_COLOR_SET(s->check_color, 255, 0x4080FF);
    s->checked = 0;
    s->text    = (const char *)0;
    IPGUI_COLOR_SET(s->text_color, 255, 0x333333);
}

__IPGUI_API__ ipgui_checkbox_t * ipgui_checkbox_create(ipgui_widget_t * parent)
{
    ipgui_checkbox_t * cb = (ipgui_checkbox_t *)ipgui_widget_create(parent);
    if (!cb) return (ipgui_checkbox_t *)0;
    ipgui_checkbox_style_init(&cb->style);
    ipgui_widget_set_render(&cb->base, cb_render);
    return cb;
}
