#ifndef IPGUI_LCD_PIX_FMT_H
#define IPGUI_LCD_PIX_FMT_H

/* 屏幕像素格式 */
typedef enum {

    /* 16-bit */
    /*
    格式     端序  [0]低字节 [1]高字节
    RGB565  小端  GGGBBBBB  RRRRRGGG 
    RGB565  大端  RRRRRGGG  GGGBBBBB 
    BGR565  小端  GGGRRRRR  BBBBBGGG
    BGR565  大端  BBBBBGGG  GGGRRRRR
    */
    PIX_FMT_RGB565 = 0,        /* 内存顺序: R[4:0]G[5:3] | G[2:0]B[4:0] */
    PIX_FMT_BGR565 = 1,        /* 内存顺序: B[4:0]G[5:3] | G[2:0]R[4:0] */
    
    /* 24-bit */
    PIX_FMT_RGB888 = 2,        /* 内存顺序: R-G-B */
    PIX_FMT_BGR888 = 3,        /* 内存顺序: B-G-R */
    
    /* 32-bit */
    PIX_FMT_ARGB8888 = 4,      /* 内存顺序: A[7:0]R[7:0]G[7:0]B[7:0] */
    PIX_FMT_ABGR8888 = 5,      /* 内存顺序: A[7:0]B[7:0]G[7:0]R[7:0] */
    PIX_FMT_RGBA8888 = 6,      /* 内存顺序: R[7:0]G[7:0]B[7:0]A[7:0] */
    PIX_FMT_BGRA8888 = 7,      /* 内存顺序: B[7:0]G[7:0]R[7:0]A[7:0] */

    PIX_FMT_MAX,
}ipgui_pix_fmt_t;

#endif 