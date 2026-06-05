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
#include "ipgui_input_dispatcher.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#undef main

extern __IPGUI_API__ ipgui_err_t ipgui_sdl_mouse_event_poll(void * priv_data, ipgui_input_src_evt_t * raw_evt);

ipgui_input_dispatcher_t dispatcher;
ipgui_input_src_t pointer_src;
ipgui_input_src_t keyboard_src;
ipgui_scr_t main_screen;
ipgui_input_src_id_t pointer_id;
ipgui_input_src_id_t keyboard_id;
ipgui_scr_id_t main_scr_id;
int main(void)
{
    ipgui_input_dispatcher_init(&dispatcher);

    pointer_src.priv_data = (void *)0;
    pointer_src.convert_event_cb = (convert_event_cb_t)0;
    pointer_src.input_src_event_read_cb = ipgui_sdl_mouse_event_poll;

    pointer_id  = ipgui_dispatcher_register_input_src(&dispatcher, &pointer_src);
    keyboard_id = ipgui_dispatcher_register_input_src(&dispatcher, &keyboard_src);
    main_scr_id = ipgui_dispatcher_register_screen(&dispatcher, &main_screen);

    ipgui_bind_input_src_with_screen(&dispatcher, pointer_id, main_scr_id);
    // ipgui_bind_input_src_with_screen(&dispatcher, keyboard_id, main_scr_id);

    if(ipgui_init() != IPGUI_ERR_OK) {
        printf("ipgui_init_err"); return 0;
    }

    // sdl_scr = ipgui_sdl_screen_create();

    while(1)
    {
        
        /* 心跳 */
        ipgui_loop_def(2);
        Sleep(2);
    }


	return 0;
}