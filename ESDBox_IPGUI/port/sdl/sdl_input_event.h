#ifndef SDL_INPUT_EVENT_H
#define SDL_INPUT_EVENT_H

#include "ipgui_utils.h"
#include "ipgui_event.h"
#include <SDL.h>

IPGUI_HEADER_BEGIN _______________MARKER_______________

__IPGUI_API__ ipgui_input_dev_t * ipgui_sdl_mouse_create_init(void);
__IPGUI_API__ ipgui_input_dev_t * ipgui_sdl_create_keyboard_init(void);

IPGUI_HEADER_END   _______________MARKER_______________
#endif
