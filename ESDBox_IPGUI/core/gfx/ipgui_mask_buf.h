#ifndef IPGUI_MASK_BUF_H
#define IPGUI_MASK_BUF_H

#include "ipgui_prim.h"

/* 申请mask缓冲区，框架内部调用 */
u8_t * ipgui_mask_buf_acquire(ipgui_coord_t w, ipgui_coord_t h, ipgui_coord_t * res_h);

/* 释放mask缓冲区，框架内部调用 */
void ipgui_mask_buf_free(void * mask);

#endif