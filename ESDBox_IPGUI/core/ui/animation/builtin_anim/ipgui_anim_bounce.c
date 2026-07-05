#include "ipgui_anim_bounce.h"

/*
 * 三次贝塞尔弹簧曲线：knob 冲向目标 → 超越 → 回弹 → 停稳。
 *
 * 控制点:
 *   P0 = 0     起点
 *   P1 ≈ 0.06·TOTAL   初始粘滞 (慢出)
 *   P2 ≈ 1.44·TOTAL   过冲幅度, 越大弹簧越猛
 *   P3 = TOTAL  终点收敛
 *
 * 曲线形状:
 *   y ↑
 *   96|          ╱‾‾╲___
 *   80|         ╱       ╲___  ← 冲过头再弹回来停住
 *   48|        ╱
 *   16|       ╱
 *    0|______╱______________→ x
 *      0    20   40   60   80
 *
 * 旧方案: y = x + 振荡   (在路上摇头晃脑, 永远不超终点)
 * 新方案: y = 贝塞尔过冲 (冲过头再优雅回弹, 真正的弹簧手感)
 */
ipgui_anim_value_t ipgui_anim_bounce(ipgui_tick_t t)
{
    ipgui_anim_value_t x = (ipgui_anim_value_t)t;
    if (x <= 0) return 0;

#define TOTAL 80
    if (x >= TOTAL) return TOTAL;

    /*
     * Cubic Bezier: B(t)=3(1-t)²t·P1+3(1-t)t²·P2+t³·P3
     * 其中 t = x/TOTAL (归一化进度)
     *
     * 整数化: 分子各项乘以 TOTAL³ 后累加, 最后整体除 TOTAL³
     */

#define P1 5      /* 初始加速度, 小=粘滞慢出 */
#define P2 150    /* 过冲力度, 约为 1.44×TOTAL */

    s32_t d  = TOTAL - x;          /* TOTAL·(1-t) */
    s32_t d2 = d * d;              /* TOTAL²·(1-t)² */
    s32_t x2 = x * x;              /* TOTAL²·t² */

    /*  3(1-t)²t·P1 + 3(1-t)t²·P2 + t³·P3,  放大 TOTAL³ 倍 */
    s32_t n  = 3 * d2 * x * P1
             + 3 * d * x2 * P2
             + x2 * x * TOTAL;

    return (ipgui_anim_value_t)(n / (TOTAL * TOTAL * TOTAL));
}
