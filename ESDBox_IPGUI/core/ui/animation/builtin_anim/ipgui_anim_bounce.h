#ifndef IPGUI_ANIM_BOUNCE_H
#define IPGUI_ANIM_BOUNCE_H

#include "ipgui_animation.h"

/*
 * 弹性动画函数：输出围绕线性值上下振荡并逐渐收敛。
 *
 * 实现方式：叠加几何衰减的三角形波 —— 周期逐次翻倍、振幅逐次减半，
 * 振荡特性随输入自适应，无硬编码参数，纯整数运算。
 */
ipgui_anim_value_t ipgui_anim_bounce(ipgui_tick_t t);

#endif
