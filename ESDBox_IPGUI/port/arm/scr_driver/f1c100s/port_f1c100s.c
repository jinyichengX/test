#include "ipgui_draw_arc.h"

typedef s16_t int16_t;
typedef u16_t uint16_t;
extern void LCD_CopyRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t * pc);

#define SURF_BUF_WIDTH 256
#define SURF_BUF_HEIGHT 256
unsigned char surf_buf[SURF_BUF_WIDTH * SURF_BUF_HEIGHT];

void draw_system_test(void)
{
    fb_f1c100s_init(0);
    
    ipgui_init();

    ipgui_surf_t surf;
    surf.surf.start.x = 0;
    surf.surf.start.y = 0;
    surf.surf.end.x   = SURF_BUF_WIDTH - 1;
    surf.surf.end.y   = SURF_BUF_HEIGHT - 1;
    surf.color  = surf_buf;
    surf.stride = SURF_BUF_WIDTH * 2/* size of rgb565 = 2 */;
    surf.pix_fmt = PIX_FMT_RGB565;
    surf.pix_size = 2/* size of rgb565 = 2 */;
    
    /* ---- 绿色圆弧 ---- */
    ipgui_arc_t arc;
    arc.cx    = 160;
    arc.cy    = 40;
    arc.er    = 25;
    arc.ir    = 10;
    arc.start = 0;
    arc.dir   = IPGUI_ARC_DRAW_DIR_CW;
    arc.angle = 270;

    ipgui_arc_style_t arc_style;
    arc_style.blend_mode = IPGUI_BLEND_NORMAL;
    arc_style.opacity    = 255;
    arc_style.sep_type   = IPGUI_ARC_ENDPOINT_TYPE_ROUND;
    arc_style.eep_type   = IPGUI_ARC_ENDPOINT_TYPE_ROUND;
    arc_style.paint.type = IPGUI_PAINT_COLOR;
    IPGUI_COLOR_SET(arc_style.paint.src.color, 255, 0x00FF00);

    ipgui_draw_arc(
        &surf,
        (ipgui_aabb_t *)0, 
        &arc, 
        &arc_style);

    LCD_CopyRect(
        0, 
        0, 
        SURF_BUF_WIDTH, 
        SURF_BUF_HEIGHT, 
        (uint16_t *)surf.color);
}