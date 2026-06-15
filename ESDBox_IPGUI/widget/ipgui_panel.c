#include "ipgui_panel.h"
#include "ipgui_prim.h"
#include "ipgui_color.h"
#include "ipgui_widget_tree.h"

static void panel_render(ipgui_widget_t * w, ipgui_widget_render_ctx_t * ctx)
{
    ipgui_panel_t * pn = (ipgui_panel_t *)w;

    ipgui_aabb_t box;
    box.start.x = 0;
    box.start.y = 0;
    box.end.x   = w->w - 1;
    box.end.y   = w->h - 1;

    ipgui_draw_box_background(ctx->surf, (ipgui_aabb_t *)0, &box, &pn->style.shape, &pn->style.bg);
    ipgui_draw_box_border(ctx->surf, (ipgui_aabb_t *)0, &box, &pn->style.shape, &pn->style.border);

    /* title bar */
    if (pn->style.title) {
        int title_h = 30;
        if (title_h > w->h) title_h = w->h;
        ipgui_aabb_t title_bar = {{0, 0}, {w->w - 1, title_h - 1}};
        ipgui_box_bg_style_t title_bg;
        title_bg.paint.type = IPGUI_PAINT_COLOR;
        title_bg.paint.src.color = pn->style.bg.paint.src.color;
        title_bg.paint.src.color.r = (u8_t)IPGUI_MAX(0, title_bg.paint.src.color.r - 20);
        title_bg.paint.src.color.g = (u8_t)IPGUI_MAX(0, title_bg.paint.src.color.g - 20);
        title_bg.paint.src.color.b = (u8_t)IPGUI_MAX(0, title_bg.paint.src.color.b - 20);
        title_bg.opacity = 255;
        title_bg.blend_mode = IPGUI_BLEND_NORMAL;
        ipgui_draw_box_background(ctx->surf, (ipgui_aabb_t *)0, &title_bar, &pn->style.shape, &title_bg);
    }
}

__IPGUI_API__ void ipgui_panel_style_init(ipgui_panel_style_t * s)
{
    if (!s) return;
    s->shape = (ipgui_box_style_t){0};
    s->shape.left_top_radius = s->shape.right_top_radius =
        s->shape.left_bottom_radius = s->shape.right_bottom_radius = 6;

    s->bg.paint.type = IPGUI_PAINT_COLOR;
    IPGUI_COLOR_SET(s->bg.paint.src.color, 255, 0xF5F5F5);
    s->bg.opacity = 255;
    s->bg.blend_mode = IPGUI_BLEND_NORMAL;

    s->border.paint.type = IPGUI_PAINT_COLOR;
    IPGUI_COLOR_SET(s->border.paint.src.color, 255, 0xDDDDDD);
    s->border.opacity = 255;
    s->border.width   = 1;
    s->border.blend_mode = IPGUI_BLEND_NORMAL;

    s->title = (const char *)0;
    IPGUI_COLOR_SET(s->title_color, 255, 0x333333);
}

__IPGUI_API__ ipgui_panel_t * ipgui_panel_create(ipgui_widget_t * parent)
{
    ipgui_panel_t * pn = (ipgui_panel_t *)ipgui_widget_create(parent);
    if (!pn) return (ipgui_panel_t *)0;
    ipgui_panel_style_init(&pn->style);
    ipgui_widget_set_render(&pn->base, panel_render);
    return pn;
}
