#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include "SDL.h"
#include "sdl_draw.h"
#include "sdl_input_event.h"

#include "ipgui_screen.h"
#include "ipgui_widget.h"
#include "ipgui_input_dispatcher.h"
#include "ipgui_blend.h"
#include "ipgui_pattle.h"
#include "ipgui_draw_arc.h"
#include "ipgui_image.h"
#include "ipgui_time.h"
#include "ipgui_animation.h"
#include "ipgui_draw_icon.h"
#include "ipgui_draw_box_background.h"
#include "ipgui_draw_box_shadow.h"
#include "ipgui_blur.h"
#include "ipgui_memory.h"
#include <math.h>
#undef main

extern __IPGUI_API__ ipgui_err_t ipgui_sdl_mouse_event_poll(void * priv_data, ipgui_input_src_evt_t * raw_evt);

ipgui_input_dispatcher_t dispatcher;
ipgui_input_src_t pointer_src;
ipgui_input_src_t keyboard_src;
ipgui_scr_t main_screen;
ipgui_input_src_id_t pointer_id;
ipgui_input_src_id_t keyboard_id;
ipgui_scr_id_t main_scr_id;

ipgui_scr_drv_t sdl_drv = {
    .xreso = 800,
    .yreso = 480,
    
    .pri_data    = &g_sdl_private,
    .put_pixel   = sdl_put_pixel,
    .fill_region = sdl_fill_region,
    // .close       = sdl_exit,
    .flush       = sdl_flush,
};

static u8_t main_screen_frame_buf[800 * 4 * 480];

void draw_main_screen_backgroud_color(ipgui_scr_t * scr, ipgui_surf_t * surf)
{
    // ipgui_color_t backgroud_color;
    // IPGUI_COLOR_SET(backgroud_color, 255, 0xfffffff);
    // ipgui_blend_color(surf, (ipgui_aabb_t *)0, &surf->surf, backgroud_color, 100 , NULL, NULL, IPGUI_BLEND_NORMAL);

    ipgui_paint_t paint;
    paint.type = IPGUI_PAINT_GRADIENT;
    paint.src.grad_src.grad_type = IPGUI_GRADIENT_TYPE_LINEAR;

    ipgui_liner_gradient_init_direct(&paint.src.grad_src.grad.liner_grad, 
        0, 0, 799, 0);
    ipgui_gradient_color_stop_t stop1;
    stop1.pos = 0;
    IPGUI_COLOR_SET(stop1.color, 255, 0x2e317c);
    ipgui_gradient_color_stop_t stop2;
    stop2.pos = 255;
    IPGUI_COLOR_SET(stop2.color, 255, 0x806d9e);
    ipgui_liner_gradient_add_stop(&paint.src.grad_src.grad.liner_grad, &stop1);
    ipgui_liner_gradient_add_stop(&paint.src.grad_src.grad.liner_grad, &stop2);

    ipgui_blend(surf, (ipgui_aabb_t *)0, &surf->surf, &paint, 255 , NULL, NULL, IPGUI_BLEND_NORMAL);
}
extern void widget1_render(struct ipgui_widget * widget, ipgui_widget_render_ctx_t * ctx);
extern void widget2_render(struct ipgui_widget * widget, ipgui_widget_render_ctx_t * ctx);
extern void widget3_render(struct ipgui_widget * widget, ipgui_widget_render_ctx_t * ctx);
extern void widget4_render(struct ipgui_widget * widget, ipgui_widget_render_ctx_t * ctx);
extern void knob_circle_render(struct ipgui_widget * widget, ipgui_widget_render_ctx_t * ctx);
extern void knob_circle_event_handler(struct ipgui_widget * widget, ipgui_widget_evt_t * evt);
extern void widget_knob_update(ipgui_widget_t * w);
extern void brightness_label_render(struct ipgui_widget * widget, ipgui_widget_render_ctx_t * ctx);
extern void widget_wave_update(ipgui_widget_t * w);
extern void widget_switch_render(struct ipgui_widget * widget, ipgui_widget_render_ctx_t * ctx);
extern void widget_switch_label_render(struct ipgui_widget * widget, ipgui_widget_render_ctx_t * ctx);
extern void widget_arc_label_render(struct ipgui_widget * widget, ipgui_widget_render_ctx_t * ctx);
extern void power_render(struct ipgui_widget * widget, ipgui_widget_render_ctx_t * ctx);
extern void tablelamp_render(struct ipgui_widget * widget, ipgui_widget_render_ctx_t * ctx);
extern void tablelamp_event_handler(struct ipgui_widget * widget, ipgui_widget_evt_t * evt);
extern void widget_tablelamp_update(ipgui_widget_t * w);
extern void arc_bg_render(struct ipgui_widget * widget, ipgui_widget_render_ctx_t * ctx);
extern void _label_livingroom_render(struct ipgui_widget * widget, ipgui_widget_render_ctx_t * ctx);
extern void livingroom_event_handler(struct ipgui_widget * widget, ipgui_widget_evt_t * evt);
extern void widget_livingroom_update(ipgui_widget_t * w);
extern void widget_switch_event_handler(struct ipgui_widget * widget, ipgui_widget_evt_t * evt);
extern void widget_switch_update(ipgui_widget_t * w);
extern void widget_switch_label_update(ipgui_widget_t * w);
extern void curve_render(struct ipgui_widget * widget, ipgui_widget_render_ctx_t * ctx);
extern void widget_curve_update(ipgui_widget_t * w);
ipgui_image_data_t wave_img;
ipgui_image_data_t main_bg_img;
ipgui_image_data_t power_img;
ipgui_image_data_t tablelamp_img;
ipgui_image_data_t tablelamp_on_img;

