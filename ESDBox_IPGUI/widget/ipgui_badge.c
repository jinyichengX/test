#include "ipgui_badge.h"
#include "ipgui_prim.h"
#include "ipgui_color.h"
#include "ipgui_widget_tree.h"

static void badge_render(ipgui_widget_t * w, ipgui_widget_render_ctx_t * ctx)
{
    ipgui_badge_t * bd = (ipgui_badge_t *)w;
    int cx = w->w / 2, cy = w->h / 2;
    int r = IPGUI_MIN(w->w, w->h) / 2;

    if (bd->style.type == IPGUI_BADGE_TYPE_RING) {
        /* outer dot */
        ipgui_box_style_t s = {0};
        s.left_top_radius = s.right_top_radius =
            s.left_bottom_radius = s.right_bottom_radius = r;
        ipgui_box_bg_style_t bg;
        bg.paint.type = IPGUI_PAINT_COLOR;
        bg.paint.src.color = bd->style.color;
        bg.opacity = 255;
        bg.blend_mode = IPGUI_BLEND_NORMAL;
        ipgui_aabb_t dot = {{cx - r, cy - r}, {cx + r - 1, cy + r - 1}};
        ipgui_draw_box_background(ctx->surf, 0, &dot, &s, &bg);

        /* inner cutout */
        int rw = IPGUI_MAX(1, bd->style.ring_width);
        int ir = r - rw;
        if (ir > 0) {
            ipgui_box_style_t is = {0};
            is.left_top_radius = is.right_top_radius =
                is.left_bottom_radius = is.right_bottom_radius = ir;
            ipgui_box_bg_style_t ib;
            ib.paint.type = IPGUI_PAINT_COLOR;
            ib.paint.src.color = bd->style.ring_color;
            ib.opacity = 255;
            ib.blend_mode = IPGUI_BLEND_NORMAL;
            ipgui_aabb_t idot = {{cx - ir, cy - ir}, {cx + ir - 1, cy + ir - 1}};
            ipgui_draw_box_background(ctx->surf, 0, &idot, &is, &ib);
        }
    } else {
        /* solid dot */
        ipgui_box_style_t s = {0};
        s.left_top_radius = s.right_top_radius =
            s.left_bottom_radius = s.right_bottom_radius = r;
        ipgui_box_bg_style_t bg;
        bg.paint.type = IPGUI_PAINT_COLOR;
        bg.paint.src.color = bd->style.color;
        bg.opacity = 255;
        bg.blend_mode = IPGUI_BLEND_NORMAL;
        ipgui_aabb_t dot = {{cx - r, cy - r}, {cx + r - 1, cy + r - 1}};
        ipgui_draw_box_background(ctx->surf, 0, &dot, &s, &bg);
    }
}

__IPGUI_API__ void ipgui_badge_style_init(ipgui_badge_style_t * s)
{
    if (!s) return;
    s->type = IPGUI_BADGE_TYPE_DOT;
    IPGUI_COLOR_SET(s->color, 255, 0xFF3B30);
    IPGUI_COLOR_SET(s->ring_color, 255, 0xFFFFFF);
    s->ring_width = 2;
}

__IPGUI_API__ ipgui_badge_t * ipgui_badge_create(ipgui_widget_t * parent)
{
    ipgui_badge_t * bd = (ipgui_badge_t *)ipgui_widget_create(parent);
    if (!bd) return (ipgui_badge_t *)0;
    ipgui_badge_style_init(&bd->style);
    ipgui_widget_set_render(&bd->base, badge_render);
    return bd;
}
