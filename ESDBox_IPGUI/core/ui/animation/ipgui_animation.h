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
    ipgui_tick_t           t1, t2;      /* t1 t2不是绝对时间，是函数上的区间 */

    /* 循环类型，默认正向FORWARD */
    ipgui_anim_loop_type_t loop_type;

    /* 循环次数，0=无限循环，1=播一次，N=播N次 */
    u32_t                  loop_count;

    /* 动画开始延迟时间，只在第一次循环有效 */
    ipgui_tick_t           start_delay;

    /* 播完后自动销毁 */
    u8_t                   auto_destroy;
} ipgui_anim_dsc_t;

typedef struct ipgui_anim_t ipgui_anim_t;

__IPGUI_API__ ipgui_anim_t *     ipgui_anim_create(const ipgui_anim_dsc_t * dsc);
__IPGUI_API__ ipgui_err_t        ipgui_anim_start(ipgui_anim_t * anim);
__IPGUI_API__ ipgui_err_t        ipgui_anim_pause(ipgui_anim_t * anim);
__IPGUI_API__ ipgui_err_t        ipgui_anim_resume(ipgui_anim_t * anim);
__IPGUI_API__ ipgui_err_t        ipgui_anim_stop(ipgui_anim_t * anim);
__IPGUI_API__ void               ipgui_anim_destroy(ipgui_anim_t * anim);
__IPGUI_API__ ipgui_anim_value_t ipgui_anim_get_value(ipgui_anim_t * anim);
__IPGUI_API__ void               ipgui_anim_update_all(void);

#endif
