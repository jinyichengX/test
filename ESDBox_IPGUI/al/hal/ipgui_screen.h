#ifndef IPGUI_SCREEN_H
#define IPGUI_SCREEN_H

#include "ipgui_utils.h"
#include "ipgui_coord.h"
#include "ipgui_prim.h"
#include "ipgui_lcd_pix_fmt.h"
#include "ipgui_widget_evt.h"
#include "ipgui_widget_tree.h"
#include "ipgui_dirty_rect.h"
#include "ipgui_core.h"

typedef struct ipgui_scr_ctx ipgui_scr_t;
typedef struct{
    void                  * pri_data;
    ipgui_coord_t           xreso;
    ipgui_coord_t           yreso;
    ipgui_pix_fmt_t         pix_fmt;

    void (* put_pixel)  (
        ipgui_scr_t       * scr,
        ipgui_coord_t       x,
        ipgui_coord_t       y,
        u8_t              * pix);

    void (* fill_region)(
        ipgui_scr_t      * scr, 
        ipgui_coord_t      x1,
        ipgui_coord_t      y1,
        ipgui_coord_t      x2,
        ipgui_coord_t      y2, 
        u8_t             * pix_buf,
        s32_t              stride);

    void (* flush)      (
        ipgui_scr_t      * scr);

}ipgui_scr_drv_t;

typedef struct ipgui_scr_ctx{
    void                 * pri_data;

    ipgui_scr_drv_t      * drv;

    /* Widget tree root */
    struct widget_tree_t   tree;

    /* Dirty rect manager */
    ipgui_dirty_rect_mgr_t dirty;

    /* offline partial framebuffer */
    ipgui_pfb_t            pfb;

    /* Background color or render callback 
     * if render callback is NULL, then use bg_color
     */
    ipgui_color_t          bg_color;

    void (* render_bg)(ipgui_scr_t * scr);
}ipgui_scr_t;

extern __IPGUI_API__ ipgui_err_t ipgui_screen_init(
                                        ipgui_scr_t        * scr, 
                                        ipgui_scr_drv_t    * drv);

extern __IPGUI_API__ ipgui_err_t ipgui_scr_create_pfb(
                                        ipgui_scr_t        * scr, 
                                        u8_t               * buf, 
                                        u32_t                buf_size, 
                                        ipgui_pix_fmt_t      pix_fmt);   

extern __IPGUI_API__ void        ipgui_screen_putpixel(
                                        ipgui_scr_t        * scr, 
                                        ipgui_coord_t        x, 
                                        ipgui_coord_t        y, 
                                        u8_t               * pix);

extern __IPGUI_API__ void        ipgui_screen_fill_region(
                                        ipgui_scr_t        * scr, 
                                        ipgui_coord_t        x1, 
                                        ipgui_coord_t        y1, 
                                        ipgui_coord_t        x2, 
                                        ipgui_coord_t        y2, 
                                        u8_t               * pix_buf, 
                                        s32_t                stride);

extern __IPGUI_API__ void        ipgui_screen_flush(
                                        ipgui_scr_t        * scr);

extern __IPGUI_API__ void        ipgui_screen_handle_widget_event(
                                        ipgui_scr_t        * scr, 
                                        ipgui_widget_evt_t * evt);

#endif
