#include "ipgui_memory.h"
#include "ipgui_timer.h"
#include "ipgui_screen.h"
#include "ipgui_gradient_color.h"
#include "ipgui_draw_polygon.h"

__IPGUI_INIT__ ipgui_err_t ipgui_timer_moudle_init(void);
ipgui_err_t ipgui_bmp_dec_module_init(void);
__IPGUI_API__ __IPGUI_INIT__ ipgui_err_t ipgui_init(void)
{
    ipgui_err_t err;

    err = ipgui_mem_module_init();
    if (err != IPGUI_ERR_OK)
        return err;

    err = ipgui_timer_moudle_init();
    if (err != IPGUI_ERR_OK)
        return err;
    err = ipgui_bmp_dec_module_init();
    if (err != IPGUI_ERR_OK)
        return err;
    err = ipgui_polygon_ras_init(&g_ras);
    if (err != IPGUI_ERR_OK)
        return err;
#if IPGUI_GRADIENT_LUT_EN == 1
    ipgui_gradient_lut_init();
#endif

    return IPGUI_ERR_OK;
}