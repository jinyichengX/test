/*
 * MIT License
 *
 * Copyright (c) 2025 JinYiCheng
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "sdl_draw.h"
#include "ipgui_color.h"


struct sdl_private_t g_sdl_private;

int ipgui_sdl_screen_init(ipgui_scr_t * scr)
{
    struct sdl_private_t * priv = (struct sdl_private_t *)scr->drv->pri_data;
    // int buffer_size = scr->drv->xreso * scr->drv->yreso * 3;//pix_size = 3
    int buffer_size = scr->drv->xreso * scr->drv->yreso * 4;//pix_size = 4
    priv->framebuffer = (unsigned int *)malloc(buffer_size);
    if(priv->framebuffer == NULL)
        return -1;

    memset(priv->framebuffer, 0, buffer_size);

    SDL_Init(SDL_INIT_VIDEO); 
    SDL_CreateWindowAndRenderer( scr->drv->xreso, scr->drv->yreso, 0, &priv->window, &priv->renderer);
    SDL_SetWindowPosition(priv->window, 100, 800);
    SDL_SetRenderDrawColor(priv->renderer, 0, 0, 0, SDL_ALPHA_OPAQUE); 
    SDL_RenderClear(priv->renderer); 
    priv->surface = SDL_GetWindowSurface(priv->window);
    if (!priv->surface)
        return -2;

    return 0;
}

void sdl_flush(ipgui_scr_t * scr)
{
    struct sdl_private_t * priv = (struct sdl_private_t *)scr->drv->pri_data;
    SDL_RenderPresent(priv->renderer);
}


void sdl_put_pixel(ipgui_scr_t * scr, ipgui_coord_t x, ipgui_coord_t y, unsigned char * pix)
{
    ipgui_color_t color;
    color.v = *((unsigned int *)pix);
    struct sdl_private_t * priv = (struct sdl_private_t *)scr->drv->pri_data;

    SDL_SetRenderDrawColor(priv->renderer, color.r, color.g, color.b, color.a);//pix_size = 4
    // SDL_SetRenderDrawColor(priv->renderer, pix[0], pix[1], pix[2], 255);//pix_size = 3
    SDL_RenderDrawPoint(priv->renderer, x, y);
    if( (x < scr->drv->xreso && x >= 0) && (y < scr->drv->yreso && y >= 0) )
    {
        priv->framebuffer[y * scr->drv->xreso + x] = color.v;
        // *((unsigned char *)priv->framebuffer + y * scr->drv->xreso  * 3 + x * 3) = pix[0];
        // *((unsigned char *)priv->framebuffer + y * scr->drv->xreso  * 3 + x * 3 + 1) = pix[1];
        // *((unsigned char *)priv->framebuffer + y * scr->drv->xreso  * 3 + x * 3 + 2) = pix[2];
    }
}

void sdl_fill_region(ipgui_scr_t * scr, 
        ipgui_coord_t x1, ipgui_coord_t y1, ipgui_coord_t x2, ipgui_coord_t y2, 
        unsigned char * pix_buf, int stride)
{
    unsigned char * row_pix;
    for (int y = y1; y <= y2; y ++) {
        row_pix = pix_buf;
        for (int x = x1; x <= x2; x ++){
            sdl_put_pixel(scr, x, y, row_pix);
            row_pix += 4;/* 4是pix_size，用户指定这个值 */
            // row_pix += 3;/* 3是pix_size，用户指定这个值 */
        }
        pix_buf += stride;
    }
}

// void sdl_exit(ipgui_scr_t * scr)
// {
//     struct sdl_private_t * priv = (struct sdl_private_t *)scr->drv->pri_data;

//     SDL_FreeSurface(priv->surface);
//     SDL_DestroyWindow(priv->window);
//     SDL_Quit();
// }