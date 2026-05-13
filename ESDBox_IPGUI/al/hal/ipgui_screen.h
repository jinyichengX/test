#ifndef IPGUI_SCREEN_H
#define IPGUI_SCREEN_H

#include "ipgui_utils.h"
#include "ipgui_color.h"
#include "ipgui_coord.h"
#include "ipgui_defs.h"
#include "ipgui_event.h"
#include "ipgui_darray.h"
#include "ipgui_timer.h"
#include "ipgui_prim.h"
#include "ipgui_lcd_pix_fmt.h"
IPGUI_HEADER_BEGIN _______________MARKER_______________

typedef struct ipgui_scr_ctx ipgui_scr_t;

typedef struct{
    void *              pri_data;
    ipgui_coord_t       xreso;
    ipgui_coord_t       yreso;
    ipgui_pix_fmt_t     pix_fmt;

    void (* put_pixel)(ipgui_scr_t * scr, ipgui_coord_t x, ipgui_coord_t y, unsigned char * pix);
    void (* fill_region)(ipgui_scr_t * scr, 
        ipgui_coord_t x1, ipgui_coord_t y1, ipgui_coord_t x2, ipgui_coord_t y2, 
        unsigned char * pix_buf, int stride);

    void (* flush)(ipgui_scr_t * scr);
}ipgui_scr_drv_t;

typedef struct ipgui_widget_ctx ipgui_widget_t;
typedef struct ipgui_scr_ctx {
    void *              pri_data;
    const char *        name;
    ipgui_node_t        node;
    ipgui_scr_drv_t *   drv;

    ipg_tmr_t *         refresh; /* input device read timer */

    /* 下面这些参数全删了，没用！ */
    ipgui_list_t        inputs;
    ipgui_list_t        handler;
    ipgui_post_event_t  post;

    ipgui_list_t        input_dev;

    ipgui_widget_t **   screens;             /* all screens */
    unsigned char       screens_cnt;         /* screen count */
    ipgui_widget_t *    cur_screen;          /* current active screen */

    ipgui_darray_t      dirty_regions;       /* dirty region manager */

    ipgui_color_t       bg_color;            /* background color */
    unsigned char       bg_color_alpha; /* background color alpha */

    void *              render_buf;
    unsigned int        render_buf_size;           /* size of render buffer */
}ipgui_scr_t;


extern __IPGUI_API__ ipgui_err_t ipgui_screen_register_input_device(ipgui_scr_t * scr, ipgui_input_dev_t * dev);

extern __IPGUI_API__ ipgui_err_t ipgui_screen_register_input_handler(ipgui_scr_t * scr, ipgui_input_handler_t * handler);
extern __IPGUI_API__ void ipgui_screen_putpixel(ipgui_scr_t * scr, ipgui_coord_t x, ipgui_coord_t y, unsigned char * pix);
extern __IPGUI_API__ void ipgui_screen_fill_region(ipgui_scr_t * scr, 
        ipgui_coord_t x1, ipgui_coord_t y1, ipgui_coord_t x2, ipgui_coord_t y2, 
        unsigned char * pix_buf, int stride);
extern __IPGUI_API__ void ipgui_screen_flush(ipgui_scr_t * scr);
IPGUI_HEADER_END   _______________MARKER_______________
#endif
