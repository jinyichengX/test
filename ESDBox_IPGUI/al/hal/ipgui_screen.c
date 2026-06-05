#include "ipgui_screen.h"
#include "ipgui_memory.h"
#include "ipgui_debug.h"

__IPGUI_API__ ipgui_err_t ipgui_screen_init(ipgui_scr_t * scr, ipgui_scr_drv_t * drv)
{
    if (!scr)
        return IPGUI_ERR_NOK;
    ipgui_memset((void *)scr, 0, sizeof(ipgui_scr_t));
    scr->drv = drv;

    /* 初始化脏矩形管理器
     * 标记全屏为脏区域
     */
    ipgui_dirty_rect_mgr_init(&scr->dirty);
    ipgui_dirty_rect_add_xywh(&scr->dirty, 
        0, 
        0, 
        scr->drv->xreso, 
        scr->drv->yreso);

    /* 初始化控件树 */
    ipgui_widget_tree_init(&scr->root);

    return IPGUI_ERR_OK;
}

__IPGUI_API__ ipgui_err_t ipgui_scr_create_pfb(
    ipgui_scr_t   * scr, 
    u8_t          * buf, 
    u32_t           buf_size, 
    ipgui_pix_fmt_t pix_fmt)
{
    if (!scr || !buf || !buf_size) {
        ipgui_dbg_error("param is invalid\n");
        return IPGUI_ERR_PARAM;
    }

    if (pix_fmt >= PIX_FMT_MAX) {
        ipgui_dbg_error("pix_fmt is invalid\n");
        return IPGUI_ERR_PARAM;
    }

    if (scr->drv->xreso <= 0 || scr->drv->yreso <= 0) {
        ipgui_dbg_error("screen resolution is invalid\n");
        return IPGUI_ERR_PARAM;
    }

    u8_t * top_buf = buf + buf_size;
    u32_t valid_size;
    u8_t * pfb_buf = IPGUI_ALIGN_U32(buf);
    
    if (pfb_buf >= top_buf) return IPGUI_ERR_NOK;

    valid_size = top_buf - pfb_buf;

    u8_t px_size = 0;
    switch ((u8_t)pix_fmt) {
        case PIX_FMT_RGB565:
        case PIX_FMT_BGR565:   px_size = 2; break;
        case PIX_FMT_RGB888:
        case PIX_FMT_BGR888:   px_size = 4; break;
        case PIX_FMT_RGBA8888:
        case PIX_FMT_BGRA8888: px_size = 4; break;
        default: break;
    }

    scr->pfb.num_pixs = valid_size / px_size;
    scr->pfb.color    = pfb_buf;
    scr->pfb.pix_fmt  = pix_fmt;
    scr->pfb.pix_size = px_size;

    /* clear buf */
    ipgui_memset(pfb_buf, 0, valid_size);

    return IPGUI_ERR_OK;
}

/* render dirty rect of screen */
__IPGUI_STATIC__ void ipgui_screen_render_dirty_rect(
    ipgui_scr_t * scr,
    ipgui_dirty_rect_t * dirty)
{

}

/* render screen */
__IPGUI_API__ void ipgui_screen_render(ipgui_scr_t * scr)
{
    /* check if the screen have dirty region */
    if (scr->dirty.pool_num == 0) return;

    /* 最优合并 */
    ipgui_dirty_rect_flush(&scr->dirty);

    s32_t idx = 0;
    ipgui_dirty_rect_t * dirty; 
    for (; idx < scr->dirty.pool_num; idx ++) {
        dirty = &scr->dirty.pool[idx];
        ipgui_screen_render_dirty_rect(scr, dirty);
    }

    /* reset dirty rect manager */
    ipgui_dirty_rect_mgr_reset(&scr->dirty);
}

__IPGUI_API__ void ipgui_screen_putpixel(ipgui_scr_t * scr, ipgui_coord_t x, ipgui_coord_t y, u8_t * pix)
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
        u8_t * pix_buf, s32_t stride)
{
    if (scr && scr->drv && scr->drv->fill_region) {
        scr->drv->fill_region(scr, x1, y1, x2, y2, pix_buf, stride);
    }
}

__IPGUI_API__ void ipgui_screen_handle_widget_event(ipgui_scr_t * scr, ipgui_widget_evt_t * evt)
{

}