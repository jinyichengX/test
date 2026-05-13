#include "elcdif.h"
#include "rgb.h"

/* 只能使用24位，其他还没调通 */

#ifndef ipgui_writel
#define ipgui_writel(a, v)                  (*(volatile unsigned int *)(a) = (v))
#endif
#ifndef ipgui_readl
#define ipgui_readl(a)                      (*(volatile unsigned int *)(a))
#endif

/* clock configure registers */
#define CCM_CSCDR2_ADDR                     0x020c4038
#define CCM_CBCMR_ADDR                      0x020c4018

/* elcdif pad iomux registers */
#define ELCDIF_IOMUX_PAD_LCD_CLK_ADDR       0x020e0104          /* LCD CLK */
#define ELCDIF_IOMUX_PAD_LCD_ENABLE_ADDR    0x020e0108          /* LCD ENABLE */
#define ELCDIF_IOMUX_PAD_LCD_HSYNC_ADDR     0x020e010c          /* LCD HSYNC */
#define ELCDIF_IOMUX_PAD_LCD_VSYNC_ADDR     0x020e0110          /* LCD VSYNC */
#define ELCDIF_IOMUX_PAD_LCD_RESET_ADDR     0x020e0114          /* LCD RESET */
#define ELCDIF_IOMUX_PAD_LCD_DATAn_ADDR(n)  (0x020e0118+4*(n))  /* LCD DATAn */ /* n:0~23 */

/* elcdif pad attr registers */
#define ELCDIF_CTL_PAD_LCD_CLK_ADDR         0x020e0390          /* LCD CLK */
#define ELCDIF_CTL_PAD_LCD_ENABLE_ADDR      0x020e0394          /* LCD ENABLE */
#define ELCDIF_CTL_PAD_LCD_HSYNC_ADDR       0x020e0398          /* LCD HSYNC */
#define ELCDIF_CTL_PAD_LCD_VSYNC_ADDR       0x020e039c          /* LCD VSYNC */
#define ELCDIF_CTL_PAD_LCD_RESET_ADDR       0x020e03a0          /* LCD RESET */
#define ELCDIF_CTL_PAD_LCD_DATAn_ADDR(n)    (0x020e03a4+4*(n))  /* LCD DATAn */ /* n:0~23 */

/* elcdif registers */
#define ELCDIF_CTRL_ADDR                    0x021c8000
#define ELCDIF_CTRL1_ADDR                   0x021c8010
#define ELCDIF_CTRL2_ADDR                   0x021c8020
#define ELCDIF_TRANSFER_COUNT_ADDR          0x021c8030
#define ELCDIF_CUR_BUF_ADDR                 0x021c8040
#define ELCDIF_NEXT_BUF_ADDR                0x021c8050
#define ELCDIF_VDCTRL0_ADDR                 0x021c8070
#define ELCDIF_VDCTRL1_ADDR                 0x021c8080
#define ELCDIF_VDCTRL2_ADDR                 0x021c8090
#define ELCDIF_VDCTRL3_ADDR                 0x021c80a0
#define ELCDIF_VDCTRL4_ADDR                 0x021c80b0

static rgb_panel_param_t lcd_para_default = {
    /* 非可视区（消隐区）参数 */
    .hsw = 20,
    .hbp = 140,
    .hfp = 160,

    .vsh = 3,
    .vbp = 20,
    .vfp = 12,

    /* 可视区参数 */
    .aw = 1024,
    .ah = 600,
	
	.bpp = 24
};

static void delay(int x)
{
    while(x --);
}

static volatile unsigned int * GPIO1_IO08_PAD =   (volatile unsigned int*)0x20e0308;
static volatile unsigned int * GPIO1_GDIR     =   (volatile unsigned int*)0x209c000;
static volatile unsigned int * GPIO1_DR       =   (volatile unsigned int*)0x209c004;

static void lcd_backlight_on(void)
{
    *GPIO1_IO08_PAD     = 0xb9;
    *GPIO1_GDIR        |= (1<<8);
	*GPIO1_DR          |= (1<<8);
}

