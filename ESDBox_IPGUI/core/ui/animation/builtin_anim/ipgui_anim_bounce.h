#ifndef IPGUI_ANIM_BOUNCE_H
#define IPGUI_ANIM_BOUNCE_H

#include "ipgui_animation.h"

/*
 * 弹簧过冲动画函数 (Cubic Bezier Spring).
 *
 * knob 冲向终点 → 超越 → 回弹 → 收敛，真正的弹簧手感。
 * 不再是旧版绕线性路径的原地振荡。
 *
 * B(t) = 3(1-t)²t·P1 + 3(1-t)t²·P2 + t³·P3
 * P1 ≈ 0.06·TOTAL (初始粘滞), P2 ≈ 1.44·TOTAL (过冲力度), P3 = TOTAL (终点).
 *
 * f(0)=0, f(TOTAL)=TOTAL, 中途 f(x) > TOTAL 实现物理 overshoot.
 * 纯整数运算, 无浮点/无除法查表.
 */
ipgui_anim_value_t ipgui_anim_bounce(struct ipgui_anim_t * anim, ipgui_tick_t t, void * data);

#endif
