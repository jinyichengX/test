#include "ipgui_queue.h"
#include "ipgui_timer.h"
#include "ipgui_list.h"
#include "SDL.h"
#include "ipgui_memory.h"
#include "ipgui_debug.h"
#include "ipgui_core.h"
#include "ipgui_pattle.h"
#include "ipgui_color.h"
#include "ipgui_membox.h"
#include "sdl_draw.h"
#include "ipgui_vfs.h"
#include "ipgui_screen.h"
#include "sdl_input_event.h"
#include "sdl_draw.h"
#include "ipgui_widget.h"
#include <time.h>
#include "ipgui_color.h"
#include "ipgui_image_dec.h"
#include "ipgui_mempool.h"
#include "ipgui_graphic2.h"
#include "ipgui_darray.h"
#include "ipgui_debug.h"
#include "ipgui_image.h"
#include "ipgui_input.h"
#include "ipgui_ring_mask.h"
#include "ipgui_blend_color.h"
#include "ipgui_blend_gradient_color.h"
#include "ipgui_blend_image.h"
#include "ipgui_widget_tree.h"
#include "ipgui_math.h"
#include "ipgui_vector.h"
#include "ipgui_draw_line.h"
#include "ipgui_draw_pixel.h"
#include "ipgui_draw_image.h"
#include "open_sans.h"
#include "ipgui_box_style.h"
#include "ipgui_draw_box_background.h"
#include "ipgui_draw_box_shadow.h"
#include "ipgui_draw_box_border.h"
#include "ipgui_edge_halfplane_mask.h"
#include "ipgui_edge_wdf_mask.h"
#include "ipgui_draw_triangle.h"
#include "ipgui_draw_arc.h"
#include "ipgui_draw_polygon.h"
#include "ipgui_draw_builtin_font.h"
#include "ipgui_gradient_color.h"
#include "ipgui_image_geometry_transform.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#undef main

int test_bmp(const s8_t * path, ipgui_img_dsc_t * image);

/* ipgui_button.h */
typedef enum {
    IPGUI_STATE_NORMAL,
    IPGUI_STATE_PRESSED,
    IPGUI_STATE_HOVER,
    IPGUI_STATE_DISABLED
} ipgui_control_state_t;

typedef struct {
    ipgui_box_bg_style_t     bg;
    ipgui_box_border_style_t border;
    ipgui_font_style_t       text_style;
    ipgui_box_style_t        box_shape; /* 包含 padding 和 radius */
} ipgui_button_style_t;

typedef struct {
    ipgui_aabb_t          area;
    const s8_t          * text;
    ipgui_control_state_t state;
} ipgui_button_t;

/* ipgui_button.c */
void ipgui_draw_button(
    ipgui_surf_t         * surf,
    ipgui_aabb_t         * clip,
    ipgui_button_t       * btn,
    ipgui_button_style_t * style)
{
    if (!surf || !btn || !style) return;

    /* 1. 绘制背景 (内部已处理 AABB 裁剪) */
    ipgui_draw_box_background(surf, clip, &btn->area, &style->box_shape, &style->bg);

    /* 2. 绘制边框 */
    ipgui_draw_box_border(surf, clip, &btn->area, &style->box_shape, &style->border);

    /* 3. 绘制文字 (居中计算) */
    if (btn->text && style->text_style.font) {
        ipgui_coord_t txt_w = ipgui_builtin_text_width(style->text_style.font, btn->text);
        ipgui_coord_t txt_h = style->text_style.font->line_height;
        
        /* 计算居中坐标 */
        ipgui_coord_t box_w = ipgui_aabb_width(&btn->area);
        ipgui_coord_t box_h = ipgui_aabb_height(&btn->area);
        ipgui_coord_t tx = btn->area.start.x + ((box_w - txt_w) >> 1);
        ipgui_coord_t ty = btn->area.start.y + ((box_h - txt_h) >> 1);

        ipgui_draw_builtin_text(surf, clip, &style->text_style, btn->text, tx, ty);
    }
}
void clear_fucking_screen(ipgui_scr_t * scr)
{   
    int x = 0;
    int y = 0;
    ipgui_color_t color;
    IPGUI_COLOR_SET(color, 255, IPGUI_COLOR_GRAY);
    for( x = 0; x < scr->drv->xreso; x ++)
    {
        for( y = 0; y < scr->drv->yreso; y ++)
        {
            ipgui_screen_putpixel(scr, x, y, (unsigned char *)&color);
        }
    }
    ipgui_screen_flush(scr);
}

