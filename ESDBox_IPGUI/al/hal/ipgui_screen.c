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

#include "ipgui_screen.h"
#include "ipgui_memory.h"
#include "ipgui_handler.h"
#include "ipgui_debug.h"
#include "ipgui_prim.h"
#include "ipgui_timer.h"

LIST_HEAD(ipgui_scr_list);


__IPGUI_API__ ipgui_err_t ipgui_screen_register_input_device(ipgui_scr_t * scr, ipgui_input_dev_t * dev)
{
    dev->scr_owner = (void *)scr;
    return ipgui_input_register_device(dev, &scr->inputs, &scr->handler);
}

__IPGUI_API__ ipgui_err_t ipgui_screen_register_input_handler(ipgui_scr_t * scr, ipgui_input_handler_t * handler)
{
    return ipgui_input_register_handler(handler, &scr->inputs, &scr->handler);
}

__IPGUI_API__ int ipgui_screen_register(ipgui_scr_drv_t * drv)
{
    ipgui_scr_t * scr = ipgui_mem_alloc(ipgui_smem, sizeof(ipgui_scr_t));
    if (!scr)
        return (ipgui_scr_t *)0;
    ipgui_memset((void *)scr, 0, sizeof(ipgui_scr_t));
    scr->drv = drv;
    ipgui_darray_init(&scr->dirty_regions, sizeof(ipgui_aabb_t));
    list_head_init(&scr->input_dev);

    ipgui_input_handler_t * def_handler;
    def_handler = (ipgui_input_handler_t *)ipgui_mem_alloc(ipgui_smem, sizeof(ipgui_input_handler_t));
    if( def_handler == (ipgui_input_handler_t *)0 )
        return IPGUI_ERR_NOMEM;
    list_head_init(&def_handler->node);
    list_head_init(&def_handler->handle_list);
    def_handler->id_table = &event_dispatcher_id_table;
    def_handler->events = ipgui_input_event_handler;
    def_handler->filter = ipgui_input_event_filter;
    def_handler->match = NULL;
    def_handler->event = NULL;
    def_handler->priv_data = (void *)0;

    list_head_init(&scr->inputs);
    list_head_init(&scr->handler);
    ipgui_screen_register_input_handler(scr, def_handler);
    ipgui_memset((void *)&scr->post, 0, sizeof(ipgui_post_event_t));

    list_add_tail(&scr->node, &ipgui_scr_list);

    return scr;
}

__IPGUI_API__ void ipgui_screen_putpixel(ipgui_scr_t * scr, ipgui_coord_t x, ipgui_coord_t y, unsigned char * pix)
{

    if (scr && scr->drv && scr->drv->put_pixel) {
        scr->drv->put_pixel(scr, x, y, pix);
    }
}

__IPGUI_API__ void ipgui_screen_flush(ipgui_scr_t * scr)
{
    if (scr && scr->drv && scr->drv->flush) {
        scr->drv->flush(scr);
    }
}

__IPGUI_API__ void ipgui_screen_fill_region(ipgui_scr_t * scr, 
        ipgui_coord_t x1, ipgui_coord_t y1, ipgui_coord_t x2, ipgui_coord_t y2, 
        unsigned char * pix_buf, int stride)
{
    if (scr && scr->drv && scr->drv->fill_region) {
        scr->drv->fill_region(scr, x1, y1, x2, y2, pix_buf, stride);
    }
}