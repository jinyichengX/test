#ifndef IPGUI_ANIMATION_H
#define IPGUI_ANIMATION_H

#include "ipgui_time.h"

/* f(t), t ∈ [t1, t2], 纯函数无副作用，返回值由调用方解释 */
typedef ipgui_anim_value_t (* ipgui_anim_func_t)(ipgui_tick_t t);

typedef enum {
    IPGUI_ANIM_LOOP_TYPE_FORWARD = 0,   /* 正向t1->t2（默认） */
    IPGUI_ANIM_LOOP_TYPE_BACKWARD,      /* 反向t2->t1 */
    IPGUI_ANIM_LOOP_TYPE_PING_PONG,     /* 往返t1->t2->t1->... */
} ipgui_anim_loop_type_t;

typedef struct {
    /* 动画函数 */
    ipgui_anim_func_t      anim_func;
    ipgui_tick_t           t1, t2;

    /* 循环类型，默认正向FORWARD */
    ipgui_anim_loop_type_t loop_type;

    /* 循环次数，0=无限循环，1=播一次，N=播N次 */
    u32_t                  loop_count;

    /* 动画开始延迟时间 */
    ipgui_tick_t           start_delay;
} ipgui_anim_dsc_t;

#endif
