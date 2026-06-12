// #include <time.h>
// #include <stdio.h>
// #include <stdlib.h>

// #include "SDL.h"
// #include "sdl_draw.h"
// #include "sdl_input_event.h"

// #include "ipgui_screen.h"
// #include "ipgui_widget.h"
// #include "ipgui_input_dispatcher.h"

// #undef main

// extern __IPGUI_API__ ipgui_err_t ipgui_sdl_mouse_event_poll(void * priv_data, ipgui_input_src_evt_t * raw_evt);

// ipgui_input_dispatcher_t dispatcher;
// ipgui_input_src_t pointer_src;
// ipgui_input_src_t keyboard_src;
// ipgui_scr_t main_screen;
// ipgui_input_src_id_t pointer_id;
// ipgui_input_src_id_t keyboard_id;
// ipgui_scr_id_t main_scr_id;

// ipgui_scr_drv_t sdl_drv = {
//     .xreso = 800,
//     .yreso = 480,

//     .pri_data    = &g_sdl_private,
//     .put_pixel   = sdl_put_pixel,
//     .fill_region = sdl_fill_region,
//     // .close       = sdl_exit,
//     .flush       = sdl_flush,
// };

// static u8_t main_screen_frame_buf[800 * 4];

// int main(void)
// {
//     ipgui_input_dispatcher_init(&dispatcher);

//     pointer_src.priv_data = (void *)0;
//     pointer_src.convert_event_cb = (convert_event_cb_t)0;
//     pointer_src.input_src_event_read_cb = ipgui_sdl_mouse_event_poll;

//     ipgui_screen_init(&main_screen, &sdl_drv);
//     ipgui_sdl_screen_init(&main_screen);

//     pointer_id  = ipgui_dispatcher_register_input_src(&dispatcher, &pointer_src);
//     // keyboard_id = ipgui_dispatcher_register_input_src(&dispatcher, &keyboard_src);
//     main_scr_id = ipgui_dispatcher_register_screen(&dispatcher, &main_screen);

//     ipgui_bind_input_src_with_screen(&dispatcher, pointer_id, main_scr_id);
//     // ipgui_bind_input_src_with_screen(&dispatcher, keyboard_id, main_scr_id);

//     ipgui_scr_create_pfb(&main_screen, main_screen_frame_buf, sizeof(main_screen_frame_buf), PIX_FMT_RGBA8888);

//     if(ipgui_init() != IPGUI_ERR_OK) {
//         printf("ipgui_init_err"); return 0;
//     }

//     // ipgui_widget_create();

//     while(1)
//     {
//         ipgui_dispatch_input_event(&dispatcher);

        

//         /* 心跳 */
//         ipgui_loop_def(2);
//         Sleep(2);
//     }


// 	return 0;
// }


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
// #include "ipgui_draw_box_shadow.h"
#include "ipgui_draw_box_border.h"
#include "ipgui_edge_halfplane_mask.h"
#include "ipgui_edge_wdf_mask.h"
#include "ipgui_draw_triangle.h"
#include "ipgui_draw_arc.h"
#include "ipgui_input_dispatcher.h"
#include "ipgui_draw_polygon.h"
#include "ipgui_draw_builtin_font.h"
#include "ipgui_gradient_color.h"
#include "ipgui_image_geometry_transform.h"
#include "ipgui_input_dispatcher.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#undef main

int test_bmp(const s8_t * path, ipgui_img_dsc_t * image);

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
ipgui_scr_drv_t sdl_drv = {
.xreso = 800,
.yreso = 480,

.pri_data    = &g_sdl_private,
.put_pixel   = sdl_put_pixel,
.fill_region = sdl_fill_region,
// .close       = sdl_exit,
.flush       = sdl_flush,
};