#include "icon_play.h"
#include "open_sans.h"
#include "quicksand_medium.h"
#include "ipgui_draw_builtin_font.h"


/* ========== 测试：滚动 vs 拖拽 ========== */
typedef struct { u8_t r, g, b; } widget_color_t;

static widget_color_t col_blue   = {0x4a, 0x90, 0xd9};
static widget_color_t col_red    = {0xe0, 0x5d, 0x5d};
static widget_color_t col_green  = {0x5d, 0xb8, 0x5d};
static widget_color_t col_orange = {0xf0, 0x8a, 0x3a};
static widget_color_t col_purple = {0x9b, 0x59, 0xb6};
static widget_color_t col_gray   = {0x99, 0x99, 0x99};

/* 通用颜色块渲染 */
void color_render(ipgui_widget_t * w, ipgui_widget_render_ctx_t * ctx)
{
    widget_color_t * c = (widget_color_t *)w->priv_data;
    ipgui_aabb_t box = {{0,0},{w->w - 1, w->h - 1}};
    ipgui_box_style_t s; ipgui_memset(&s, 0, sizeof(s));
    s.left_top_radius = s.right_top_radius = s.left_bottom_radius = s.right_bottom_radius = 12;
    ipgui_box_bg_style_t bg;
    bg.paint.type = IPGUI_PAINT_COLOR;
    IPGUI_COLOR_SET(bg.paint.src.color, 255, ((u32_t)c->r << 16) | ((u32_t)c->g << 8) | c->b);
    bg.opacity = 100; bg.blend_mode = IPGUI_BLEND_NORMAL;
    ipgui_draw_box_background(ctx->surf, NULL, &box, &s, &bg);

    /* 居中绘制控件名 */
    if (w->name) {
        ipgui_font_style_t fs;
        ipgui_memset(&fs, 0, sizeof(fs));
        fs.font = &open_sans_18px;
        fs.paint.type = IPGUI_PAINT_COLOR;
        IPGUI_COLOR_SET(fs.paint.src.color, 255, 0xFFFFFF);
        fs.opacity = 220;
        fs.blend_mode = IPGUI_BLEND_NORMAL;

        ipgui_coord_t tw = ipgui_builtin_text_width(fs.font, (const s8_t *)w->name);
        ipgui_coord_t tx = ((w->w - 1) - tw) / 2;
        ipgui_coord_t ty = ((w->h - 1) - fs.font->line_height) / 2;
        ipgui_draw_builtin_text(ctx->surf, NULL, &fs, (const s8_t *)w->name, tx, ty);
    }
}

// /* 滚动事件处理：修改触发控件的 scroll_x/y */
// void scroll_handler(ipgui_widget_t * w, ipgui_widget_evt_t * e)
// {
//     if (e->type != IPGUI_WIDGET_EVENT_PRESSED) return;
//     ipgui_coord_t dx = e->evt.pressed_evt.x - e->evt.pressed_evt.last_press_x;
//     ipgui_coord_t dy = e->evt.pressed_evt.y - e->evt.pressed_evt.last_press_y;
//     if (dx == 0 && dy == 0) return;
//     w->scroll_x -= dx;
//     w->scroll_y -= dy;
//     ipgui_widget_mark_dirty(w);
// }

// /* 滚动 + 拖拽处理：拖拽时同时驱动内容滚动和控件位移，wid1 专用 */
// void scroll_drag_handler(ipgui_widget_t * w, ipgui_widget_evt_t * e)
// {
//     if (e->type != IPGUI_WIDGET_EVENT_PRESSED) return;
//     ipgui_coord_t dx = e->evt.pressed_evt.x - e->evt.pressed_evt.last_press_x;
//     ipgui_coord_t dy = e->evt.pressed_evt.y - e->evt.pressed_evt.last_press_y;
//     if (dx == 0 && dy == 0) return;

//     /* 滚动: 驱动子控件内容 */
//     w->scroll_x -= dx;
//     w->scroll_y -= dy;

//     /* 拖拽: 同时移动控件本身 */
//     ipgui_widget_mark_dirty(w);
//     w->x += dx;
//     w->y += dy;
//     ipgui_widget_mark_dirty(w);
// }

/* 拖拽事件处理：直接修改当前控件的 x/y */
void drag_handler(ipgui_widget_t * w, ipgui_widget_evt_t * e)
{
    if (e->type != IPGUI_WIDGET_EVENT_PRESSED) return;
    ipgui_coord_t dx = e->evt.pressed_evt.x - e->evt.pressed_evt.last_press_x;
    ipgui_coord_t dy = e->evt.pressed_evt.y - e->evt.pressed_evt.last_press_y;
    if (dx == 0 && dy == 0) return;
    /* 先标脏旧位置，再移动，再标脏新位置，避免旧位置残留 */
    ipgui_widget_mark_dirty(w);
    w->x += dx;
    w->y += dy;
    ipgui_widget_mark_dirty(w);
}
extern void ipgui_average_blur_hor(
    ipgui_mask_surface_t * in,
    ipgui_coord_t          kn_size, /* average blur kernel size */
    ipgui_mask_surface_t * out);
