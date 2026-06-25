#include "ipgui_anim_bounce.h"

ipgui_anim_value_t ipgui_anim_bounce(ipgui_tick_t t)
{
    ipgui_anim_value_t x = (ipgui_anim_value_t)t;
    if (x <= 0) {
        return 0;
    }

    /*
     * 超过峰值点后不再振荡，直接返回线性值，
     * 保证终点附近 knob 平滑到位、绝不冲出边界。
     */
#define BOUNCE_PEAK 80
    if (x >= BOUNCE_PEAK) {
        return x;
    }

    /*
     * 固定周期 ≈ peak/3，全程 3 次等频振荡，消除"一抖一抖"感。
     * 固定频率让眼睛感知到的节奏规律、不突兀。
     */
#define BOUNCE_PERIOD 27
    ipgui_tick_t pos  = (ipgui_tick_t)x % BOUNCE_PERIOD;
    ipgui_tick_t half = BOUNCE_PERIOD >> 1;

    /*
     * 振幅 = 抛物线包络，两端为 0、中间最大。
     * x=40 时峰值 amp ≈ 20 tick，映射到 knob 位置约 ±12px，明显可见。
     */
    ipgui_anim_value_t amp = (x * (BOUNCE_PEAK - x)) / BOUNCE_PEAK;
    if (amp < 1) {
        return x;
    }

    /* 三角波 [-amp, +amp]，pos=0 时输出 +amp */
    ipgui_anim_value_t ph;
    if (pos <= half) {
        ph = amp - (ipgui_anim_value_t)((amp * pos * 2) / half);
    } else {
        ipgui_tick_t d = pos - half;
        ph = (ipgui_anim_value_t)((amp * d * 2) / half) - amp;
    }

    return x + ph;
}
