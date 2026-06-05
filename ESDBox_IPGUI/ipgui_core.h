#ifndef IPGUI_CORE_H
#define IPGUI_CORE_H

#include "ipgui_lcd_pix_fmt.h"
#include "ipgui_prim.h"

extern __IPGUI_API__ ipgui_err_t ipgui_init(void);

/* 离屏缓冲/帧缓冲映射到屏幕上的表面，帧缓冲可以是相对坐标，用于绘制控件因为控件的坐标是相对于父控件的，具体用法视情况而定 */
typedef struct {
    /* the region of screen offline buffer 
     * or frame buffer 
     */
    ipgui_aabb_t    surf;

    /* point to the first pixel in the surf aabb */
    u8_t          * color;

    /* surf aabb每行所占用的内存字节数，
     * ipgui_surf_t作为帧缓冲时，stride's value = (x2-x1+1)*每个像素的字节数，
     * 作为离屏缓冲时可以大于stride's value
     */
    u32_t           stride;

    ipgui_pix_fmt_t pix_fmt;

    /* 每像素所占内存字节数（可能包含空字节用于对齐）
     * 例如RGB888的pix_size为3,但是在内存中可能占用4个字节为RGBx8888，x为无效字节
     * ipgui_surf_t作为帧缓冲时，pix_size's value = pix_fmt所指定的字节数，
     * 作为离屏缓冲时可以大于pix_size's value
     */
    u8_t            pix_size; 
}ipgui_surf_t;

/* 离屏缓冲/帧缓冲，只含大小和像素信息，不含位置信息 */
typedef struct {
    /* the pixels number */
    ipgui_coord_t   num_pixs;

    /* point to the first pixel in the surf aabb */
    u8_t          * color;


    ipgui_pix_fmt_t pix_fmt;

    /* 每像素所占内存字节数（可能包含空字节用于对齐）
     * 例如RGB888的pix_size为3,但是在内存中可能占用4个字节为RGBx8888，x为无效字节
     * ipgui_surf_t作为帧缓冲时，pix_size's value = pix_fmt所指定的字节数，
     * 作为离屏缓冲时可以大于pix_size's value
     */
    u8_t            pix_size; 
}ipgui_pfb_t;

/* x和y是surf内部的相对于surf的左上角的相对坐标 */
__IPGUI_STATIC__ __IPGUI_INLINE__ unsigned char * ipgui_surf_color_get(ipgui_surf_t * surf, ipgui_coord_t x, ipgui_coord_t y)
{
    return (surf->color + y * surf->stride + x * surf->pix_size);
}

#endif