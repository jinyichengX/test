#include "ipgui_screen.h"
#include "ipgui_memory.h"
#include "ipgui_debug.h"

__IPGUI_API__ int ipgui_screen_register(ipgui_scr_drv_t * drv)
{
    ipgui_scr_t * scr = ipgui_mem_alloc(ipgui_smem, sizeof(ipgui_scr_t));
    if (!scr)
        return (ipgui_scr_t *)0;
    ipgui_memset((void *)scr, 0, sizeof(ipgui_scr_t));
    scr->drv = drv;

    /* 初始化脏矩形管理器 */
    ipgui_dirty_rect_mgr_init(&scr->dirty);

    /* 初始化控件树 */
    ipgui_widget_tree_init(&scr->root);

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

__IPGUI_API__ void ipgui_screen_handle_widget_event(ipgui_scr_t * scr, ipgui_widget_evt_t * evt)
{

}