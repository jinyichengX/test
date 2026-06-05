#ifndef IPGUI_SCREEN_H
#define IPGUI_SCREEN_H

#include "ipgui_utils.h"
#include "ipgui_coord.h"
#include "ipgui_prim.h"
#include "ipgui_lcd_pix_fmt.h"
#include "ipgui_widget_evt.h"
#include "ipgui_widget_tree.h"
#include "ipgui_dirty_rect.h"

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

typedef struct ipgui_scr_ctx {
    void *              pri_data;
    const char *        name;
    ipgui_scr_drv_t *   drv;

    struct widget_tree_t root;
    /*
    +------------+ 
    | widget_tree_t（root link）|
    | (parent=NULL)| 
    +------------+  
            |                                    
            | first_child                           
            ↓                                       
        +------------+     +------------+       
        | link A     |<--->| link  C    |     
        +------------+     +------------+    
            |                   |
            | first_child       | first_child
            ↓                   ↓
        +------------+     +------------+
        | link  B    |     | linkD      |
        +------------+     +------------+

    */
    ipgui_dirty_rect_mgr_t dirty;
}ipgui_scr_t;

extern __IPGUI_API__ void ipgui_screen_putpixel(ipgui_scr_t * scr, ipgui_coord_t x, ipgui_coord_t y, unsigned char * pix);
extern __IPGUI_API__ void ipgui_screen_fill_region(ipgui_scr_t * scr, 
        ipgui_coord_t x1, ipgui_coord_t y1, ipgui_coord_t x2, ipgui_coord_t y2, 
        unsigned char * pix_buf, int stride);
extern __IPGUI_API__ void ipgui_screen_flush(ipgui_scr_t * scr);
extern __IPGUI_API__ void ipgui_screen_handle_widget_event(ipgui_scr_t * scr, ipgui_widget_evt_t * evt);

#endif
