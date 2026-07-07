#include "sdl_input_event.h"
#include "ipgui_memory.h"
#include "ipgui_debug.h"
#include "ipgui_input_src_event.h"

int g_sdl_mouse_x = 0, g_sdl_mouse_y = 0;
int g_pressed = 0, g_last_pressed = 0;

extern ipgui_input_src_id_t pointer_id;

/* must be called periodically */
__IPGUI_API__ ipgui_err_t ipgui_sdl_mouse_event_poll(void * priv_data, ipgui_input_src_evt_t * raw_evt)
{
    SDL_Event event;
    
    if (SDL_PollEvent(&event))
    {
        if (event.type == SDL_MOUSEBUTTONDOWN)
        {
            g_pressed = g_last_pressed = 1;
            g_sdl_mouse_x = event.button.x;
            g_sdl_mouse_y = event.button.y;
        }
        else if (event.type == SDL_MOUSEBUTTONUP)
        {
            g_pressed = g_last_pressed = 0;
            g_sdl_mouse_x = event.button.x;
            g_sdl_mouse_y = event.button.y;
        }
        else if (event.type == SDL_MOUSEMOTION)
        {
            g_pressed = g_last_pressed ? 1 : 0;
            g_sdl_mouse_x = event.motion.x;
            g_sdl_mouse_y = event.motion.y;
        }
        else if (event.type == SDL_QUIT)
        {
            SDL_Quit();
            /* 结束程序 */
            exit(0);
        }
        raw_evt->input_src_id = pointer_id;
        raw_evt->input_src_evt = g_pressed ? IPGUI_INPUT_SRC_EVENT_POINTER_PRESS : IPGUI_INPUT_SRC_EVENT_POINTER_RELEASE;
        raw_evt->evt_info.pointer_pos.x = g_sdl_mouse_x;
        raw_evt->evt_info.pointer_pos.y = g_sdl_mouse_y;
    }
}

/* must be called periodically */
__IPGUI_API__ void ipgui_sdl_keyboard_event_poll(void * args)
{
    SDL_Event event;
    
    if( SDL_PollEvent( &event ) )
    {
        if( event.type == SDL_KEYDOWN )
        {

        }
        else if( event.type == SDL_KEYUP )
        {

        }
    }
}