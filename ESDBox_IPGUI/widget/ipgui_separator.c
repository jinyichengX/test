#include "ipgui_separator.h"
#include "ipgui_prim.h"
#include "ipgui_color.h"
#include "ipgui_widget_tree.h"

static void sep_render(ipgui_widget_t * w, ipgui_widget_render_ctx_t * ctx)
{
    ipgui_separator_t * sep = (ipgui_separator_t *)w;
    int t = IPGUI_MAX(1, sep->style.thickness);

    if (sep->style.dir == IPGUI_SEP_VERTICAL) {
        int cx = (w->w - t) / 2;
        ipgui_aabb_t r = {{cx, 0}, {cx + t - 1, w->h - 1}};
        ipgui_box_style_t s = {0};
        ipgui_box_bg_style_t b;
        b.paint.type = IPGUI_PAINT_COLOR;
        b.paint.src.color = sep->style.color;
        b.opacity = 255;
        b.blend_mode = IPGUI_BLEND_NORMAL;
        ipgui_draw_box_background(ctx->surf, 0, &r, &s, &b);
    } else {
        int cy = (w->h - t) / 2;
        ipgui_aabb_t r = {{0, cy}, {w->w - 1, cy + t - 1}};
        ipgui_box_style_t s = {0};
        ipgui_box_bg_style_t b;
        b.paint.type = IPGUI_PAINT_COLOR;
        b.paint.src.color = sep->style.color;
        b.opacity = 255;
        b.blend_mode = IPGUI_BLEND_NORMAL;
        ipgui_draw_box_background(ctx->surf, 0, &r, &s, &b);
    }
}

__IPGUI_API__ void ipgui_separator_style_init(ipgui_separator_style_t * s)
{
    if (!s) return;
    s->dir = IPGUI_SEP_HORIZONTAL;
    IPGUI_COLOR_SET(s->color, 255, 0xDDDDDD);
    s->thickness = 1;
}

__IPGUI_API__ ipgui_separator_t * ipgui_separator_create(ipgui_widget_t * parent)
{
    ipgui_separator_t * sep = (ipgui_separator_t *)ipgui_widget_create(parent);
    if (!sep) return (ipgui_separator_t *)0;
    ipgui_separator_style_init(&sep->style);
    ipgui_widget_set_render(&sep->base, sep_render);
    return sep;
}
