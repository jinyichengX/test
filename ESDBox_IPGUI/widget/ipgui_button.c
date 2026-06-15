#include "ipgui_button.h"
#include "ipgui_prim.h"
#include "ipgui_color.h"
#include "ipgui_widget_tree.h"
#include <stddef.h>

/* 按钮渲染回调 */
static void btn_render(ipgui_widget_t * widget, ipgui_widget_render_ctx_t * ctx)
{
    ipgui_button_t * btn = (ipgui_button_t *)widget;

    /* 控件盒子在本地坐标系中，从 (0,0) 开始 */
    ipgui_aabb_t box;
    ipgui_button_get_box(btn, &box);

    /* 1. 绘制阴影（在最底层） */
    if (btn->style.shadow.enabled) {
        ipgui_draw_box_shadow(
            ctx->surf,
            (ipgui_aabb_t *)0,  /* clip 由框架保证，传 NULL 即可 */
            &box,
            &btn->style.shape,
            &btn->style.shadow.style);
    }

    /* 2. 绘制背景 */
    ipgui_draw_box_background(
        ctx->surf,
        (ipgui_aabb_t *)0,
        &box,
        &btn->style.shape,
        &btn->style.bg);

    /* 3. 绘制边框 */
    ipgui_draw_box_border(
        ctx->surf,
        (ipgui_aabb_t *)0,
        &box,
        &btn->style.shape,
        &btn->style.border);
}

__IPGUI_API__ void ipgui_button_get_box(ipgui_button_t * btn, ipgui_aabb_t * box)
{
    if (!btn || !box) return;
    box->start.x = 0;
    box->start.y = 0;
    box->end.x   = btn->base.w - 1;
    box->end.y   = btn->base.h - 1;
}

__IPGUI_API__ void ipgui_button_style_init(ipgui_button_style_t * style)
{
    if (!style) return;

    /* 盒子形状：默认无圆角，无 padding */
    style->shape.left_padding    = 0;
    style->shape.right_padding   = 0;
    style->shape.top_padding     = 0;
    style->shape.bottom_padding  = 0;
    style->shape.left_top_radius     = 0;
    style->shape.right_top_radius    = 0;
    style->shape.left_bottom_radius  = 0;
    style->shape.right_bottom_radius = 0;

    /* 背景：默认浅灰色 */
    style->bg.paint.type = IPGUI_PAINT_COLOR;
    IPGUI_COLOR_SET(style->bg.paint.src.color, 255, 0xC0C0C0);
    style->bg.opacity    = 255;
    style->bg.blend_mode = IPGUI_BLEND_NORMAL;

    /* 边框：默认无边框 */
    style->border.paint.type = IPGUI_PAINT_COLOR;
    IPGUI_COLOR_SET(style->border.paint.src.color, 255, 0x000000);
    style->border.opacity    = 0;
    style->border.width      = 1;
    style->border.blend_mode = IPGUI_BLEND_NORMAL;

    /* 阴影：默认关闭 */
    style->shadow.enabled = 0;
    IPGUI_COLOR_SET(style->shadow.style.color, 255, 0x000000);
    style->shadow.style.opacity   = 128;
    style->shadow.style.blur      = 4;
    style->shadow.style.spread    = 0;
    style->shadow.style.offset_x  = 2;
    style->shadow.style.offset_y  = 2;
}

__IPGUI_API__ ipgui_button_t * ipgui_button_create(ipgui_widget_t * parent)
{
    ipgui_button_t * btn = (ipgui_button_t *)ipgui_widget_create(parent);
    if (!btn) return (ipgui_button_t *)0;

    ipgui_button_style_init(&btn->style);
    btn->text = (const char *)0;

    /* 注册渲染回调 */
    ipgui_widget_set_render(&btn->base, btn_render);

    return btn;
}
