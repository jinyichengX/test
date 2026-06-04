#include "sdl_input_event.h"
#include "ipgui_memory.h"
#include "ipgui_debug.h"

int g_sdl_mouse_x = 0, g_sdl_mouse_y = 0;
int g_pressed = 0, g_last_pressed = 0;

// int sdl_mouse_read(struct ipgui_input_drv_t * dev, ipgui_input_data_t * data)
// {
//     data->data.pid.x = g_sdl_mouse_x;
//     data->data.pid.y = g_sdl_mouse_y;
//     data->data.pid.state = g_pressed ? IPGUI_PRESS_STATE_DOWN : IPGUI_PRESS_STATE_UP;
//     return 0;
// }

/* must be called periodically */
// __IPGUI_API__ void ipgui_sdl_mouse_event_poll(ipgui_input_dev_t * dev, void * args)
// {
//     SDL_Event event;

//     ipgui_input_dev_t * sdl_input_dev = dev;
    
//     if (SDL_PollEvent(&event))
//     {
//         if (event.type == SDL_MOUSEBUTTONDOWN)
//         {
//             g_pressed = g_last_pressed = 1;
//             g_sdl_mouse_x = event.button.x;
//             g_sdl_mouse_y = event.button.y;
//         }
//         else if (event.type == SDL_MOUSEBUTTONUP)
//         {
//             g_pressed = g_last_pressed = 0;
//             g_sdl_mouse_x = event.button.x;
//             g_sdl_mouse_y = event.button.y;
//         }
//         else if (event.type == SDL_MOUSEMOTION)
//         {
//             g_pressed = g_last_pressed ? 1 : 0;
//             g_sdl_mouse_x = event.motion.x;
//             g_sdl_mouse_y = event.motion.y;
//         }
//     }
// }

// /* must be called periodically */
// __IPGUI_API__ void ipgui_sdl_keyboard_event_poll(ipgui_input_dev_t * dev, void * args)
// {
//     SDL_Event event;

//     ipgui_input_dev_t * sdl_input_dev = dev;
    
//     if( SDL_PollEvent( &event ) )
//     {
//         if( event.type == SDL_KEYDOWN )
//         {
//             ipgui_input_event(dev, IPGUI_EVT_TYPE_KEY, IPGUI_EVT_KEY_1, 1);
//             ipgui_input_sync(dev);
//         }
//         else if( event.type == SDL_KEYUP )
//         {
//             ipgui_input_event(dev, IPGUI_EVT_TYPE_KEY, IPGUI_EVT_KEY_1, 0);
//             ipgui_input_sync(dev);
//         }
//     }
// }