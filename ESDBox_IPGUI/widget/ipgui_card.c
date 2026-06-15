#include "ipgui_card.h"
#include "ipgui_prim.h"
#include "ipgui_color.h"
#include "ipgui_widget_tree.h"

static void card_render(ipgui_widget_t * w, ipgui_widget_render_ctx_t * ctx)
{
    ipgui_card_t * cd = (ipgui_card_t *)w;
    ipgui_aabb_t box;
    box.start.x = 0; box.start.y = 0;
    box.end.x = w->w - 1; box.end.y = w->h - 1;

    if (cd->style.has_shadow)
        ipgui_draw_box_shadow(ctx->surf, 0, &box, &cd->style.shape, &cd->style.shadow);
    ipgui_draw_box_background(ctx->surf, 0, &box, &cd->style.shape, &cd->style.bg);
    ipgui_draw_box_border(ctx->surf, 0, &box, &cd->style.shape, &cd->style.border);

    /* image area placeholder at top */
    if (cd->style.image_height > 0 && cd->style.image_height < w->h) {
        ipgui_box_style_t is = {0};
        is.left_top_radius = cd->style.shape.left_top_radius;
        is.right_top_radius = cd->style.shape.right_top_radius;
        int ih = cd->style.image_height;
        ipgui_aabb_t ia = {{0, 0}, {w->w - 1, ih - 1}};
        ipgui_draw_box_background(ctx->surf, 0, &ia, &is, &cd->style.img_area_bg);

        /* divider line below image area */
        ipgui_aabb_t div = {{0, ih - 1}, {w->w - 1, ih}};
        ipgui_box_bg_style_t dbg;
        dbg.paint.type = IPGUI_PAINT_COLOR;
        dbg.paint.src.color = cd->style.border.paint.src.color;
        dbg.paint.src.color.a = 80;
        dbg.opacity = 255;
        dbg.blend_mode = IPGUI_BLEND_NORMAL;
        ipgui_box_style_t ds = {0};
        ipgui_draw_box_background(ctx->surf, 0, &div, &ds, &dbg);
    }
}

__IPGUI_API__ void ipgui_card_style_init(ipgui_card_style_t * s)
{
    if (!s) return;
    s->shape = (ipgui_box_style_t){0};
    s->shape.left_top_radius = s->shape.right_top_radius =
        s->shape.left_bottom_radius = s->shape.right_bottom_radius = 10;

    s->bg.paint.type = IPGUI_PAINT_COLOR;
    IPGUI_COLOR_SET(s->bg.paint.src.color, 255, 0xFFFFFF);
    s->bg.opacity = 255;
    s->bg.blend_mode = IPGUI_BLEND_NORMAL;

    s->border.paint.type = IPGUI_PAINT_COLOR;
    IPGUI_COLOR_SET(s->border.paint.src.color, 255, 0xE0E0E0);
    s->border.opacity = 255;
    s->border.width = 1;
    s->border.blend_mode = IPGUI_BLEND_NORMAL;

    s->shadow = (ipgui_box_shadow_style_t){0};
    IPGUI_COLOR_SET(s->shadow.color, 255, 0x000000);
    s->shadow.opacity = 40;
    s->shadow.blur = 8;
    s->shadow.spread = 0;
    s->shadow.offset_x = 0;
    s->shadow.offset_y = 4;
    s->has_shadow = 1;

    s->image_height = 80;
    s->img_area_bg.paint.type = IPGUI_PAINT_COLOR;
    IPGUI_COLOR_SET(s->img_area_bg.paint.src.color, 255, 0xF5F5F5);
    s->img_area_bg.opacity = 255;
    s->img_area_bg.blend_mode = IPGUI_BLEND_NORMAL;
}

__IPGUI_API__ ipgui_card_t * ipgui_card_create(ipgui_widget_t * parent)
{
    ipgui_card_t * cd = (ipgui_card_t *)ipgui_widget_create(parent);
    if (!cd) return (ipgui_card_t *)0;
    ipgui_card_style_init(&cd->style);
    ipgui_widget_set_render(&cd->base, card_render);
    return cd;
}