void blur_render(struct ipgui_widget * widget, ipgui_widget_render_ctx_t * ctx)
{
    static const unsigned char open_sans_25px_char_065_bitmap[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x23, 0xfe, 0xf4, 0x0d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 行 0 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x87, 0xff, 0xff, 0x65, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 行 1 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0xe6, 0xd3, 0xf7, 0xc9, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 行 2 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x50, 0xff, 0x7c, 0xb3, 0xff, 0x2d, 0x00, 0x00, 0x00, 0x00, 0x00, /* 行 3 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0xb4, 0xff, 0x2c, 0x57, 0xff, 0x90, 0x00, 0x00, 0x00, 0x00, 0x00, /* 行 4 */
    0x00, 0x00, 0x00, 0x00, 0x1b, 0xfc, 0xcc, 0x00, 0x07, 0xed, 0xec, 0x07, 0x00, 0x00, 0x00, 0x00, /* 行 5 */
    0x00, 0x00, 0x00, 0x00, 0x7d, 0xff, 0x67, 0x00, 0x00, 0x92, 0xff, 0x58, 0x00, 0x00, 0x00, 0x00, /* 行 6 */
    0x00, 0x00, 0x00, 0x01, 0xdf, 0xf5, 0x0e, 0x00, 0x00, 0x2d, 0xff, 0xbc, 0x00, 0x00, 0x00, 0x00, /* 行 7 */
    0x00, 0x00, 0x00, 0x46, 0xff, 0x9e, 0x00, 0x00, 0x00, 0x00, 0xca, 0xfe, 0x21, 0x00, 0x00, 0x00, /* 行 8 */
    0x00, 0x00, 0x00, 0xab, 0xff, 0x3a, 0x00, 0x00, 0x00, 0x00, 0x66, 0xff, 0x83, 0x00, 0x00, 0x00, /* 行 9 */
    0x00, 0x00, 0x14, 0xf9, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe4, 0x03, 0x00, 0x00, /* 行10 */
    0x00, 0x00, 0x73, 0xff, 0xe8, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xf0, 0xff, 0x4b, 0x00, 0x00, /* 行11 */
    0x00, 0x00, 0xd7, 0xfb, 0x16, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x52, 0xff, 0xaf, 0x00, 0x00, /* 行12 */
    0x00, 0x3c, 0xff, 0xb6, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0xee, 0xfb, 0x17, 0x00, /* 行13 */
    0x00, 0xa1, 0xff, 0x5a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x99, 0xff, 0x76, 0x00, /* 行14 */
    0x0f, 0xf5, 0xf3, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3c, 0xff, 0xd9, 0x00, /* 行15 */
    0x6a, 0xff, 0xa2, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xde, 0xff, 0x3e, /* 行16 */
    0xce, 0xff, 0x46, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x82, 0xff, 0xa2, /* 行17 */
    };
    static const unsigned char open_sans_25px_char_066_bitmap[] = {
    0x8c, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xf4, 0xe0, 0xb6, 0x6d, 0x0b, 0x00, 0x00, /* 行 0 */
    0x8c, 0xff, 0xe7, 0xcc, 0xcc, 0xcd, 0xd7, 0xf5, 0xff, 0xff, 0xe2, 0x25, 0x00, /* 行 1 */
    0x8c, 0xff, 0x88, 0x00, 0x00, 0x00, 0x00, 0x03, 0x45, 0xdb, 0xff, 0xc2, 0x00, /* 行 2 */
    0x8c, 0xff, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x33, 0xff, 0xff, 0x17, /* 行 3 */
    0x8c, 0xff, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf4, 0xff, 0x30, /* 行 4 */
    0x8c, 0xff, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0b, 0xfe, 0xff, 0x16, /* 行 5 */
    0x8c, 0xff, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x59, 0xff, 0xc9, 0x00, /* 行 6 */
    0x8c, 0xff, 0x88, 0x00, 0x00, 0x00, 0x05, 0x20, 0x6a, 0xf0, 0xe8, 0x30, 0x00, /* 行 7 */
    0x8c, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf3, 0x7e, 0x18, 0x00, 0x00, /* 行 8 */
    0x8c, 0xff, 0xe3, 0xc4, 0xc4, 0xc4, 0xc9, 0xdf, 0xfe, 0xf2, 0x9b, 0x18, 0x00, /* 行 9 */
    0x8c, 0xff, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1a, 0x98, 0xff, 0xe3, 0x16, /* 行10 */
    0x8c, 0xff, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xba, 0xff, 0x8a, /* 行11 */
    0x8c, 0xff, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x72, 0xff, 0xc3, /* 行12 */
    0x8c, 0xff, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0xff, 0xc4, /* 行13 */
    0x8c, 0xff, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc5, 0xff, 0x98, /* 行14 */
    0x8c, 0xff, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1a, 0x9f, 0xff, 0xfc, 0x30, /* 行15 */
    0x8c, 0xff, 0xe3, 0xc4, 0xc4, 0xc4, 0xc7, 0xdc, 0xfe, 0xff, 0xf8, 0x5f, 0x00, /* 行16 */
    0x8c, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf3, 0xd3, 0x92, 0x26, 0x00, 0x00, /* 行17 */
};
static const unsigned char open_sans_25px_char_067_bitmap[] = {
    0x00, 0x00, 0x00, 0x00, 0x03, 0x55, 0xaa, 0xde, 0xf7, 0xfb, 0xeb, 0xc6, 0x89, 0x2f, 0x00, /* 行 0 */
    0x00, 0x00, 0x00, 0x25, 0xd1, 0xff, 0xff, 0xfd, 0xea, 0xe8, 0xfe, 0xff, 0xff, 0xcc, 0x00, /* 行 1 */
    0x00, 0x00, 0x38, 0xf0, 0xff, 0xda, 0x5b, 0x0f, 0x00, 0x00, 0x07, 0x3b, 0x95, 0x48, 0x00, /* 行 2 */
    0x00, 0x0c, 0xdd, 0xff, 0xba, 0x0b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 行 3 */
    0x00, 0x89, 0xff, 0xe1, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 行 4 */
    0x03, 0xec, 0xff, 0x69, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 行 5 */
    0x35, 0xff, 0xfe, 0x13, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 行 6 */
    0x5f, 0xff, 0xdd, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 行 7 */
    0x72, 0xff, 0xc8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 行 8 */
    0x70, 0xff, 0xc7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 行 9 */
    0x5f, 0xff, 0xda, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 行10 */
    0x3b, 0xff, 0xfd, 0x0d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 行11 */
    0x08, 0xf8, 0xff, 0x59, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 行12 */
    0x00, 0xa4, 0xff, 0xd1, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 行13 */
    0x00, 0x2c, 0xf9, 0xff, 0x9e, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 行14 */
    0x00, 0x00, 0x6b, 0xff, 0xff, 0xc7, 0x4a, 0x08, 0x00, 0x00, 0x00, 0x1b, 0x54, 0x28, 0x00, /* 行15 */
    0x00, 0x00, 0x00, 0x60, 0xf1, 0xff, 0xff, 0xf9, 0xe3, 0xe1, 0xf6, 0xff, 0xff, 0x54, 0x00, /* 行16 */
    0x00, 0x00, 0x00, 0x00, 0x13, 0x78, 0xc0, 0xe8, 0xfa, 0xf3, 0xe1, 0xc1, 0x82, 0x1c, 0x00, /* 行17 */
};
static const unsigned char open_sans_25px_char_038_bitmap[] = {
    0x00, 0x00, 0x00, 0x27, 0xa3, 0xdb, 0xf6, 0xde, 0xa3, 0x21, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 行 0 */
    0x00, 0x00, 0x3b, 0xf8, 0xff, 0xdb, 0xb5, 0xe0, 0xff, 0xf2, 0x29, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 行 1 */
    0x00, 0x00, 0xbe, 0xff, 0x8b, 0x01, 0x00, 0x04, 0xa9, 0xff, 0xa1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 行 2 */
    0x00, 0x00, 0xec, 0xff, 0x1a, 0x00, 0x00, 0x00, 0x42, 0xff, 0xcc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 行 3 */
    0x00, 0x00, 0xd5, 0xff, 0x2d, 0x00, 0x00, 0x00, 0x5d, 0xff, 0xb2, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 行 4 */
    0x00, 0x00, 0x86, 0xff, 0xa5, 0x00, 0x00, 0x16, 0xdb, 0xff, 0x51, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 行 5 */
    0x00, 0x00, 0x12, 0xe4, 0xff, 0x73, 0x43, 0xdf, 0xff, 0x9a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 行 6 */
    0x00, 0x00, 0x00, 0x33, 0xf3, 0xff, 0xff, 0xf6, 0x72, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 行 7 */
    0x00, 0x00, 0x07, 0x89, 0xfa, 0xff, 0xff, 0x96, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 行 8 */
    0x00, 0x18, 0xce, 0xff, 0xd6, 0x6c, 0xf5, 0xff, 0x88, 0x00, 0x00, 0x00, 0x00, 0x26, 0xff, 0xe7, 0x00, 0x00, /* 行 9 */
    0x01, 0xc8, 0xff, 0xac, 0x09, 0x00, 0x3f, 0xf4, 0xff, 0x8f, 0x00, 0x00, 0x00, 0x74, 0xff, 0x9e, 0x00, 0x00, /* 行10 */
    0x4b, 0xff, 0xde, 0x06, 0x00, 0x00, 0x00, 0x3d, 0xf3, 0xff, 0x96, 0x01, 0x00, 0xcd, 0xff, 0x40, 0x00, 0x00, /* 行11 */
    0x87, 0xff, 0x8e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3b, 0xf2, 0xff, 0x9c, 0x5e, 0xff, 0xcb, 0x00, 0x00, 0x00, /* 行12 */
    0x98, 0xff, 0x8e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x39, 0xf1, 0xff, 0xfe, 0xfb, 0x39, 0x00, 0x00, 0x00, /* 行13 */
    0x75, 0xff, 0xd2, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79, 0xff, 0xff, 0xc7, 0x03, 0x00, 0x00, 0x00, /* 行14 */
    0x1f, 0xf9, 0xff, 0xa6, 0x17, 0x00, 0x00, 0x03, 0x37, 0xa3, 0xfe, 0xfe, 0xf5, 0xff, 0xa7, 0x03, 0x00, 0x00, /* 行15 */
    0x00, 0x61, 0xfc, 0xff, 0xfd, 0xe0, 0xda, 0xf3, 0xff, 0xff, 0xdd, 0x40, 0x34, 0xee, 0xff, 0xaa, 0x04, 0x00, /* 行16 */
    0x00, 0x00, 0x32, 0xa2, 0xe1, 0xf9, 0xf5, 0xdf, 0xa9, 0x59, 0x04, 0x00, 0x00, 0x30, 0xeb, 0xff, 0xac, 0x05, /* 行17 */
};
    ipgui_mask_surface_t char_mask;
    char_mask.w = 16;
    char_mask.h = 18;
    char_mask.mask = (u8_t *)open_sans_25px_char_065_bitmap;

    u8_t mask_blur_out[30 * 30];
    u8_t mask_blur_out1[40 * 40];
    u8_t mask_blur_out2[50 * 50];

    ipgui_mask_surface_t char_a_mask_blur_out;
    char_a_mask_blur_out.w = 30;
    char_a_mask_blur_out.h = 30;
    char_a_mask_blur_out.mask = mask_blur_out;

    ipgui_mask_surface_t char_a_mask_blur_out1;
    char_a_mask_blur_out1.w = 40;
    char_a_mask_blur_out1.h = 40;
    char_a_mask_blur_out1.mask = mask_blur_out1;

    ipgui_mask_surface_t char_a_mask_blur_out2;
    char_a_mask_blur_out2.w = 50;
    char_a_mask_blur_out2.h = 50;
    char_a_mask_blur_out2.mask = mask_blur_out2;

    ipgui_average_blur_hor(&char_mask, 4, &char_a_mask_blur_out);
    ipgui_average_blur_hor(&char_mask, 4, &char_a_mask_blur_out1);
    ipgui_average_blur_hor(&char_mask, 4, &char_a_mask_blur_out2);

    ipgui_average_blur_ver(&char_a_mask_blur_out2, 4, &char_a_mask_blur_out2);
    ipgui_average_blur_ver(&char_a_mask_blur_out2, 4, &char_a_mask_blur_out2);
    ipgui_average_blur_ver(&char_a_mask_blur_out2, 4, &char_a_mask_blur_out2);

    ipgui_color_t color11;
    IPGUI_COLOR_SET(color11, 255, 0xff0000);
    ipgui_aabb_t aabb;
    aabb.start.x = 0;
    aabb.start.y = 0;
    aabb.end.x = 49;
    aabb.end.y = 49;
    ipgui_blend_color(
        ctx->surf,
        NULL,
        &aabb,
        color11,
        255,
        mask_blur_out2,      /* mask覆盖mask_aabb区域 */
        &aabb,  /* mask对应的坐标区域，必须大于或等于dest区域 */
    0);
    
    char_mask.w = 13;
    char_mask.h = 18;
    char_mask.mask = (u8_t *)open_sans_25px_char_066_bitmap;

    ipgui_average_blur_hor(&char_mask, 4, &char_a_mask_blur_out);
    ipgui_average_blur_hor(&char_mask, 4, &char_a_mask_blur_out1);
    ipgui_average_blur_hor(&char_mask, 4, &char_a_mask_blur_out2);

    ipgui_average_blur_ver(&char_a_mask_blur_out2, 4, &char_a_mask_blur_out2);
    ipgui_average_blur_ver(&char_a_mask_blur_out2, 4, &char_a_mask_blur_out2);
    ipgui_average_blur_ver(&char_a_mask_blur_out2, 4, &char_a_mask_blur_out2);

    aabb.start.x = 18;
    aabb.start.y = 0;
    aabb.end.x = 49 + 18;
    aabb.end.y = 49;
    ipgui_blend_color(
        ctx->surf,
        NULL,
        &aabb,
        color11,
        255,
        mask_blur_out2,      /* mask覆盖mask_aabb区域 */
        &aabb,  /* mask对应的坐标区域，必须大于或等于dest区域 */
    0);

    char_mask.w = 15;
    char_mask.h = 18;
    char_mask.mask = (u8_t *)open_sans_25px_char_067_bitmap;
    ipgui_average_blur_hor(&char_mask, 4, &char_a_mask_blur_out);
    ipgui_average_blur_hor(&char_mask, 4, &char_a_mask_blur_out1);
    ipgui_average_blur_hor(&char_mask, 4, &char_a_mask_blur_out2);

    ipgui_average_blur_ver(&char_a_mask_blur_out2, 4, &char_a_mask_blur_out2);
    ipgui_average_blur_ver(&char_a_mask_blur_out2, 4, &char_a_mask_blur_out2);
    ipgui_average_blur_ver(&char_a_mask_blur_out2, 4, &char_a_mask_blur_out2);

    aabb.start.x = 33;
    aabb.start.y = 0;
    aabb.end.x = 49 + 16 + 17;
    aabb.end.y = 49;
    ipgui_blend_color(
        ctx->surf,
        NULL,
        &aabb,
        color11,
        255,
        mask_blur_out2,      /* mask覆盖mask_aabb区域 */
        &aabb,  /* mask对应的坐标区域，必须大于或等于dest区域 */
    0);

    char_mask.w = 18;
    char_mask.h = 18;
    char_mask.mask = (u8_t *)open_sans_25px_char_038_bitmap;

    ipgui_average_blur_hor(&char_mask, 4, &char_a_mask_blur_out);
    ipgui_average_blur_hor(&char_mask, 4, &char_a_mask_blur_out1);
    ipgui_average_blur_hor(&char_mask, 4, &char_a_mask_blur_out2);

    ipgui_average_blur_ver(&char_a_mask_blur_out2, 4, &char_a_mask_blur_out2);
    ipgui_average_blur_ver(&char_a_mask_blur_out2, 4, &char_a_mask_blur_out2);
    ipgui_average_blur_ver(&char_a_mask_blur_out2, 4, &char_a_mask_blur_out2);

    aabb.start.x = 32 + 16;
    aabb.start.y = 0;
    aabb.end.x = 49 + 16 + 16 + 16;
    aabb.end.y = 49;
    ipgui_blend_color(
        ctx->surf,
        NULL,
        &aabb,
        color11,
        255,
        mask_blur_out2,      /* mask覆盖mask_aabb区域 */
        &aabb,  /* mask对应的坐标区域，必须大于或等于dest区域 */
    0);

    ipgui_font_style_t font_style;
    font_style.blend_mode = IPGUI_BLEND_NORMAL;
    font_style.opacity = 200;
    font_style.line_spacing = 0;
    font_style.font = &open_sans_25px;
    font_style.paint.type = IPGUI_PAINT_COLOR;
IPGUI_COLOR_SET(font_style.paint.src.color, 255, IPGUI_COLOR_WHITE);
    ipgui_draw_builtin_text(
        ctx->surf,
        (ipgui_aabb_t *)0,
        &font_style,
        "ABC&",
        1,
        30);
}