ipgui_scr_t * sdl_scr;
ipgui_color_t g_color;
ipgui_surf_t surf;
ipgui_aabb_t clip;
int cnt11 = 0;
ipgui_input_src_id_t pointer_id;
#define EGUI_ALPHA_100 255
#define EGUI_MASK_CIRCLE_AA_HALF_256  192
static u8_t egui_mask_circle_edge_smoothstep(int32_t signed_dist_256)
{
    int32_t coverage;
    int32_t t;
    int32_t t_sq;
    int32_t smooth;
    int32_t alpha_range_sq = EGUI_ALPHA_100 * EGUI_ALPHA_100;

    if (signed_dist_256 <= -EGUI_MASK_CIRCLE_AA_HALF_256)
    {
        return EGUI_ALPHA_100;
    }

    if (signed_dist_256 >= EGUI_MASK_CIRCLE_AA_HALF_256)
    {
        return 0;
    }

    coverage = EGUI_MASK_CIRCLE_AA_HALF_256 - signed_dist_256;
    t = (coverage * EGUI_ALPHA_100 + EGUI_MASK_CIRCLE_AA_HALF_256) / (EGUI_MASK_CIRCLE_AA_HALF_256 << 1);
    t_sq = t * t;
    smooth = (3 * t_sq * EGUI_ALPHA_100 - 2 * t_sq * t + (alpha_range_sq >> 1)) / alpha_range_sq;

    if (smooth <= 0)
    {
        return 0;
    }

    if (smooth >= EGUI_ALPHA_100)
    {
        return EGUI_ALPHA_100;
    }

    return (u8_t)smooth;
}

#define RENDER_MODE 3
 #define RENDER_MODE 1 /* 鍗曡娓叉煋 */
 #define RENDER_MODE 2 /* 鍗曡娓叉煋 */
 #define RENDER_MODE 3 /* 鍏ㄥ睆娓叉煋 */
