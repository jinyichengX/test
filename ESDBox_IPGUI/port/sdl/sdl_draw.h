#ifndef SDL_DRAW_H
#define SDL_DRAW_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ipgui_utils.h"
#include "ipgui_screen.h"
#include <SDL.h>

struct sdl_private_t {
    SDL_Window * window;
    SDL_Surface * surface;
    SDL_Renderer * renderer;
    SDL_Texture * texture;
    unsigned int * framebuffer;
};

extern struct sdl_private_t g_sdl_private;
extern void sdl_put_pixel(ipgui_scr_t * scr, ipgui_coord_t x, ipgui_coord_t y, unsigned char * pix);
extern void sdl_fill_region(ipgui_scr_t * scr, 
        ipgui_coord_t x1, ipgui_coord_t y1, ipgui_coord_t x2, ipgui_coord_t y2, 
        unsigned char * pix_buf, int stride);
extern void sdl_flush(ipgui_scr_t * scr);
int ipgui_sdl_screen_init(ipgui_scr_t * scr);

#ifdef __cplusplus
}
#endif

#endif