void icon_render(struct ipgui_widget * widget, ipgui_widget_render_ctx_t * ctx)
{
    ipgui_icon_data_t icon_play = {
       .w = 500, .h = 500,
       .mask = (u8_t *)icon_play_mask,
    };
    ipgui_point_t pivot = {0, 0};
    ipgui_point_t anchor = {0, 0};
    ipgui_draw_icon_style_t style;
    style.blend_mode = IPGUI_BLEND_NORMAL;
    style.opacity = 255;

    //图片填充
    ipgui_aabb_t img_aabb;
    img_aabb.start.x = 0;
    img_aabb.start.y = 0;
    img_aabb.end.x = main_bg_img.w - 1;
    img_aabb.end.y = main_bg_img.h - 1;

    ipgui_image_src_t img_src;
    img_src.buf       = main_bg_img.pixmap;
    img_src.img_pxfmt = main_bg_img.fmt;
    img_src.px_size   = main_bg_img.px_size;
    img_src.stride    = main_bg_img.stride;
    img_src.img_aabb  = &img_aabb;

    style.paint.type = IPGUI_PAINT_IMAGE;
    style.paint.src.image_src = img_src;

    // // 渐变填充
    // style.paint.type = IPGUI_PAINT_GRADIENT;
    // style.paint.src.grad_src.grad_type = IPGUI_GRADIENT_TYPE_LINEAR;
    // ipgui_liner_gradient_init_direct(&style.paint.src.grad_src.grad.liner_grad, 
    //     0,0, 240,240);

    // ipgui_gradient_color_stop_t stop0;
    // stop0.pos = 0;
    // IPGUI_COLOR_SET(stop0.color, 255, 0xff0000);
    // ipgui_liner_gradient_add_stop(&style.paint.src.grad_src.grad.liner_grad, &stop0);

    // ipgui_gradient_color_stop_t stop255;
    // stop255.pos = 255;
    // IPGUI_COLOR_SET(stop255.color, 255, 0xffff00);
    // ipgui_liner_gradient_add_stop(&style.paint.src.grad_src.grad.liner_grad, &stop255);

    ipgui_draw_icon(
        ctx->surf,
        NULL,
        &icon_play,
        &pivot,    /* 相对于图标的变换点 如果是子图标那么就是相对于子图标的 */
        &anchor,
        NULL,
        &style);
}

