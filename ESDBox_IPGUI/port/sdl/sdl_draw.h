#ifndef SDL_DRAW_H
#define SDL_DRAW_H

#include "ipgui_utils.h"
#include "ipgui_screen.h"
#include <SDL.h>
IPGUI_HEADER_BEGIN _______________MARKER_______________
struct sdl_private_t {
    SDL_Window * window;
    SDL_Surface * surface;
    SDL_Renderer * renderer;
    unsigned int * framebuffer;
};

extern ipgui_scr_t ipgui_scr1;
extern ipgui_scr_t * ipgui_sdl_screen_create(void);

IPGUI_HEADER_END   _______________MARKER_______________

#endif
