#ifndef RGB_H
#define RGB_H

typedef struct {
    int de;    /* 高/低电平使能数据 */
	int vclk;  /* 在上/下降沿获取数据 */
	int hsync; /* 高/低脉冲 */
	int vsync; /* 高/低脉冲  */
}rgb_timing_pol_t;

typedef struct {
    unsigned int hsw : 16;      /* horizontal sync width  */
    unsigned int hbp : 8;       /* horizontal back porch  */
    unsigned int hfp : 8;       /* horizontal front porch */

    unsigned int vsh : 16;      /* vertical sync height   */
    unsigned int vbp : 8;       /* vertical back porch    */
    unsigned int vfp : 8;       /* vertical front porch   */

    unsigned int aw : 12;       /* active width           */
    unsigned int ah : 12;       /* active height          */

    unsigned int bpp : 8;       /* bits per pixel         */
}rgb_panel_param_t;

#endif