int main(void)
{
    ipgui_input_dispatcher_init(&dispatcher);

    pointer_src.priv_data = (void *)0;
    pointer_src.input_src_event_read_cb = ipgui_sdl_mouse_event_poll;

    ipgui_screen_init(&main_screen, &sdl_drv);
    ipgui_sdl_screen_init(&main_screen);

    pointer_id  = ipgui_dispatcher_register_input_src(&dispatcher, &pointer_src);
    // keyboard_id = ipgui_dispatcher_register_input_src(&dispatcher, &keyboard_src);
    main_scr_id = ipgui_dispatcher_register_screen(&dispatcher, &main_screen);

    ipgui_bind_input_src_with_screen(&dispatcher, pointer_id, main_scr_id);
    // ipgui_bind_input_src_with_screen(&dispatcher, keyboard_id, main_scr_id);

    ipgui_scr_create_pfb(&main_screen, main_screen_frame_buf, sizeof(main_screen_frame_buf), PIX_FMT_RGBA8888);

    if(ipgui_init() != IPGUI_ERR_OK) {
        printf("ipgui_init_err"); return 0;
    }

    main_screen.render_bg = draw_main_screen_backgroud_color;
    ipgui_widget_t * widget4 = ipgui_widget_create(NULL);
    widget4->name = "主背景";
    widget4->render = widget4_render;
    widget4->x = 0;
    widget4->y = 0;
    widget4->w = 1600;
    widget4->h = 480;

        ipgui_widget_t * widget3 = ipgui_widget_create(NULL);
    widget3->name = "波浪";
    widget3->render = widget3_render;
    widget3->x = 0;
    widget3->y = 250;
    widget3->w = 800;
    widget3->h = 140;

    ipgui_widget_t * widget_arc_bg = ipgui_widget_create(NULL);
    widget_arc_bg->name = "圆弧背景";
    widget_arc_bg->render = arc_bg_render;
    widget_arc_bg->x = 0;
    widget_arc_bg->y = 0;
    widget_arc_bg->w = 800;
    widget_arc_bg->h = 480;

    ipgui_widget_t * widget_knob = ipgui_widget_create(NULL);
    widget_knob->name = "旋钮圆";
    widget_knob->render = knob_circle_render;
    widget_knob->event_handler = knob_circle_event_handler;
    widget_knob->x = 60;
    widget_knob->y = 102;
    widget_knob->w = 26;
    widget_knob->h = 26;

    ipgui_widget_t * widget_brightness_label = ipgui_widget_create(NULL);
    widget_brightness_label->name = "亮度数值";
    widget_brightness_label->render = brightness_label_render;
    widget_brightness_label->x = 58;
    widget_brightness_label->y = 265;
    widget_brightness_label->w = 110;
    widget_brightness_label->h = 35;



    // /* create widget */
    // ipgui_widget_t * widget1 = ipgui_widget_create(NULL);
    // widget1->render = widget1_render;
    // widget1->x = 0;
    // widget1->y = 0;
    // widget1->w = 800;
    // widget1->h = 479;

    // ipgui_widget_t * widget2 = ipgui_widget_create(widget1);
    // widget2->render = widget2_render;
    // widget2->x = 0;
    // widget2->y = 0;
    // widget2->w = 800;
    // widget2->h = 479;



    ipgui_img_dsc_t img_dsc;
    if(0 != test_bmp("M:/test/ESDBox_IPGUI/widget/src/wave.bmp", &img_dsc))
    {
        printf("Failed to load bitmap image, please run the program at the main.c page\n");
        return -1;
    }


    ipgui_img_dsc_t img_bg_dsc;
    if(0 != test_bmp("M:/test/ESDBox_IPGUI/widget/src/bg.bmp", &img_bg_dsc))
    {
        printf("Failed to load bitmap image, please run the program at the main.c page\n");
        return -1;
    }
    ipgui_img_dsc_t img_power_dsc;
    if(0 != test_bmp("M:/test/ESDBox_IPGUI/widget/src/lightning.bmp", &img_power_dsc))
    {
        printf("Failed to load bitmap image, please run the program at the main.c page\n");
        return -1;
    }

    main_bg_img.pixmap = img_bg_dsc.pixmap;
    main_bg_img.px_size = img_bg_dsc.stride / img_bg_dsc.w;
    main_bg_img.fmt = IPGUI_IMG_FMT_RGBA8888;
    main_bg_img.stride = img_bg_dsc.stride;
    main_bg_img.w = img_bg_dsc.w;
    main_bg_img.h = img_bg_dsc.h;


    wave_img.pixmap = img_dsc.pixmap;
    wave_img.px_size = img_dsc.stride / img_dsc.w;
    wave_img.fmt = IPGUI_IMG_FMT_RGBA8888;
    wave_img.stride = img_dsc.stride;
    wave_img.w = img_dsc.w;
    wave_img.h = img_dsc.h;

    power_img.pixmap = img_power_dsc.pixmap;
    power_img.px_size = img_power_dsc.stride / img_power_dsc.w;
    power_img.fmt = IPGUI_IMG_FMT_BGRA8888;
    power_img.stride = img_power_dsc.stride;
    power_img.w = img_power_dsc.w;
    power_img.h = img_power_dsc.h;

    ipgui_img_dsc_t img_tablelamp_dsc;
    if(0 != test_bmp("M:/test/ESDBox_IPGUI/widget/src/tablelamp.bmp", &img_tablelamp_dsc))
    {
        printf("Failed to load bitmap image, please run the program at the main.c page\n");
        return -1;
    }
    tablelamp_img.pixmap = img_tablelamp_dsc.pixmap;
    tablelamp_img.px_size = img_tablelamp_dsc.stride / img_tablelamp_dsc.w;
    tablelamp_img.fmt = IPGUI_IMG_FMT_BGRA8888;
    tablelamp_img.stride = img_tablelamp_dsc.stride;
    tablelamp_img.w = img_tablelamp_dsc.w;
    tablelamp_img.h = img_tablelamp_dsc.h;

    ipgui_img_dsc_t img_tablelamp_on_dsc;
    if(0 != test_bmp("M:/test/ESDBox_IPGUI/widget/src/tablelamp_on.bmp", &img_tablelamp_on_dsc))
    {
        printf("Failed to load bitmap image, please run the program at the main.c page\n");
        return -1;
    }
    tablelamp_on_img.pixmap = img_tablelamp_on_dsc.pixmap;
    tablelamp_on_img.px_size = img_tablelamp_on_dsc.stride / img_tablelamp_on_dsc.w;
    tablelamp_on_img.fmt = IPGUI_IMG_FMT_BGRA8888;
    tablelamp_on_img.stride = img_tablelamp_on_dsc.stride;
    tablelamp_on_img.w = img_tablelamp_on_dsc.w;
    tablelamp_on_img.h = img_tablelamp_on_dsc.h;

    ipgui_widget_t * widget_switch = ipgui_widget_create(NULL);
    widget_switch->name = "开关";
    widget_switch->render = widget_switch_render;
    widget_switch->event_handler = widget_switch_event_handler;
    widget_switch->x = 365;
    widget_switch->y = 400;
    widget_switch->w = 90;
    widget_switch->h = 40;

    ipgui_widget_t * widget_switch_label = ipgui_widget_create(NULL);
    widget_switch_label->name = "开关标签";
    widget_switch_label->render = widget_switch_label_render;
    widget_switch_label->x = 362;
    widget_switch_label->y = 376;
    widget_switch_label->w = 110;
    widget_switch_label->h = 30;

    ipgui_widget_t * widget_arc_label = ipgui_widget_create(NULL);
    widget_arc_label->name = "圆弧标签";
    widget_arc_label->render = widget_arc_label_render;
    widget_arc_label->x = 55;
    widget_arc_label->y = 260;
    widget_arc_label->w = 120;
    widget_arc_label->h = 80;

    ipgui_widget_t * widget_power = ipgui_widget_create(NULL);
    widget_power->name = "电源图标和标签";
    widget_power->render = power_render;
    widget_power->x = 350;
    widget_power->y = 50;
    widget_power->w = 130;
    widget_power->h = 100;

    ipgui_widget_t * widget_tablelamp = ipgui_widget_create(NULL);
    widget_tablelamp->name = "台灯图标和标签";
    widget_tablelamp->render = tablelamp_render;
    widget_tablelamp->event_handler = tablelamp_event_handler;
    widget_tablelamp->x = 630;
    widget_tablelamp->y = 50;
    widget_tablelamp->w = 100;
    widget_tablelamp->h = 100;

    ipgui_widget_t * widget_labal_livingroom = ipgui_widget_create(NULL);
    widget_labal_livingroom->name = "客厅标签";
    widget_labal_livingroom->render =_label_livingroom_render;
    widget_labal_livingroom->event_handler = livingroom_event_handler;
    widget_labal_livingroom->x = 628;
    widget_labal_livingroom->y = 435;
    widget_labal_livingroom->w = 200;
    widget_labal_livingroom->h = 60;

    // //测试图标渲染
    // ipgui_widget_t * widget_icon = ipgui_widget_create(NULL);
    // widget_icon->name = "小红书";
    // widget_icon->render = icon_render;
    // widget_icon->x = 0;
    // widget_icon->y = 0;
    // widget_icon->w = 250;
    // widget_icon->h = 250;

    /* ========== 测试：滚动 vs 拖拽 ==========
     * wid1: 屏幕级, SCROLLABLE, 不可拖拽
     * wid2: wid1 的子控件, SCROLLABLE, 不可拖拽
     * wid3: wid1 的子控件, 不可滚动, 可拖拽
     * wid4: wid2 的子控件, 什么都不可
     */
    /* wid1: 根级可滚动容器 */
    ipgui_widget_t * wid1 = ipgui_widget_create(NULL);
    wid1->name = "wid1_scroll";
    wid1->x = 380; wid1->y = 20;
    wid1->w = 380; wid1->h = 420;
    wid1->flags |= IPGUI_WIDGET_FLAG_SCROLLABLE;
    wid1->event_handler = NULL;
    wid1->priv_data = (void *)&col_blue;
    wid1->render = color_render;

    /* wid2: 嵌套可滚动容器（wid1 的子控件） */
    ipgui_widget_t * wid2 = ipgui_widget_create(wid1);
    wid2->name = "wid2_scroll";
    wid2->x = 10; wid2->y = 100;
    wid2->w = 200; wid2->h = 220;
    wid2->flags |= IPGUI_WIDGET_FLAG_SCROLLABLE;
    wid2->scroll_dir = IPGUI_SCROLL_DIR_GESTURE;
    wid2->event_handler = NULL;
    wid2->priv_data = (void *)&col_green;
    wid2->render = color_render;


    /* wid2 的子控件：模拟长列表内容，超出 wid2 可视区即可触发滚动 */
    static widget_color_t item_colors[] = {
        {0xe0, 0x5d, 0x5d}, {0x5d, 0xb8, 0x5d}, {0x4a, 0x90, 0xd9},
        {0xf0, 0x8a, 0x3a}, {0x9b, 0x59, 0xb6}, {0xe9, 0x45, 0x60},
        {0x00, 0xb4, 0xd8}, {0xf4, 0xa2, 0x61}, {0x2a, 0x9d, 0x8f},
        {0x83, 0x38, 0xec}, {0xff, 0x00, 0x6e}, {0x3a, 0x86, 0xff},
        {0xfb, 0x56, 0x07}, {0x06, 0xd6, 0xa0}, {0xc7, 0x7d, 0xff},
    };
    static char item_names[15][20];
    for (int i = 0; i < 15; i++) {
        ipgui_widget_t * item = ipgui_widget_create(wid2);
        snprintf(item_names[i], sizeof(item_names[i]), "Item %d", i);
        item->name = item_names[i];
        item->x = 5 + i * 85;
        item->y = 5;
        item->w = 80;
        item->h = 210;
        item->priv_data = (void *)&item_colors[i % 15];
        item->render = color_render;
    }

    /* wid3: wid1 的子控件，可拖拽 */
    ipgui_widget_t * wid3 = ipgui_widget_create(wid1);
    wid3->name = "wid3_drag";
    wid3->x = 230; wid3->y = 100;
    wid3->w = 130; wid3->h = 220;
    wid3->priv_data = (void *)&col_orange;
    wid3->render = color_render;
    wid3->event_handler = drag_handler;
    /* 不设 SCROLLABLE —— 不可滚动，但 drag_handler 直接改 x/y */

    /* ========== 曲线控件 ========== */
    ipgui_widget_t * widget_curve = ipgui_widget_create(NULL);
    widget_curve->name   = "实时曲线";
    widget_curve->render = curve_render;
    widget_curve->x      = 0;
    widget_curve->y      = 0;
    widget_curve->w      = 800;
    widget_curve->h      = 480;

    ipgui_widget_t * blur_widget = ipgui_widget_create(NULL);
    blur_widget->name   = "blur test";
    blur_widget->render = blur_render;
    blur_widget->x      = 0;
    blur_widget->y      = 0;
    blur_widget->w      = 800;
    blur_widget->h      = 480;

    ipgui_input_src_evt_t raw_evt;
    while(1)
    {
        /* 读取原始事件 */
        pointer_src.input_src_event_read_cb(&pointer_src, &raw_evt);
        /* push into queue */
        ipgui_norm_queue_post(&dispatcher.evt_queue, &raw_evt, sizeof(raw_evt));

        // printf("原始事件: 输入源ID=%d, 事件=%s, 坐标(%d, %d)\n", 
        //     raw_evt.input_src_id, 
        //     raw_evt.input_src_evt == IPGUI_INPUT_SRC_EVENT_POINTER_PRESS  ? "按压" :
        //     raw_evt.input_src_evt == IPGUI_INPUT_SRC_EVENT_POINTER_RELEASE ? "释放" : "其他",
        //     raw_evt.evt_info.pointer_pos.x, 
        //     raw_evt.evt_info.pointer_pos.y);

        /* 悬停状态每帧复位，dispatch 会在当帧重新设置 */
        widget_livingroom_update(widget_labal_livingroom);
        widget_tablelamp_update(widget_tablelamp);
        
        ipgui_dispatch_input_event(&dispatcher);
        /* 驱动所有动画，需在渲染之前调用 */
        ipgui_anim_update_all();
        ipgui_screen_render(&main_screen);

        /* 每个控件自己的 per-frame update */
        widget_switch_label_update(widget_switch_label);
        widget_switch_update(widget_switch);
        widget_knob_update(widget_brightness_label);
        widget_wave_update(widget3);
        widget_curve_update(widget_curve);

        /* 心跳 */
        ipgui_tick_inc();
        Sleep(1);

    }


	return 0;
}
