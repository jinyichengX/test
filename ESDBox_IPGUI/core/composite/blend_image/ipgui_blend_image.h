#ifndef IPGUI_BLEND_IMAGE_H
#define IPGUI_BLEND_IMAGE_H

#include "ipgui_core.h"
#include "ipgui_blend_mode.h"

typedef enum {
    /* luminance */
    IPGUI_IMG_FMT_L8 = 0,       /* 8bit亮度（灰度图） */
    IPGUI_IMG_FMT_LA88,     /* 带alpha通道的8bit亮度（灰度图） */

    /* 16bit */
    IPGUI_IMG_FMT_RGB565,   /* 内存顺序: R[4:0]G[5:3] | G[2:0]B[4:0] */
    IPGUI_IMG_FMT_BGR565,   /* 内存顺序: B[4:0]G[5:3] | G[2:0]R[4:0] */

    /* 这个内存顺序是对的，可以取消注释 */
    // IPGUI_IMG_FMT_RGBA5658, /* 内存顺序: R[4:0]G[4:3] | G[2:1]B[4:0] A[7:0] */
    // IPGUI_IMG_FMT_ARGB8565, /* 内存顺序: A[7:0] R[4:0]G[4:3] | G[2:1]B[4:0] */
    // IPGUI_IMG_FMT_BGRA5658, /* 内存顺序: B[4:0]G[5:3] | G[2:0]R[4:0] A[7:0] */
    // IPGUI_IMG_FMT_ABGR8565, /* 内存顺序: A[7:0] B[4:0]G[5:3] | G[2:0]R[4:0] */
    
    /* 注释掉的部分内存顺序可能不对 */
    // IPGUI_IMG_FMT_RGBA5551, /* 内存顺序: R[4:0]G[4:0]B[4:0] A[0] */
    // IPGUI_IMG_FMT_ARGB1555, /* 内存顺序: A[0] R[4:0]G[4:0]B[4:0] */
    // IPGUI_IMG_FMT_BGRA5551, /* 内存顺序: B[4:0]G[4:0]R[4:0] A[0] */
    // IPGUI_IMG_FMT_ABGR1555, /* 内存顺序: A[0] B[4:0]G[4:0]R[4:0] */

    // IPGUI_IMG_FMT_RGBA4444, /* 内存顺序: R[3:0]G[3:0]B[3:0] A[3:0] */
    // IPGUI_IMG_FMT_ARGB4444, /* 内存顺序: A[3:0] R[3:0]G[3:0]B[3:0] */
    // IPGUI_IMG_FMT_BGRA4444, /* 内存顺序: B[3:0]G[3:0]R[3:0] A[3:0] */
    // IPGUI_IMG_FMT_ABGR4444, /* 内存顺序: A[3:0] B[3:0]G[3:0]R[3:0] */

    /* 24bit */
    IPGUI_IMG_FMT_RGB888,   /* 内存顺序: R-G-B */
    IPGUI_IMG_FMT_BGR888,   /* 内存顺序: B-G-R */

    /* 32bit */
    IPGUI_IMG_FMT_ARGB8888, /* 内存顺序: A[7:0]R[7:0]G[7:0]B[7:0] */
    IPGUI_IMG_FMT_ABGR8888, /* 内存顺序: A[7:0]B[7:0]G[7:0]R[7:0] */
    IPGUI_IMG_FMT_RGBA8888, /* 内存顺序: R[7:0]G[7:0]B[7:0]A[7:0] */
    IPGUI_IMG_FMT_BGRA8888, /* 内存顺序: B[7:0]G[7:0]R[7:0]A[7:0] */

    IPGUI_IMG_FMT_MAX,
}ipgui_image_fomat_t;

/* image src是image_data进行定位后的图像数据，可以是一张完整图像的子图 */
typedef struct {
    ipgui_aabb_t      * img_aabb;   /* 图像包围盒，包围盒必须与图像大小一致！ */
    u32_t               stride;     /* 图像行跨度（单位：字节） */   
    u8_t              * buf;        /* 图像数据 */
    ipgui_image_fomat_t img_pxfmt;  /* 图像像素格式 */
    u8_t                px_size;    /* 每像素大小（单位：字节），必须大于等于像素格式对应的字节数 */
}ipgui_image_src_t;

/* image data是一张图像的格式/像素信息，不包含定位信息，可以是一张完整图像的子图 */
typedef struct {
    /* 图片宽度和高度 */
    ipgui_coord_t       w, h;

    /* per line stride
     * 这个参数是用于加速用，一般是每像素大小乘以宽度 + padding
     * 可以让一个ipgui_img_raw_t指向大图的中间的部分子图
     * 只要stride依然是大图的宽度，采样器就能正确换行
     */
    u32_t              stride;

    /* 像素数据 */
    u8_t             * pixmap;

    /* 像素格式 */
    ipgui_image_fomat_t fmt;

    u8_t                px_size;    /* 每像素大小（单位：字节），必须大于等于像素格式对应的字节数 */
}ipgui_image_data_t;

extern __IPGUI_API__ void ipgui_blend_image_v1(
    ipgui_surf_t      * surf,
    ipgui_aabb_t      * clip,
    ipgui_image_src_t * img_src,
    u8_t              * mask,       /* 蒙版 */
    ipgui_aabb_t      * mask_aabb,  /* 蒙版区域，必须大于等于图像包围盒*/
    u8_t                opacity,
    ipgui_blend_mode_t  blend_mode);

extern __IPGUI_API__ void ipgui_blend_image_v2(
    ipgui_surf_t      * surf,
    ipgui_aabb_t      * clip,
    ipgui_aabb_t      * dest,
    ipgui_image_src_t * img_src,
    u8_t                opacity,
    u8_t              * mask,       /* 蒙版 */
    ipgui_aabb_t      * mask_aabb,  /* 蒙版区域，必须大于等于dest围盒*/
    ipgui_blend_mode_t  blend_mode);

#endif