/* configure elcdif */
int elcdif_ll_init(rgb_panel_param_t * dsc, rgb_timing_pol_t * pol, void * cur_fb, void * next_fb)
{   
    int reg;
    unsigned int mode;
    if ((!dsc) || (!pol) || (!cur_fb)) return -1;

    // if ((dsc->bpp != 24)&&(dsc->bpp != 16)) return -1;

    /* configure iomux 
     * then configure pad electronic attributes 
     */
    ipgui_writel(ELCDIF_IOMUX_PAD_LCD_CLK_ADDR,    0x00000000);         /* LCD_CLK */
    ipgui_writel(ELCDIF_IOMUX_PAD_LCD_ENABLE_ADDR, 0x00000000);         /* LCD_ENABLE */
    ipgui_writel(ELCDIF_IOMUX_PAD_LCD_HSYNC_ADDR,  0x00000000);         /* LCD_HSYNC */
    ipgui_writel(ELCDIF_IOMUX_PAD_LCD_VSYNC_ADDR,  0x00000000);         /* LCD_VSYNC */
    for (int i = 0; i < dsc->bpp; i++)
    ipgui_writel(ELCDIF_IOMUX_PAD_LCD_DATAn_ADDR(i), 0x00000000);       /* LCD_DATAn */

    ipgui_writel(ELCDIF_CTL_PAD_LCD_CLK_ADDR,      0xb9);               /* LCD_CLK */
    ipgui_writel(ELCDIF_CTL_PAD_LCD_ENABLE_ADDR,   0xb9);               /* LCD_ENABLE */
    ipgui_writel(ELCDIF_CTL_PAD_LCD_HSYNC_ADDR,    0xb9);               /* LCD_HSYNC */
    ipgui_writel(ELCDIF_CTL_PAD_LCD_VSYNC_ADDR,    0xb9);               /* LCD_VSYNC */
    for (int i = 0; i < dsc->bpp; i++)
    ipgui_writel(ELCDIF_CTL_PAD_LCD_DATAn_ADDR(i), 0xb9);               /* LCD_DATAn */

    /* configure pixel clock only
     * suppose video pll(pll5) has been configured 768Mhz 
     */
    reg = ipgui_readl(CCM_CSCDR2_ADDR);
    reg &= ~(7 << 15); 	   
    reg |=  (2 << 15);                  /* choose pll5(video pll) */
    reg &= ~(7 << 12); 	 
    reg |=  (2 << 12); 					/* divide by 3 */	 
    reg &= ~(7 << 9);
    ipgui_writel(CCM_CSCDR2_ADDR, reg);

    reg = ipgui_readl(CCM_CBCMR_ADDR);
    reg &= ~(7 << 23);					 
    reg |=	4 << 23;	                /* divide by 5 */
    ipgui_writel(CCM_CBCMR_ADDR, reg);

    /* must force a blocked level soft reset */
    reg = ipgui_readl(ELCDIF_CTRL_ADDR);
    reg |= (1 << 31);
    ipgui_writel(ELCDIF_CTRL_ADDR, reg);
    delay(100);  /* wait several clocks */
    reg = ipgui_readl(ELCDIF_CTRL_ADDR);
    reg &= ~(1 << 31);
    ipgui_writel(ELCDIF_CTRL_ADDR, reg);

    /* enable clock gate */
    reg = ipgui_readl(ELCDIF_CTRL_ADDR);
    reg &= ~(1 << 30);
    ipgui_writel(ELCDIF_CTRL_ADDR, reg);
    /* 上面都可以不用动了 */

    /* configure elcdif */
    if (dsc->bpp == 16)
        mode = 0;
    else if (dsc->bpp == 24)
        mode = 3;

    reg = ipgui_readl(ELCDIF_CTRL_ADDR);
    reg |= (1 << 19) | (1 << 17) | (3 << 10) | (mode << 8) | (1 << 5); /* configure DOTCLK mode interface */
    ipgui_writel(ELCDIF_CTRL_ADDR, reg);

    if (mode == 0) {
        reg = ipgui_readl(ELCDIF_CTRL_ADDR);
        reg &= ~(1 << 3);
        ipgui_writel(ELCDIF_CTRL_ADDR, reg);

        reg = ipgui_readl(ELCDIF_CTRL1_ADDR);
        reg &= ~(0xf << 16);
        reg |= (0x0f << 16);
        ipgui_writel(ELCDIF_CTRL1_ADDR, reg);
    }
    /* if mode == 3, then 4 bytes(ARGB) indicate 1 pixel, drop Alpha channel */
    else if (mode == 3) {
        reg = ipgui_readl(ELCDIF_CTRL1_ADDR);
        reg &= ~(0xf << 16);
        reg |= (0x07 << 16);
        ipgui_writel(ELCDIF_CTRL1_ADDR, reg);
    }
    /* 下面都可以不用动了 */
    reg = (dsc->ah << 16) | dsc->aw;
    ipgui_writel(ELCDIF_TRANSFER_COUNT_ADDR, reg);      /* configure active area */

    reg = (1 << 28) | (pol->vsync << 27) | (pol->hsync << 26) | (pol->vclk << 25) | (pol->de << 24) /* configure VDCTRL0: pol and HV or DE mode */
          | (1 << 21) | (1 << 20) | (dsc->vsh << 0);
    ipgui_writel(ELCDIF_VDCTRL0_ADDR, reg);

    reg = dsc->vbp + dsc->vfp + dsc->vsh + dsc->ah;      /* configure VDCTRL1: VBP + VFP + VSH + VACT */
    ipgui_writel(ELCDIF_VDCTRL1_ADDR, reg);

    reg = (dsc->hsw << 18) | (dsc->hbp + dsc->hfp + dsc->hsw + dsc->aw );                 /* configure VDCTRL2: HBP + HFP + HSW + HACT */
    ipgui_writel(ELCDIF_VDCTRL2_ADDR, reg);

    reg = ((dsc->hsw + dsc->hbp) << 16) | (dsc->vsh + dsc->vbp);    /* configure VDCTRL3 */
    ipgui_writel(ELCDIF_VDCTRL3_ADDR, reg);

    reg = (1 << 18) | (dsc->aw << 0);                    /* configure VDCTRL4 */
    ipgui_writel(ELCDIF_VDCTRL4_ADDR, reg);

    reg = (int)cur_fb;
    ipgui_writel(ELCDIF_CUR_BUF_ADDR, reg);              /* configure CUR_BUF */

    if (next_fb)
        reg = (int)next_fb;
    ipgui_writel(ELCDIF_NEXT_BUF_ADDR, reg);             /* configure NEXT_BUF */

    /* turn back light on */
    lcd_backlight_on();

    /* enable lcd controller */
    reg = ipgui_readl(ELCDIF_CTRL_ADDR);
    reg |= (1 << 0);
    ipgui_writel(ELCDIF_CTRL_ADDR, reg);

    return 0;
}

