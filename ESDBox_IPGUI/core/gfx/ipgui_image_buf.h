#ifndef IPGUI_IMAGE_BUF_H
#define IPGUI_IMAGE_BUF_H

#include "ipgui_prim.h"

/* 申请image缓冲区，框架内部调用 */
u8_t * ipgui_image_buf_acquire(u32_t w_stride, ipgui_coord_t h, ipgui_coord_t * res_h);

/* 释放mask缓冲区，框架内部调用 */
void ipgui_image_buf_free(void * data);

#endif