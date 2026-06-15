#include "ipgui_scrollbar.h"
#include "ipgui_prim.h"
#include "ipgui_color.h"
#include "ipgui_widget_tree.h"

static void sb_render(ipgui_widget_t * w, ipgui_widget_render_ctx_t * ctx)
{
    ipgui_scrollbar_t * sb = (ipgui_scrollbar_t *)w;
    int t = IPGUI_MAX(4, sb->style.thickness);

    if (sb->style.dir == IPGUI_SCROLLBAR_VERTICAL) {
        /* track */
        int tx = (w->w - t) / 2;
        ipgui_aabb_t trk = {{tx, 0}, {tx + t - 1, w->h - 1}};
        ipgui_box_style_t ts = {0};
        ts.left_top_radius = ts.right_top_radius =
            ts.left_bottom_radius = ts.right_bottom_radius = t / 2;
        ipgui_box_bg_style_t tb;
        tb.paint.type = IPGUI_PAINT_COLOR;
        tb.paint.src.color = sb->style.track_color;
        tb.opacity = 255;
        tb.blend_mode = IPGUI_BLEND_NORMAL;
        ipgui_draw_box_background(ctx->surf, 0, &trk, &ts, &tb);

        /* thumb */
        int th = sb->style.thumb_len > 0 ? sb->style.thumb_len : w->h * 30 / 100;
        if (th < t * 2) th = t * 2;
        float ratio = sb->style.max > 0 ? (float)sb->style.value / sb->style.max : 0;
        int ty = (int)(ratio * (w->h - th));
        if (ty < 0) ty = 0;
        ipgui_aabb_t thumb = {{tx + 1, ty}, {tx + t - 2, ty + th - 1}};
        ipgui_box_style_t ms = {0};
        ms.left_top_radius = ms.right_top_radius =
            ms.left_bottom_radius = ms.right_bottom_radius = (t - 2) / 2;
        ipgui_box_bg_style_t mb;
        mb.paint.type = IPGUI_PAINT_COLOR;
        mb.paint.src.color = sb->style.thumb_color;
        mb.opacity = 255;
        mb.blend_mode = IPGUI_BLEND_NORMAL;
        ipgui_draw_box_background(ctx->surf, 0, &thumb, &ms, &mb);
    } else {
        /* track */
        int ty = (w->h - t) / 2;
        ipgui_aabb_t trk = {{0, ty}, {w->w - 1, ty + t - 1}};
        ipgui_box_style_t ts = {0};
        ts.left_top_radius = ts.right_top_radius =
            ts.left_bottom_radius = ts.right_bottom_radius = t / 2;
        ipgui_box_bg_style_t tb;
        tb.paint.type = IPGUI_PAINT_COLOR;
        tb.paint.src.color = sb->style.track_color;
        tb.opacity = 255;
        tb.blend_mode = IPGUI_BLEND_NORMAL;
        ipgui_draw_box_background(ctx->surf, 0, &trk, &ts, &tb);

        /* thumb */
        int tw = sb->style.thumb_len > 0 ? sb->style.thumb_len : w->w * 30 / 100;
        if (tw < t * 2) tw = t * 2;
        float ratio = sb->style.max > 0 ? (float)sb->style.value / sb->style.max : 0;
        int tx = (int)(ratio * (w->w - tw));
        if (tx < 0) tx = 0;
        ipgui_aabb_t thumb = {{tx, ty + 1}, {tx + tw - 1, ty + t - 2}};
        ipgui_box_style_t ms = {0};
        ms.left_top_radius = ms.right_top_radius =
            ms.left_bottom_radius = ms.right_bottom_radius = (t - 2) / 2;
        ipgui_box_bg_style_t mb;
        mb.paint.type = IPGUI_PAINT_COLOR;
        mb.paint.src.color = sb->style.thumb_color;
        mb.opacity = 255;
        mb.blend_mode = IPGUI_BLEND_NORMAL;
        ipgui_draw_box_background(ctx->surf, 0, &thumb, &ms, &mb);
    }
}

__IPGUI_API__ void ipgui_scrollbar_style_init(ipgui_scrollbar_style_t * s)
{
    if (!s) return;
    s->dir = IPGUI_SCROLLBAR_VERTICAL;
    IPGUI_COLOR_SET(s->track_color, 255, 0xEEEEEE);
    IPGUI_COLOR_SET(s->thumb_color, 255, 0xBBBBBB);
    s->thickness = 10;
    s->value = 30;
    s->max = 100;
    s->thumb_len = 0;
}

__IPGUI_API__ ipgui_scrollbar_t * ipgui_scrollbar_create(ipgui_widget_t * parent)
{
    ipgui_scrollbar_t * sb = (ipgui_scrollbar_t *)ipgui_widget_create(parent);
    if (!sb) return (ipgui_scrollbar_t *)0;
    ipgui_scrollbar_style_init(&sb->style);
    ipgui_widget_set_render(&sb->base, sb_render);
    return sb;
}
