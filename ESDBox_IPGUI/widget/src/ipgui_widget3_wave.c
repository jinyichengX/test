#include "ipgui_draw_image_api.h"
#include "ipgui_widget.h"
#include "ipgui_blend.h"
#include "ipgui_pattle.h"
#include "ipgui_image_geometry_transform.h"
#include "ipgui_draw_arc.h"
#include "ipgui_time.h"
#include "open_sans.h"
extern ipgui_image_data_t wave_img;

void widget3_render(struct ipgui_widget * widget, ipgui_widget_render_ctx_t * ctx)
{
    ipgui_image_draw_style_t style;
    style.blend_mode = IPGUI_BLEND_NORMAL;
    style.opacity = 200;

    int img_w = (int)wave_img.w;
    int img_h = (int)wave_img.h;
    if (img_w <= 0 || img_h <= 0) return;

    /* 保持与原来相同的高度，宽度按比例缩放避免形变 */
    int tile_h = 150;
    int tile_w = tile_h * img_w / img_h;

    /* 从左向右无限滚动：ipgui_sys_tick 每 tick 前进 1 像素 */
    int offset = (int)ipgui_sys_tick % tile_w;
    int x_start = -offset;

    /* 只画 surf 可见范围（screen_render 已裁剪为脏区交集≈800px），
     * 不再遍历 1600px，大幅降低每帧 tile 绘制量 */
    int surf_w = ctx->surf->surf.end.x - ctx->surf->surf.start.x;

    ipgui_aabb_t target;
    target.start.y = 0;
    target.end.y = tile_h;  /* 控件本地 y=0，widget3 整体已在屏幕 y=250 */

    int x;
    for (x = x_start; x < surf_w; x += tile_w) {
        target.start.x = x;
        target.end.x = x + tile_w;
        ipgui_draw_image_in_rect(
            ctx->surf,
            &wave_img,
            &target,
            IPGUI_IMG_ALIGN_CENTER,
            IPGUI_IMG_FIT_STRETCH,
            &style
        );
    }
}

extern ipgui_image_data_t power_img;
void power_render(struct ipgui_widget * widget, ipgui_widget_render_ctx_t * ctx)
{
    ipgui_image_draw_style_t style;
    style.blend_mode = IPGUI_BLEND_NORMAL;
    style.opacity = 200;

    ipgui_draw_image_at(
    ctx->surf,
    &power_img,
    45,
    35,
    &style);

    ipgui_arc_t arc;
    arc.cx = 60;
    arc.cy = 49;
    arc.er = 25;
    arc.ir = arc.er - 2;
    arc.start = 0;
    arc.angle = 360;
    arc.dir = IPGUI_ARC_DRAW_DIR_CW;

    ipgui_arc_style_t arc_style;
    arc_style.paint.type = IPGUI_PAINT_COLOR;
    IPGUI_COLOR_SET(arc_style.paint.src.color, 255, 0x761ac7);
    arc_style.opacity    = style.opacity;
    arc_style.blend_mode = style.blend_mode;

    ipgui_draw_arc(
        ctx->surf,
        (ipgui_aabb_t *) 0,
        &arc,
        &arc_style);

    ipgui_font_style_t font_style;
    font_style.blend_mode = IPGUI_BLEND_NORMAL;
    font_style.opacity = 255;
    font_style.line_spacing = 0;
    font_style.font = &open_sans_20px;
    font_style.paint.type = IPGUI_PAINT_COLOR;
    IPGUI_COLOR_SET(font_style.paint.src.color, 255, 0x761ac7);

    ipgui_draw_builtin_text(
        ctx->surf,
        (ipgui_aabb_t *)0,
        &font_style,
        "Total 18KWH",
        0,
        80);
}
