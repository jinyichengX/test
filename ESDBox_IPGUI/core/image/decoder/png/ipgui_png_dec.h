#ifndef __IPGUI_PNG_DEC_H__
#define __IPGUI_PNG_DEC_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "ipgui_coord.h"
#include "ipgui_types.h"

/* PNG颜色类型 */
#define PNG_COLOR_GRAY       0  /* 灰度 */
#define PNG_COLOR_RGB        2  /* RGB */
#define PNG_COLOR_PALETTE    3  /* 索引色 */
#define PNG_COLOR_GRAY_ALPHA 4  /* 灰度+Alpha */
#define PNG_COLOR_RGBA       6  /* RGBA */

/* PNG滤波器类型 */
#define PNG_FILTER_NONE    0
#define PNG_FILTER_SUB     1
#define PNG_FILTER_UP      2
#define PNG_FILTER_AVG     3
#define PNG_FILTER_PAETH   4

struct png_inf
{
    const char *path;

    ipgui_coord_t w;
    ipgui_coord_t h;
    
    unsigned char bit_depth;    /* 1, 2, 4, 8, 16 */
    unsigned char color_type;   /* 0, 2, 3, 4, 6 */
    unsigned char interlace;    /* 0:无隔行 1:Adam7 */
    
    unsigned char bpp;          /* 每像素字节数(解码后) */
    unsigned char channels;     /* 通道数 */
    
    unsigned int plte_entries;  /* 调色板条目数 */
    unsigned char *palette;     /* 调色板 RGB, 3字节/条目 */
    unsigned char *trns;        /* 透明度信息 */
    unsigned int trns_entries;
    
    /* IDAT数据信息 */
    unsigned int idat_offset;   /* 第一个IDAT的数据偏移 */
    unsigned int idat_total_sz; /* 所有IDAT数据总大小 */
};

/* 解析PNG头信息 */
ipgui_err_t ipgui_png_dec(const char *path, struct png_inf *inf);

/* 逐行读取，输出BGRA32格式 */
ipgui_err_t ipgui_png_read_linebyline(struct png_inf *inf,
    ipgui_coord_t img_x, ipgui_coord_t img_y,
    void *buffer, ipgui_coord_t pix_num,
    ipgui_coord_t *pixs_nr);

/* 释放png_inf中分配的内存 */
void ipgui_png_free(struct png_inf *inf);

#ifdef __cplusplus
}
#endif

#endif /* __IPGUI_PNG_DEC_H__ */