#include "ipgui_rating.h"
#include "ipgui_prim.h"
#include "ipgui_color.h"
#include "ipgui_widget_tree.h"

static void rt_render(ipgui_widget_t * w, ipgui_widget_render_ctx_t * ctx)
{
    ipgui_rating_t * rt = (ipgui_rating_t *)w;
    int sz = rt->style.size > 0 ? rt->style.size : IPGUI_MIN(w->h, w->w / rt->style.max);
    if (sz < 4) sz = 4;
    int gap = sz / 4;
    if (gap < 1) gap = 1;

    int i;
    for (i = 0; i < rt->style.max; i++) {
        ipgui_color_t col = (i < rt->style.value) ? rt->style.active_color : rt->style.inactive_color;
        int ox = i * (sz + gap);
        int oy = (w->h - sz) / 2;

        if (rt->style.display == IPGUI_RATING_STYLE_DOT) {
            ipgui_aabb_t d = {{ox, oy}, {ox + sz - 1, oy + sz - 1}};
            ipgui_box_style_t s = {0};
            int r = sz / 2;
            s.left_top_radius = s.right_top_radius =
                s.left_bottom_radius = s.right_bottom_radius = r;
            ipgui_box_bg_style_t b;
            b.paint.type = IPGUI_PAINT_COLOR;
            b.paint.src.color = col;
            b.opacity = 255;
            b.blend_mode = IPGUI_BLEND_NORMAL;
            ipgui_draw_box_background(ctx->surf, 0, &d, &s, &b);
        } else {
            /* star = composed of diagonal cross bars + center dot */
            ipgui_box_bg_style_t b;
            b.paint.type = IPGUI_PAINT_COLOR;
            b.paint.src.color = col;
            b.opacity = 255;
            b.blend_mode = IPGUI_BLEND_NORMAL;
            ipgui_box_style_t s = {0};
            s.left_top_radius = s.right_top_radius =
                s.left_bottom_radius = s.right_bottom_radius = 1;

            /* 5-point star via 4 diagonal arms + center */
            int cx = ox + sz / 2, cy = oy + sz / 2;
            int arm = sz / 2;
            int arm_w = IPGUI_MAX(2, sz / 8);

            /* vertical */
            {ipgui_aabb_t r = {{cx - arm_w/2, cy - arm}, {cx + arm_w/2 - 1, cy + arm - 1}};
             ipgui_draw_box_background(ctx->surf, 0, &r, &s, &b);}
            /* horizontal */
            {ipgui_aabb_t r = {{cx - arm, cy - arm_w/2}, {cx + arm - 1, cy + arm_w/2 - 1}};
             ipgui_draw_box_background(ctx->surf, 0, &r, &s, &b);}
            /* diagonal \ */
            {ipgui_aabb_t r = {{cx - arm/2, cy - arm/2}, {cx + arm/2 - 1, cy + arm/2 - 1}};
             int rad = arm_w/2; s.left_top_radius = s.right_top_radius = s.left_bottom_radius = s.right_bottom_radius = rad;
             ipgui_draw_box_background(ctx->surf, 0, &r, &s, &b);}
            /* diagonal / */
            {ipgui_aabb_t r = {{cx - arm/2 - arm_w/2, cy + arm*2/5}, {cx + arm/2 + arm_w/2 - 1, cy - arm*2/5}};
             s.left_top_radius = s.right_top_radius = s.left_bottom_radius = s.right_bottom_radius = arm_w/2;
             ipgui_draw_box_background(ctx->surf, 0, &r, &s, &b);}
        }
    }
}

__IPGUI_API__ void ipgui_rating_style_init(ipgui_rating_style_t * s)
{
    if (!s) return;
    s->display = IPGUI_RATING_STYLE_STAR;
    IPGUI_COLOR_SET(s->active_color, 255, 0xFF9500);
    IPGUI_COLOR_SET(s->inactive_color, 255, 0xDDDDDD);
    s->value = 3;
    s->max = 5;
    s->size = 20;
}

__IPGUI_API__ ipgui_rating_t * ipgui_rating_create(ipgui_widget_t * parent)
{
    ipgui_rating_t * rt = (ipgui_rating_t *)ipgui_widget_create(parent);
    if (!rt) return (ipgui_rating_t *)0;
    ipgui_rating_style_init(&rt->style);
    ipgui_widget_set_render(&rt->base, rt_render);
    return rt;
}
