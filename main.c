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

static u8_t main_screen_frame_buf[800 * 4];

void draw_main_screen_backgroud_color(ipgui_scr_t * scr, ipgui_surf_t * surf)
{
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
extern void widget_switch_render(struct ipgui_widget * widget, ipgui_widget_render_ctx_t * ctx);
extern void widget_switch_label_render(struct ipgui_widget * widget, ipgui_widget_render_ctx_t * ctx);
extern void widget_arc_label_render(struct ipgui_widget * widget, ipgui_widget_render_ctx_t * ctx);
extern void power_render(struct ipgui_widget * widget, ipgui_widget_render_ctx_t * ctx);
extern void tablelamp_render(struct ipgui_widget * widget, ipgui_widget_render_ctx_t * ctx);

ipgui_image_data_t wave_img;
ipgui_image_data_t main_bg_img;
ipgui_image_data_t power_img;
ipgui_image_data_t tablelamp_img;
int main(void)
{
    ipgui_input_dispatcher_init(&dispatcher);

    pointer_src.priv_data = (void *)0;
    pointer_src.convert_event_cb = (convert_event_cb_t)0;
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
    widget4->render = widget4_render;
    widget4->x = 0;
    widget4->y = 0;
    widget4->w = 1600;
    widget4->h = 480;

    /* create widget */
    ipgui_widget_t * widget1 = ipgui_widget_create(NULL);
    widget1->render = widget1_render;
    widget1->x = 0;
    widget1->y = 0;
    widget1->w = 800;
    widget1->h = 479;

    ipgui_widget_t * widget2 = ipgui_widget_create(widget1);
    widget2->render = widget2_render;
    widget2->x = 0;
    widget2->y = 0;
    widget2->w = 800;
    widget2->h = 479;

    ipgui_widget_t * widget3 = ipgui_widget_create(NULL);
    widget3->render = widget3_render;
    widget3->x = 0;
    widget3->y = 0;
    widget3->w = 1599;
    widget3->h = 959;

    ipgui_img_dsc_t img_dsc;
    if(0 != test_bmp("M:/test/ESDBox_IPGUI/widget/src/wave.bmp", &img_dsc))
    {
        printf("Failed to load bitmap image, please run the program at the main.c page\n");
        return -1;
    }


    ipgui_img_dsc_t img_bg_dsc;
    if(0 != test_bmp("M:/test/ESDBox_IPGUI/widget/src/bg_no_dash.bmp", &img_bg_dsc))
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

    ipgui_widget_t * widget_switch = ipgui_widget_create(NULL);
    widget_switch->render = widget_switch_render;
    widget_switch->x = 365;
    widget_switch->y = 400;
    widget_switch->w = 90;
    widget_switch->h = 50;

    ipgui_widget_t * widget_switch_label = ipgui_widget_create(NULL);
    widget_switch_label->render = widget_switch_label_render;
    widget_switch_label->x = 362;
    widget_switch_label->y = 376;
    widget_switch_label->w = 110;
    widget_switch_label->h = 50;

        ipgui_widget_t * widget_arc_label = ipgui_widget_create(NULL);
    widget_arc_label->render = widget_arc_label_render;
    widget_arc_label->x = 55;
    widget_arc_label->y = 260;
    widget_arc_label->w = 120;
    widget_arc_label->h = 80;

        ipgui_widget_t * widget_power = ipgui_widget_create(NULL);
    widget_power->render = power_render;
    widget_power->x = 370;
    widget_power->y = 50;
    widget_power->w = 100;
    widget_power->h = 100;

    ipgui_widget_t * widget_tablelamp = ipgui_widget_create(NULL);
    widget_tablelamp->render = tablelamp_render;
    widget_tablelamp->x = 630;
    widget_tablelamp->y = 50;
    widget_tablelamp->w = 100;
    widget_tablelamp->h = 100;

    ipgui_input_src_evt_t raw_evt;
    int step = 0;
    while(1)
    {

        /* 读取原始事件 */
        pointer_src.input_src_event_read_cb(&pointer_src, &raw_evt);
        /* push into queue */
        ipgui_norm_queue_post(&dispatcher.evt_queue, &raw_evt, sizeof(raw_evt));

        printf("raw_evt: input_src_id=%d, input_src_evt=%d, pointer_pos.x=%d, y=%d\n", 
            raw_evt.input_src_id, 
            raw_evt.input_src_evt, 
            raw_evt.evt_info.pointer_pos.x, 
            raw_evt.evt_info.pointer_pos.y);

        // ipgui_dispatch_input_event(&dispatcher);

        ipgui_screen_render(&main_screen);

        /* 心跳 */
        ipgui_loop_def(2);
        Sleep(10);

    }


	return 0;
}



