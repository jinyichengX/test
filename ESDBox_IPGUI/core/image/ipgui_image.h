#ifndef IPGUI_IMAGE_H
#define IPGUI_IMAGE_H

#include "ipgui_prim.h"
#include "ipgui_blend_image.h"

typedef struct {
    ipgui_image_fomat_t fmt;
    ipgui_coord_t w, h;
    ipgui_coord_t stride; /* width stride */ /* 这个参数是用于加速用，一般是每像素大小乘以宽度 */
    unsigned char * pixmap;/* 像素数据 */

    unsigned char * mask; /* 1. mask是整张图片的mask 2. 与alpha通道要区别开 */
}ipgui_img_dsc_t;

typedef struct {
    void * usr_data;
    void (* img_decoder)(void);
    
}ipgui_img_ops_t;

#endif