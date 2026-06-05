#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include "SDL.h"
#include "sdl_draw.h"
#include "sdl_input_event.h"

#include "ipgui_screen.h"
#include "ipgui_widget.h"
#include "ipgui_input_dispatcher.h"

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

    while(1)
    {
        ipgui_dispatch_input_event(&dispatcher);

        

        /* 心跳 */
        ipgui_loop_def(2);
        Sleep(2);
    }


	return 0;
}