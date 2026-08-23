#include "ipgui_blur.h"

__IPGUI_API__ void ipgui_blur_mask(
    u8_t        * mask, 
    ipgui_coord_t w, 
    ipgui_coord_t h)
{
    /* 策略1：三次均值模糊逼近高斯模糊 */


    /* 策略2：
     * （1）降采样 
     * （2）单次均值模糊
     * （3）升采样 
     */
    
}