int main(void)
{
    ipgui_input_dispatcher_t dispatcher;
    ipgui_input_dispatcher_init(&dispatcher);

    ipgui_input_src_t touch_src;
    ipgui_input_src_t key_src;
    ipgui_scr_t sdl_scr;
    
    // 2. 娉ㄥ唽杈撳叆婧愬拰灞忓箷
    s32_t touch_id = ipgui_dispatcher_register_input_src(&dispatcher, &touch_src);
    s32_t key_id = ipgui_dispatcher_register_input_src(&dispatcher, &key_src);
    s32_t main_scr_id = ipgui_dispatcher_register_screen(&dispatcher, &sdl_scr);

    // 3. 缁戝畾鏄犲皠
    ipgui_bind_input_src_with_screen(&dispatcher, touch_id, main_scr_id);
    ipgui_bind_input_src_with_screen(&dispatcher, key_id, main_scr_id);

    IPGUI_COLOR_SET(g_color, 255, IPGUI_COLOR_RED);
    if(ipgui_init() != IPGUI_ERR_OK)
    {
        printf("ipgui_init_err");
        return 0;
    }

    /* GUI */
    ipgui_screen_init(&sdl_scr, &sdl_drv);
    ipgui_sdl_screen_init(&sdl_scr);

    surf.color = ((struct sdl_private_t *)sdl_scr.drv->pri_data)->framebuffer;
    surf.surf.start.x = 0;
    surf.surf.start.y = 0;
    surf.surf.end.x = sdl_scr.drv->xreso - 1;
    surf.surf.end.y = sdl_scr.drv->yreso - 1;
    surf.pix_fmt = PIX_FMT_RGBA8888;
    surf.pix_size = 4;
    // surf.pix_fmt = PIX_FMT_RGB888;
    // surf.pix_size = 3;
    surf.stride = sdl_scr.drv->xreso * surf.pix_size;

    clip.start.x = 0;
    clip.start.y = 0;
    clip.end.x = 0;
    clip.end.y = 0;


    // ipgui_color_t color[sdl_scr->drv->yreso * sdl_scr->drv->xreso];
    ipgui_line_t line;
    line.start.x = 100;
    line.start.y = 479;
    line.end.x = 500;
    line.end.y = 450;
    ipgui_line_style_t line_style;
    line_style.cap = IPGUI_LINE_CAP_BUTT;
    line_style.opacity = 255;
    line_style.blend_mode = 0;
    line_style.width = 10;
    line_style.paint.type = IPGUI_PAINT_GRADIENT;

    line_style.paint.src.color = g_color;
    line_style.paint.src.grad_src.grad_type = IPGUI_GRADIENT_TYPE_LINEAR;
    ipgui_liner_gradient_init_direct(&line_style.paint.src.grad_src.grad.liner_grad, 
    line.start.x, line.start.y, line.end.x, line.end.y);
    ipgui_gradient_color_stop_t stop00;
    stop00.pos = 0;
    IPGUI_COLOR_SET(stop00.color, 255, IPGUI_COLOR_BLUE);
    ipgui_gradient_color_stop_t stop01;
    stop01.pos = 255;
    IPGUI_COLOR_SET(stop01.color, 100, IPGUI_COLOR_RED);
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
    img_data.fmt = IPGUI_IMG_FMT_BGR888;//鍙互鏀规垚L8鎴栬€匧A88鎴栬€匯GB565璇曡瘯锛岃櫧鐒惰繖涔堟敼閫昏緫涓婁笉瀵癸紝浣嗘槸鏈夋晥鏋?
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
    img2_data.fmt = IPGUI_IMG_FMT_BGR888;//鍙互鏀规垚L8鎴栬€匧A88鎴栬€匯GB565璇曡瘯锛岃櫧鐒惰繖涔堟敼閫昏緫涓婁笉瀵癸紝浣嗘槸鏈夋晥鏋?
    img2_data.stride = img2_dsc.stride;
    img2_data.w = img2_dsc.w;
    img2_data.h = img2_dsc.h;

    ipgui_img_dsc_t img3_dsc;
    if(0 != test_bmp("M:/test/ESDBox_IPGUI/core/image/decoder/material/bmp/lena_c.bmp", &img3_dsc))
    {
        printf("Failed to load bitmap image, please run the program at the main.c page\n");
        return -1;
    }
    ipgui_image_data_t img3_data;
    img3_data.pixmap = img3_dsc.pixmap;
    img3_data.px_size = img3_dsc.stride / img3_dsc.w;
    img3_data.fmt = IPGUI_IMG_FMT_BGR888;//鍙互鏀规垚L8鎴栬€匧A88鎴栬€匯GB565璇曡瘯锛岃櫧鐒惰繖涔堟敼閫昏緫涓婁笉瀵癸紝浣嗘槸鏈夋晥鏋?
    img3_data.stride = img3_dsc.stride;
    img3_data.w = img3_dsc.w;
    img3_data.h = img3_dsc.h;

    ipgui_img_dsc_t img4_dsc;
    if(0 != test_bmp("M:/test/ESDBox_IPGUI/core/image/decoder/material/bmp/kbm.bmp", &img4_dsc))
    {
        printf("Failed to load bitmap image, please run the program at the main.c page\n");
        return -1;
    }
    ipgui_image_data_t img4_data;
    img4_data.pixmap = img4_dsc.pixmap;
    img4_data.px_size = img4_dsc.stride / img4_dsc.w;
    img4_data.fmt = IPGUI_IMG_FMT_BGR888;//鍙互鏀规垚L8鎴栬€匧A88鎴栬€匯GB565璇曡瘯锛岃櫧鐒惰繖涔堟敼閫昏緫涓婁笉瀵癸紝浣嗘槸鏈夋晥鏋?
    img4_data.stride = img4_dsc.stride;
    img4_data.w = img4_dsc.w;
    img4_data.h = img4_dsc.h;

    ipgui_point_t pivot;
    pivot.x = img_dsc.w / 2;
    pivot.y = img_dsc.h / 2;
    ipgui_point_t anchor;
    anchor.x = sdl_scr.drv->xreso / 2 + 200;
    anchor.y = sdl_scr.drv->yreso / 2;

    ipgui_image_draw_style_t img_style;
    img_style.blend_mode = 0;
    img_style.opacity = 255;
    ipgui_image_draw_style_t img2_style;
    img2_style.blend_mode = 0;
    img2_style.opacity = 255;
    ipgui_image_draw_style_t img4_style;
    img4_style.blend_mode = 0;
    img4_style.opacity = 127;
    ipgui_box_bg_style_t box_bg_style;
    box_bg_style.blend_mode = 0;
    box_bg_style.opacity = 255;
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



    // ipgui_image_data_t img_data;
    // img_data.pixmap = img_dsc.pixmap;
    // img_data.px_size = img_dsc.stride / img_dsc.w;
    // img_data.fmt = IPGUI_IMG_FMT_BGR888;//鍙互鏀规垚L8鎴栬€匧A88鎴栬€匯GB565璇曡瘯锛岃櫧鐒惰繖涔堟敼閫昏緫涓婁笉瀵癸紝浣嗘槸鏈夋晥鏋?
    // img_data.stride = img_dsc.stride;
    // img_data.w = img_dsc.w;
    // img_data.h = img_dsc.h;

    ipgui_aabb_t box1;
    box1.start.x = 100;
    box1.start.y = 100;
    box1.end.x = 199;
    box1.end.y = 199;
    
    ipgui_box_style_t box_style1;
    box_style1.bottom_padding = 0;
    box_style1.left_padding = 0;
    box_style1.top_padding = 0;
    box_style1.right_padding = 0;

    box_style1.left_bottom_radius = 50;
    box_style1.left_top_radius = 50;
    box_style1.right_bottom_radius = 50;
    box_style1.right_top_radius = 50;

    ipgui_image_src_t img_src;
    img_src.buf = img3_data.pixmap;
    img_src.img_pxfmt = IPGUI_IMG_FMT_BGR888;
    img_src.px_size = img3_data.px_size;
    img_src.stride = img3_data.stride;

    ipgui_aabb_t img_src_img_aabb;
    img_src_img_aabb.start.x = box1.start.x - 50;
    img_src_img_aabb.start.y = box1.start.y;
    img_src_img_aabb.end.x = img_src_img_aabb.start.x + img3_data.w - 1;
    img_src_img_aabb.end.y = img_src_img_aabb.start.y + img3_data.h - 1;
    img_src.img_aabb = &img_src_img_aabb;

    ipgui_box_bg_style_t box_bg_style1;
    box_bg_style1.blend_mode = 0;
    box_bg_style1.opacity = 250;
    box_bg_style1.paint.type = IPGUI_PAINT_IMAGE;
    box_bg_style1.paint.src.image_src = img_src;
    ipgui_box_border_style_t box_border_style1;
    box_border_style1.blend_mode = 0;
    box_border_style1.width = 2;
    box_border_style1.opacity = 200;
    box_border_style1.paint.type = IPGUI_PAINT_COLOR;
    IPGUI_COLOR_SET(box_border_style1.paint.src.color, 255, IPGUI_COLOR_220);

    static ipgui_point_t tri_p1;
    tri_p1.x = 220;
    tri_p1.y = 50;
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
    IPGUI_COLOR_SET(stop11.color, 255, IPGUI_COLOR_GREEN);
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
    ipgui_conic_gradient_init(&arc_style.paint.src.grad_src.grad.conic_grad, arc.cx, arc.cy, 30);
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

    static u32_t degree = 20;
    static u32_t degree1 = 0;
    float sx = 0.5;
    float sy = 0.5;
    
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
                for (int i = 0; i < sdl_scr.drv->xreso; i ++) {
                    IPGUI_COLOR_SET(row0[i], 255, IPGUI_COLOR_WHITE);
                }
                for (int y = 1; y < sdl_scr.drv->yreso; y ++) {
                    memcpy(offline_buffer + y * surf1.stride, offline_buffer, surf1.stride);
                }
                surf1.surf.start.x = 0;
                surf1.surf.start.y = 0;
                surf1.surf.end.x   = sdl_scr.drv->xreso - 1;
                surf1.surf.end.y   = sdl_scr.drv->yreso- 1;
#endif
                /* 鐢诲浘寮€濮?*/

                ipgui_draw_image(
                    &surf1,
                    NULL,
                    &img_data,
                    &pivot,
                    &anchor,
                    NULL,
                    &img_style
                );

                ipgui_draw_pixel(
                    &surf1, 
                    NULL,
                    anchor.x, anchor.y,
                    g_color,
                    255,
                    255,
                    0);

                ipgui_img_geo_trans_t trans;
                ipgui_point_t pivot1;
                pivot1.x = 190;
                pivot1.y = 190;
                ipgui_image_trans_init(&trans);
                ipgui_image_trans_pivot(&trans, pivot1);
                // ipgui_image_trans_scale(&trans, 2, 1.2);
                ipgui_image_trans_rotate_degree(&trans, degree);
                ipgui_point_t anchor1;
                anchor1.x = 200;
                anchor1.y = 200;
                ipgui_draw_image( 
                    &surf1,       //杩欎釜鍑芥暟甯﹀彉鎹㈠弬鏁版椂鍙敮鎸丷GB888鏍煎紡鐨勫浘鍍忥紝闇€瑕佷紭鍖?
                    NULL,
                    &img2_data,
                    &trans.pivot,
                    &anchor1,
                    (ipgui_trans_mat_t *)&trans.mat,
                    &img2_style
                );



                ipgui_draw_arc(
                    &surf1, 
                    NULL,
                    &arc, 
                    &arc_style);

                ipgui_draw_triangle(
                    &surf1,
                    NULL,
                    &tri_p1, &tri_p2, &tri_p3,
                    &tri_style);

                // ipgui_draw_box_shadow(&surf1, 
                //     NULL, 
                //     &box, 
                //     &shadow_style);

                ipgui_draw_box_background(
                    &surf1,
                    NULL,
                    &box,
                    &box_style,
                    &box_bg_style);

                ipgui_draw_box_border(
                    &surf1,
                    NULL,
                    &box,
                    &box_style,
                    &box_border_style);

                ipgui_draw_builtin_text(
                    &surf1,
                    NULL,
                    &font_style,
                    "hello kitty@@@@@ a lazy dog%%5 ^&*())__+;'",
                    50,
                    50);

                ipgui_draw_line_generic(
                    &surf1,
                    NULL,
                    &line,
                    &line_style);

                ipgui_draw_box_background(
                    &surf1,
                    NULL,
                    &box1,
                    &box_style1,
                    &box_bg_style1);
                
                ipgui_draw_box_border(
                    &surf1,
                    NULL,
                    &box1,
                    &box_style1,
                    &box_border_style1);


                ipgui_img_geo_trans_t trans1;

                ipgui_point_t pivot2;
                pivot2.x = img4_data.w / 2;
                pivot2.y = img4_data.h / 2;
                ipgui_image_trans_init(&trans1);
                ipgui_image_trans_pivot(&trans1, pivot2);
                ipgui_image_trans_scale(&trans1, sx, sy);
                ipgui_image_trans_rotate_degree(&trans1, degree1);
                ipgui_point_t anchor2;
                anchor2.x = 400;
                anchor2.y = 240;
                ipgui_draw_image(//杩欎釜鍑芥暟甯﹀彉鎹㈠弬鏁版椂鍙敮鎸丷GB888鏍煎紡鐨勫浘鍍忥紝闇€瑕佷紭鍖?
                    &surf1,
                    NULL,
                    &img4_data,
                    &trans1.pivot,
                    &anchor2,
                    (ipgui_trans_mat_t *)&trans1.mat,
                    &img4_style
                );

                /* 鐢诲浘缁撴潫 */
#if RENDER_MODE == 1
                ipgui_screen_fill_region(sdl_scr, 
                x, y, x, y, 
                surf1.color, surf1.stride);
#elif RENDER_MODE == 2
                ipgui_screen_fill_region(sdl_scr, 
                0, y, sdl_scr->drv->xreso - 1, y, 
                surf1.color, surf1.stride);
#elif RENDER_MODE == 3
                ipgui_screen_fill_region(&sdl_scr, 
                    0, 0, sdl_scr.drv->xreso - 1, sdl_scr.drv->yreso - 1, 
                    surf1.color, surf1.stride);
#endif

#if RENDER_MODE == 1
            }
        }