clock_t start, end;
int sdl_mouse_read(struct ipgui_input_drv_t * dev, ipgui_input_data_t * data);
ipgui_input_drv_t drv = {
    .priv_data = "sdl_mouse",
    .type = IPGUI_INPUT_TYPE_PID,
    .read = sdl_mouse_read,
};
ipgui_scr_t * sdl_scr;
ipgui_color_t g_color;
ipgui_surf_t surf;
ipgui_aabb_t clip;
int cnt11 = 0;

#define RENDER_MODE 3
// #define RENDER_MODE 1 /* 单行渲染 */
// #define RENDER_MODE 2 /* 单行渲染 */
// #define RENDER_MODE 3 /* 全屏渲染 */
int main(void)
{
    ipgui_edge_wdf_param_t p;
    p = ipgui_edge_wdf_param_init(0,0, 1000, 1000);
    
    ipgui_coord_t x_span;
    x_span = ipgui_edge_wdf_xspan(&p, 300);

    printf("x_span: %d\n");
    // return 0;

    IPGUI_COLOR_SET(g_color, 255, IPGUI_COLOR_RED);
    if(ipgui_init() != IPGUI_ERR_OK)
    {
        printf("ipgui_init_err");
        return 0;
    }

    /* GUI */
    ipgui_input_dev_t * sdl_mouse = ipgui_sdl_mouse_create_init();
    ipgui_input_dev_t * sdl_keyboard = ipgui_sdl_create_keyboard_init();
    sdl_scr = ipgui_sdl_screen_create();
    ipgui_screen_register_input_device(sdl_scr, sdl_mouse);
    ipgui_screen_register_input_device(sdl_scr, sdl_keyboard);
    ipgui_input_device_register(&drv, sdl_scr, 10);
    clear_fucking_screen(sdl_scr);


    surf.color = ((struct sdl_private_t *)sdl_scr->drv->pri_data)->framebuffer;
    surf.surf.start.x = 0;
    surf.surf.start.y = 0;
    surf.surf.end.x = sdl_scr->drv->xreso - 1;
    surf.surf.end.y = sdl_scr->drv->yreso - 1;
    surf.pix_fmt = PIX_FMT_RGBA8888;
    surf.pix_size = 4;
    // surf.pix_fmt = PIX_FMT_RGB888;
    // surf.pix_size = 3;
    surf.stride = sdl_scr->drv->xreso * surf.pix_size;

    clip.start.x = 0;
    clip.start.y = 0;
    clip.end.x = 0;
    clip.end.y = 0;


    // ipgui_color_t color[sdl_scr->drv->yreso * sdl_scr->drv->xreso];
    ipgui_line_t line;
    line.start.x = 0;
    line.start.y = 0;
    line.end.x = 100;
    line.end.y = 100;
    ipgui_line_style_t line_style;
    line_style.opacity = 255;
    line_style.blend_mode = 0;
    line_style.width = 10;
    line_style.paint.type = IPGUI_PAINT_GRADIENT;

    line_style.paint.src.color = g_color;
    line_style.paint.src.grad_src.grad_type = IPGUI_GRADIENT_TYPE_LINEAR;
    ipgui_liner_gradient_init_direct(&line_style.paint.src.grad_src.grad.liner_grad, 
    line.start.x, line.start.y, line.start.x, line.end.y);
    ipgui_gradient_color_stop_t stop00;
    stop00.pos = 0;
    IPGUI_COLOR_SET(stop00.color, 255, IPGUI_COLOR_BLUE);
    ipgui_gradient_color_stop_t stop01;
    stop01.pos = 255;
    IPGUI_COLOR_SET(stop01.color, 255, IPGUI_COLOR_RED);
    ipgui_liner_gradient_add_stop(&line_style.paint.src.grad_src.grad.liner_grad, &stop00);
    ipgui_liner_gradient_add_stop(&line_style.paint.src.grad_src.grad.liner_grad, &stop01);

    ipgui_aabb_t box;
    box.start.x = 0 + 300;
    box.start.y = 0 + 300;
    box.end.x = 199+ 300;
    box.end.y = 99+ 300;
    
    ipgui_box_style_t box_style;
    box_style.bottom_padding = 0;
    box_style.left_padding = 0;
    box_style.top_padding = 0;
    box_style.right_padding = 0;

    box_style.left_bottom_radius = 20;
    box_style.left_top_radius = 20;
    box_style.right_bottom_radius = 20;
    box_style.right_top_radius = 20;

    ipgui_box_border_style_t box_border_style;
    box_border_style.blend_mode = 0;
    box_border_style.width = 3;
    box_border_style.opacity = 155;
    box_border_style.paint.type = IPGUI_PAINT_COLOR;
    IPGUI_COLOR_SET(box_border_style.paint.src.color, 255, IPGUI_COLOR_463);
    // IPGUI_COLOR_SET(box_border_style.paint.src.color, 255, IPGUI_COLOR_283);
    // box_border_style.paint.src.grad_src.grad_type = IPGUI_GRADIENT_TYPE_RADIAL;
    // ipgui_radial_gradient_init(&box_border_style.paint.src.grad_src.grad.radial_grad, 
    //     (box.start.x + box.end.x) / 2, (box.start.y + box.end.y) / 2, 330);
    // ipgui_gradient_color_stop_t stop3;
    // stop3.pos = 0;
    // IPGUI_COLOR_SET(stop3.color, 255, IPGUI_COLOR_118);
    // ipgui_gradient_color_stop_t stop4;
    // stop4.pos = 255;
    // IPGUI_COLOR_SET(stop4.color, 255, IPGUI_COLOR_160);
    // ipgui_radial_gradient_add_stop(&box_border_style.paint.src.grad_src.grad.radial_grad, &stop3);
    // ipgui_radial_gradient_add_stop(&box_border_style.paint.src.grad_src.grad.radial_grad, &stop4);


    ipgui_img_dsc_t img_dsc;
    if(0 != test_bmp("M:/test/ESDBox_IPGUI/core/image/decoder/material/bmp/paimeng24.bmp", &img_dsc))
    {
        printf("Failed to load bitmap image, please run the program at the main.c page\n");
        return -1;
    }

    ipgui_image_data_t img_data;
    img_data.pixmap = img_dsc.pixmap;
    img_data.px_size = img_dsc.stride / img_dsc.w;
    img_data.fmt = IPGUI_IMG_FMT_BGR888;//可以改成L8或者LA88或者RGB565试试，虽然这么改逻辑上不对，但是有效果
    img_data.stride = img_dsc.stride;
    img_data.w = img_dsc.w;
    img_data.h = img_dsc.h;

    ipgui_img_dsc_t img2_dsc;
    if(0 != test_bmp("M:/test/ESDBox_IPGUI/core/image/decoder/material/bmp/keli.bmp", &img2_dsc))
    {
        printf("Failed to load bitmap image, please run the program at the main.c page\n");
        return -1;
    }
    ipgui_image_data_t img2_data;
    img2_data.pixmap = img2_dsc.pixmap;
    img2_data.px_size = img2_dsc.stride / img2_dsc.w;
    img2_data.fmt = IPGUI_IMG_FMT_BGR888;//可以改成L8或者LA88或者RGB565试试，虽然这么改逻辑上不对，但是有效果
    img2_data.stride = img2_dsc.stride;
    img2_data.w = img2_dsc.w;
    img2_data.h = img2_dsc.h;

    ipgui_point_t pivot;
    pivot.x = img_dsc.w / 2;
    pivot.y = img_dsc.h / 2;
    ipgui_point_t anchor;
    anchor.x = sdl_scr->drv->xreso / 2 + 200;
    anchor.y = sdl_scr->drv->yreso / 2;
// anchor.x = anchor.y = 0;

    ipgui_image_draw_style_t img_style;
    img_style.blend_mode = 0;
    img_style.opacity = 255;
    ipgui_image_draw_style_t img2_style;
    img2_style.blend_mode = 0;
    img2_style.opacity = 100;

    ipgui_box_bg_style_t box_bg_style;
    box_bg_style.blend_mode = 0;
    box_bg_style.opacity = 200;
    box_bg_style.paint.type = IPGUI_PAINT_GRADIENT;
    // box_bg_style.paint.src.image_src = image_src1;
    box_bg_style.paint.src.color = g_color;
    box_bg_style.paint.src.grad_src.grad_type = IPGUI_GRADIENT_TYPE_LINEAR;
    ipgui_liner_gradient_init_direct(&box_bg_style.paint.src.grad_src.grad.liner_grad, 
        300, 300, 500, 419);
    ipgui_gradient_color_stop_t stop1;
    stop1.pos = 0;
    IPGUI_COLOR_SET(stop1.color, 255, 0x283c86);
    ipgui_gradient_color_stop_t stop2;
    stop2.pos = 255;
    IPGUI_COLOR_SET(stop2.color, 255, 0x45a247);
    ipgui_liner_gradient_add_stop(&box_bg_style.paint.src.grad_src.grad.liner_grad, &stop1);
    ipgui_liner_gradient_add_stop(&box_bg_style.paint.src.grad_src.grad.liner_grad, &stop2);

    // ipgui_radial_gradient_init(&box_bg_style.paint.src.grad_src.grad.radial_grad, 
    // (box.start.x + box.end.x) >> 1, (box.start.y + box.end.y) >> 1, 150);
    // ipgui_gradient_color_stop_t stop1;
    // stop1.pos = 0;
    // IPGUI_COLOR_SET(stop1.color, 255, IPGUI_COLOR_RED);
    // ipgui_gradient_color_stop_t stop2;
    // stop2.pos = 255;
    // IPGUI_COLOR_SET(stop2.color, 255, IPGUI_COLOR_BLUE);
    // ipgui_radial_gradient_add_stop(&box_bg_style.paint.src.grad_src.grad.radial_grad, &stop1);
    // ipgui_radial_gradient_add_stop(&box_bg_style.paint.src.grad_src.grad.radial_grad, &stop2);

    // box_bg_style.paint.src.grad_src.grad_type = IPGUI_GRADIENT_TYPE_CONIC;
    // ipgui_conic_gradient_init(&box_bg_style.paint.src.grad_src.grad.conic_grad, 
    //     (box.start.x + box.end.x) / 2, (box.start.y + box.end.y) / 2, 330);
    // ipgui_gradient_color_stop_t stop5;
    // stop5.pos = 100;
    // IPGUI_COLOR_SET(stop5.color, 255, IPGUI_COLOR_519);
    // ipgui_gradient_color_stop_t stop6;
    // stop6.pos = 200;
    // IPGUI_COLOR_SET(stop6.color, 255, IPGUI_COLOR_356);
    // ipgui_conic_gradient_add_stop(&box_bg_style.paint.src.grad_src.grad.conic_grad, &stop5);
    // ipgui_conic_gradient_add_stop(&box_bg_style.paint.src.grad_src.grad.conic_grad, &stop6);

    static ipgui_point_t tri_p1;
    tri_p1.x = 220;
    tri_p1.y = 100;
    static ipgui_point_t tri_p2;
    tri_p2.x = 140;
    tri_p2.y = 420;
    static ipgui_point_t tri_p3;
    tri_p3.x = 300;
    tri_p3.y = 390;

    ipgui_triangle_style_t tri_style;
    tri_style.blend_mode = 0;
    tri_style.opacity = 100;
    tri_style.paint.src.color = g_color;
    tri_style.paint.type = IPGUI_PAINT_GRADIENT;

    // tri_style.paint.src.grad_src.grad_type = IPGUI_GRADIENT_TYPE_LINEAR;
    // ipgui_liner_gradient_init_direct(&tri_style.paint.src.grad_src.grad.liner_grad, 
    //     tri_p1.x, tri_p1.y, tri_p2.x, tri_p2.y);
    // ipgui_gradient_color_stop_t stop11;
    // stop11.pos = 0;
    // IPGUI_COLOR_SET(stop11.color, 255, IPGUI_COLOR_YELLOW);
    // ipgui_gradient_color_stop_t stop22;
    // stop22.pos = 255;
    // IPGUI_COLOR_SET(stop22.color, 255, IPGUI_COLOR_40);
    // ipgui_liner_gradient_add_stop(&tri_style.paint.src.grad_src.grad.liner_grad, &stop11);
    // ipgui_liner_gradient_add_stop(&tri_style.paint.src.grad_src.grad.liner_grad, &stop22);

    
    tri_style.paint.src.grad_src.grad_type = IPGUI_GRADIENT_TYPE_RADIAL;
    ipgui_radial_gradient_init(&tri_style.paint.src.grad_src.grad.radial_grad, 
        tri_p1.x, tri_p1.y, 500);
    ipgui_gradient_color_stop_t stop11;
    stop11.pos = 0;
    IPGUI_COLOR_SET(stop11.color, 255, IPGUI_COLOR_4);
    ipgui_gradient_color_stop_t stop22;
    stop22.pos = 255;
    IPGUI_COLOR_SET(stop22.color, 255, IPGUI_COLOR_288);
    ipgui_radial_gradient_add_stop(&tri_style.paint.src.grad_src.grad.liner_grad, &stop11);
    ipgui_radial_gradient_add_stop(&tri_style.paint.src.grad_src.grad.liner_grad, &stop22);

    ipgui_arc_t arc;
    arc.cx = 200;
    arc.cy = 200;
    arc.dir = IPGUI_ARC_DRAW_DIR_CCW;
    arc.er = 200;
    arc.ir = 176;
    arc.start = 50;
    arc.angle = 230;

    ipgui_arc_style_t arc_style;
    arc_style.blend_mode = 0;
    arc_style.sep_type = IPGUI_ARC_ENDPOINT_TYPE_ROUND;
    arc_style.eep_type = IPGUI_ARC_ENDPOINT_TYPE_ROUND;
    arc_style.opacity = 255;
    arc_style.paint.type = IPGUI_PAINT_GRADIENT;
    IPGUI_COLOR_SET(arc_style.paint.src.color, 255, IPGUI_COLOR_424);

        arc_style.paint.src.grad_src.grad_type = IPGUI_GRADIENT_TYPE_CONIC;
    ipgui_conic_gradient_init(&arc_style.paint.src.grad_src.grad.conic_grad, 
        arc.cx, arc.cy, 30);
    ipgui_gradient_color_stop_t stop21;
    stop21.pos = 0;
    IPGUI_COLOR_SET(stop21.color, 255, IPGUI_COLOR_4);
    ipgui_gradient_color_stop_t stop31;
    stop31.pos = 255;
    IPGUI_COLOR_SET(stop31.color, 255, IPGUI_COLOR_390);
    ipgui_gradient_color_stop_t stop41;
    stop41.pos = 128;
    IPGUI_COLOR_SET(stop41.color, 200, IPGUI_COLOR_RED);
    ipgui_gradient_color_stop_t stop51;
    stop51.pos = 192;
    IPGUI_COLOR_SET(stop51.color, 255, IPGUI_COLOR_313);
    ipgui_conic_gradient_add_stop(&arc_style.paint.src.grad_src.grad.conic_grad, &stop21);
    ipgui_conic_gradient_add_stop(&arc_style.paint.src.grad_src.grad.conic_grad, &stop31);
    ipgui_conic_gradient_add_stop(&arc_style.paint.src.grad_src.grad.conic_grad, &stop41);
    ipgui_conic_gradient_add_stop(&arc_style.paint.src.grad_src.grad.conic_grad, &stop51);

    ipgui_font_style_t font_style;
    font_style.blend_mode = 0;

    font_style.opacity = 255;
    font_style.line_spacing = 0;
    font_style.font = &open_sans_23px;
    font_style.paint.type = IPGUI_PAINT_GRADIENT;
    IPGUI_COLOR_SET(font_style.paint.src.color, 255, IPGUI_COLOR_288);
        font_style.paint.src.grad_src.grad_type = IPGUI_GRADIENT_TYPE_LINEAR;
    ipgui_liner_gradient_init_direct(&font_style.paint.src.grad_src.grad.liner_grad, 
        0,0, 400,0);
    ipgui_gradient_color_stop_t stop111;
    stop111.pos = 0;
    IPGUI_COLOR_SET(stop111.color, 255, IPGUI_COLOR_BLACK);
    ipgui_gradient_color_stop_t stop222;
    stop222.pos = 255;
    IPGUI_COLOR_SET(stop222.color, 255, IPGUI_COLOR_483);
    ipgui_liner_gradient_add_stop(&font_style.paint.src.grad_src.grad.liner_grad, &stop111);
    ipgui_liner_gradient_add_stop(&font_style.paint.src.grad_src.grad.liner_grad, &stop222);

    ipgui_surf_t surf1;
    surf1.pix_fmt = PIX_FMT_RGBA8888;
    surf1.pix_size = 4;
#if RENDER_MODE == 1
    surf1.stride = 4;    u8_t offline_buffer[4];
#elif RENDER_MODE == 2
    surf1.stride = 4 * 800;    u8_t offline_buffer[4 * 800];
#elif RENDER_MODE == 3
    surf1.stride = 4 * 800;    u8_t offline_buffer[4 * 800 * 480];
#endif

    surf1.color = offline_buffer;


    ipgui_box_shadow_style_t shadow_style;
    IPGUI_COLOR_SET(shadow_style.color, 255, IPGUI_COLOR_4);
    shadow_style.blur          = 20;
    shadow_style.spread        = 0;
    shadow_style.offset_x      = 0;
    shadow_style.offset_y      = 0;
    shadow_style.corner_radius = 20;
    shadow_style.opacity       = 200;
    shadow_style.blend_mode    = 0;

ipgui_button_t btn1 = {
.area = {.start.x = 100, .start.y = 100, .end.x = 200, .end.y = 160},
.state = IPGUI_STATE_NORMAL,
.text = "click"
};
ipgui_button_style_t btn_style = {
    .bg = box_bg_style,
    .border = box_border_style,
    .box_shape = box_style,
    .text_style = font_style,
};


    while(1) {
#if RENDER_MODE == 1
        for (int y = 0; y < sdl_scr->drv->yreso; y ++) {
            for (int x = 0; x < sdl_scr->drv->xreso; x ++) {
#elif RENDER_MODE == 2
        for (int y = 0; y < sdl_scr->drv->yreso; y ++) {
#elif RENDER_MODE == 3

#endif

#if RENDER_MODE == 1
                ipgui_color_t * color = (ipgui_color_t *)offline_buffer;
                IPGUI_COLOR_SET((*color), 255, IPGUI_COLOR_WHITE);
                surf1.surf.start.x = x;
                surf1.surf.start.y = y;
                surf1.surf.end.x = x;
                surf1.surf.end.y = y;
#elif RENDER_MODE == 2
                ipgui_color_t * color;
                for (int i = 0; i < sdl_scr->drv->xreso; i++) {
                    color = (ipgui_color_t *)offline_buffer + i;
                    IPGUI_COLOR_SET((*color), 255, IPGUI_COLOR_WHITE);
                }
                surf1.surf.start.x = 0;
                surf1.surf.start.y = y;
                surf1.surf.end.x   = sdl_scr->drv->xreso - 1;
                surf1.surf.end.y   = y;
#elif RENDER_MODE == 3
                ipgui_color_t * row0 = (ipgui_color_t *)offline_buffer;
                for (int i = 0; i < sdl_scr->drv->xreso; i ++) {
                    IPGUI_COLOR_SET(row0[i], 255, IPGUI_COLOR_WHITE);
                }
                for (int y = 1; y < sdl_scr->drv->yreso; y ++) {
                    memcpy(offline_buffer + y * surf1.stride, offline_buffer, surf1.stride);
                }
                surf1.surf.start.x = 0;
                surf1.surf.start.y = 0;
                surf1.surf.end.x   = sdl_scr->drv->xreso - 1;
                surf1.surf.end.y   = sdl_scr->drv->yreso- 1;
#endif
                /* 画图开始 */
                // ipgui_draw_button(  
                //     &surf1, 
                //     NULL,
                //     &btn1,
                //     &btn_style);

                // ipgui_draw_box_shadow(&surf1, 
                //     NULL, 
                //     &box, 
                //     &shadow_style);

                // ipgui_draw_image(
                //     &surf1,
                //     NULL,
                //     &img_data,
                //     &pivot,
                //     &anchor,
                //     NULL,
                //     &img_style
                // );

                // ipgui_draw_image(
                //     &surf1,
                //     NULL,
                //     &img2_data,
                //     &pivot,
                //     &anchor,
                //     NULL,
                //     &img2_style
                // );


                // ipgui_draw_arc(
                //     &surf1, 
                //     NULL,
                //     &arc, 
                //     &arc_style);

                // ipgui_draw_triangle(
                //     &surf1,
                //     NULL,
                //     &tri_p1, &tri_p2, &tri_p3,
                //     &tri_style);

                // ipgui_draw_box_background(
                //     &surf1,
                //     NULL,
                //     &box,
                //     &box_style,
                //     &box_bg_style);

                // ipgui_draw_box_border(
                //     &surf1,
                //     NULL,
                //     &box,
                //     &box_style,
                //     &box_border_style);

                // ipgui_draw_builtin_text(
                //     &surf1,
                //     NULL,
                //     &font_style,
                //     "hello kitty@@@@@ a lazy dog%%5 ^&*())__+;'",
                //     50,
                //     50);
                ipgui_draw_line(
                    &surf1,
                    NULL,
                    &line,
                    &line_style);
                /* 画图结束 */
#if RENDER_MODE == 1
                ipgui_screen_fill_region(sdl_scr, 
                x, y, x, y, 
                surf1.color, surf1.stride);
#elif RENDER_MODE == 2
                ipgui_screen_fill_region(sdl_scr, 
                0, y, sdl_scr->drv->xreso - 1, y, 
                surf1.color, surf1.stride);
#elif RENDER_MODE == 3
                ipgui_screen_fill_region(sdl_scr, 
                    0, 0, sdl_scr->drv->xreso - 1, sdl_scr->drv->yreso - 1, 
                    surf1.color, surf1.stride);
#endif

#if RENDER_MODE == 1
            }
        }
#elif RENDER_MODE == 2
        }
#elif RENDER_MODE == 3
        
#endif
        sdl_scr->drv->flush(sdl_scr);
arc.start += 1;
// arc_style.paint.src.grad_src.grad.conic_grad.angle_start ++;
        /* 改变位置 */
        // if(cnt11 ++ < 400) {
        //     box.start.x += 1;
        //     box.start.y += 1;
        //     box.end.x += 1;
        //     box.end.y += 1;

        //     line.start.x += 1;
        //     line.start.y += 1;
        //     line.end.x += 1;
        //     line.end.y += 1;
        //     clip.end.x ++;
        //     clip.end.y ++;

            
        //     // box_style.left_bottom_radius ++;
        //     // box_style.left_top_radius ++;
        //     // box_style.right_bottom_radius ++;
        //     // box_style.right_top_radius ++;
        // }
        // else if(cnt11 ++ < 800) {
        //     box.start.x -= 2;
        //     box.start.y -= 2;
        //     box.end.x -= 2;
        //     box.end.y -= 2;
        //     line.start.x -= 2;
        //     line.start.y -= 2;
        //     line.end.x -= 2;
        //     line.end.y -= 2;
        // } else {
        //     cnt11 = 0;
        // }

        /* 心跳 */
        ipgui_event_loop(sdl_scr);
        ipgui_loop_def(2);
        Sleep(2);
    }


	return 0;
}