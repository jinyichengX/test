#include "ipgui_dropdown.h"
#include "ipgui_prim.h"
#include "ipgui_color.h"
#include "ipgui_widget_tree.h"

static void dd_render(ipgui_widget_t * w, ipgui_widget_render_ctx_t * ctx)
{
    ipgui_dropdown_t * dd = (ipgui_dropdown_t *)w;
    ipgui_aabb_t box;
    box.start.x = 0; box.start.y = 0;
    box.end.x = w->w - 1; box.end.y = w->h - 1;

    ipgui_draw_box_background(ctx->surf, (ipgui_aabb_t *)0, &box, &dd->style.shape, &dd->style.bg);
    ipgui_draw_box_border(ctx->surf, (ipgui_aabb_t *)0, &box, &dd->style.shape, &dd->style.border);

    /* arrow ▼ — 逐行递减的矩形模拟三角箭头 */
    {
        int asz = 12, ax = w->w - asz - 12, ay = (w->h - asz / 2) / 2;
        ipgui_box_style_t ps = {0};
        ps.left_top_radius = ps.right_top_radius =
            ps.left_bottom_radius = ps.right_bottom_radius = 1;
        ipgui_box_bg_style_t pb;
        pb.paint.type = IPGUI_PAINT_COLOR;
        pb.paint.src.color = dd->style.arrow_color;
        pb.opacity = 255;
        pb.blend_mode = IPGUI_BLEND_NORMAL;
        int h = IPGUI_MAX(3, asz * 2 / 3);
        int row;
        for (row = 0; row < h; row++) {
            int rw = asz - row * asz / h;
            if (rw < 2) rw = 2;
            int lx = ax + (asz - rw) / 2;
            ipgui_aabb_t rr = {{lx, ay + row}, {lx + rw - 1, ay + row}};
            ipgui_draw_box_background(ctx->surf, 0, &rr, &ps, &pb);
        }
    }

    /* expand dropdown list */
    if (dd->style.expanded && dd->style.opt_count > 0) {
        int ih = 30;
        int i;
        for (i = 0; i < dd->style.opt_count && i < 16; i++) {
            int iy = w->h + i * ih;
            ipgui_aabb_t ib = {{1, iy}, {w->w - 2, iy + ih - 1}};
            ipgui_box_bg_style_t ibg;
            ibg.paint.type = IPGUI_PAINT_COLOR;
            IPGUI_COLOR_SET(ibg.paint.src.color, 255, 0xFFFFFF);
            ibg.opacity = 255;
            ibg.blend_mode = IPGUI_BLEND_NORMAL;
            ipgui_draw_box_background(ctx->surf, 0, &ib, &dd->style.shape, &ibg);
            if (i == dd->style.selected) {
                ipgui_box_bg_style_t sb;
                sb.paint.type = IPGUI_PAINT_COLOR;
                IPGUI_COLOR_SET(sb.paint.src.color, 255, 0x4080FF);
                sb.opacity = 40;
                sb.blend_mode = IPGUI_BLEND_NORMAL;
                ipgui_draw_box_background(ctx->surf, 0, &ib, &dd->style.shape, &sb);
            }
        }
    }
}

__IPGUI_API__ void ipgui_dropdown_style_init(ipgui_dropdown_style_t * s)
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

    IPGUI_COLOR_SET(s->text_color, 255, 0x333333);
    IPGUI_COLOR_SET(s->arrow_color, 255, 0x999999);
    s->value      = (const char *)0;
    s->opt_count  = 0;
    s->selected   = -1;
    s->expanded   = 0;
}

__IPGUI_API__ ipgui_dropdown_t * ipgui_dropdown_create(ipgui_widget_t * parent)
{
    ipgui_dropdown_t * dd = (ipgui_dropdown_t *)ipgui_widget_create(parent);
    if (!dd) return (ipgui_dropdown_t *)0;
    ipgui_dropdown_style_init(&dd->style);
    ipgui_widget_set_render(&dd->base, dd_render);
    return dd;
}