#elif RENDER_MODE == 2
        }
#elif RENDER_MODE == 3
        
#endif
        sdl_scr.drv->flush(&sdl_scr);
        arc.start += 1;
        degree += 2;
        degree1 -= 1;
        line.start.y --;
        arc_style.paint.src.grad_src.grad.conic_grad.angle_start ++;
        /* 鏀瑰彉浣嶇疆 */
        if(cnt11 ++ < 400) {
            sx += 0.007;
                        sy += 0.007;
            if(img4_style.opacity < 255)
                img4_style.opacity ++;

            // box.start.x += 1;
            // box.start.y += 1;
            // box.end.x += 1;
            // box.end.y += 1;

            // line.start.x += 1;
            // line.start.y += 1;
            // line.end.x += 1;
            // line.end.y += 1;
            // clip.end.x ++;
            // clip.end.y ++;

            
            // box_style.left_bottom_radius ++;
            // box_style.left_top_radius ++;
            // box_style.right_bottom_radius ++;
            // box_style.right_top_radius ++;
        }
        else if(cnt11 ++ < 800) {
            if(img4_style.opacity > 50)
                img4_style.opacity -= 2;
            sx -= 0.014;
            sy -= 0.014;
            // box.start.x -= 2;
            // box.start.y -= 2;
            // box.end.x -= 2;
            // box.end.y -= 2;
            // line.start.x -= 2;
            // line.start.y -= 2;
            // line.end.x -= 2;
            // line.end.y -= 2;
        } else {
            cnt11 = 0;
        }

        /* 蹇冭烦 */
        ipgui_loop_def(2);
        Sleep(2);
    }


	return 0;
}
