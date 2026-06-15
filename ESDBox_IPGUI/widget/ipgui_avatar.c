#include "ipgui_avatar.h"
#include "ipgui_prim.h"
#include "ipgui_color.h"
#include "ipgui_widget_tree.h"

static void av_render(ipgui_widget_t * w, ipgui_widget_render_ctx_t * ctx)
{
    ipgui_avatar_t * av = (ipgui_avatar_t *)w;
    ipgui_aabb_t box;
    box.start.x = 0; box.start.y = 0;
    box.end.x = w->w - 1; box.end.y = w->h - 1;

    ipgui_draw_box_background(ctx->surf, 0, &box, &av->style.shape, &av->style.bg);
    ipgui_draw_box_border(ctx->surf, 0, &box, &av->style.shape, &av->style.border);

    /* online status dot (bottom-right) */
    if (av->style.online > 0) {
        int dr = IPGUI_MAX(2, w->w / 8);
        int dx = w->w - dr - 2, dy = w->h - dr - 2;
        ipgui_box_style_t ds = {0};
        ds.left_top_radius = ds.right_top_radius =
            ds.left_bottom_radius = ds.right_bottom_radius = dr;
        ipgui_box_bg_style_t db;
        db.paint.type = IPGUI_PAINT_COLOR;
        db.paint.src.color = av->style.dot_color;
        db.opacity = 255;
        db.blend_mode = IPGUI_BLEND_NORMAL;
        ipgui_aabb_t d = {{dx, dy}, {dx + dr * 2 - 1, dy + dr * 2 - 1}};
        ipgui_draw_box_background(ctx->surf, 0, &d, &ds, &db);

        /* white ring around dot */
        ipgui_box_border_style_t brd = {0};
        brd.paint.type = IPGUI_PAINT_COLOR;
        IPGUI_COLOR_SET(brd.paint.src.color, 255, 0xFFFFFF);
        brd.opacity = 255;
        brd.width = 2;
        brd.blend_mode = IPGUI_BLEND_NORMAL;
        ipgui_draw_box_border(ctx->surf, 0, &d, &ds, &brd);
    }
}

__IPGUI_API__ void ipgui_avatar_style_init(ipgui_avatar_style_t * s)
{
    if (!s) return;
    s->shape = (ipgui_box_style_t){0};
    s->shape.left_top_radius = s->shape.right_top_radius =
        s->shape.left_bottom_radius = s->shape.right_bottom_radius = 24;

    s->bg.paint.type = IPGUI_PAINT_COLOR;
    IPGUI_COLOR_SET(s->bg.paint.src.color, 255, 0xD0D8E8);
    s->bg.opacity = 255;
    s->bg.blend_mode = IPGUI_BLEND_NORMAL;

    s->border = (ipgui_box_border_style_t){0};
    s->border.paint.type = IPGUI_PAINT_COLOR;
    IPGUI_COLOR_SET(s->border.paint.src.color, 255, 0xFFFFFF);
    s->border.opacity = 255;
    s->border.width = 3;
    s->border.blend_mode = IPGUI_BLEND_NORMAL;

    s->online = 0;
    IPGUI_COLOR_SET(s->dot_color, 255, 0x34C759);
}

__IPGUI_API__ ipgui_avatar_t * ipgui_avatar_create(ipgui_widget_t * parent)
{
    ipgui_avatar_t * av = (ipgui_avatar_t *)ipgui_widget_create(parent);
    if (!av) return (ipgui_avatar_t *)0;
    ipgui_avatar_style_init(&av->style);
    ipgui_widget_set_render(&av->base, av_render);
    return av;
}
