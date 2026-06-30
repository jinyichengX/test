#include "ipgui_animation.h"
#include "ipgui_membox.h"
#include "ipgui_list.h"
#include "ipgui_conf.h"

__IPGUI_STATIC__ ipgui_membox_t * anim_box = 0;       /* 内存池 */

__IPGUI_STATIC__ LIST_HEAD(anim_ready_list);       /* 已创建、待启动 */
__IPGUI_STATIC__ LIST_HEAD(anim_running_list);     /* 运行中 */

enum {
    IPGUI_ANIM_ST_READY,
    IPGUI_ANIM_ST_RUNNING,
};

struct ipgui_anim_t {
    struct list_head   node;
    ipgui_anim_dsc_t   dsc;
    u8_t               state;       /* READY / RUNNING */
    ipgui_tick_t       duration;    /* t2 - t1 + 1 */
    ipgui_tick_t       start;       /* 启动时刻 + start_delay 锚点 */
};

__IPGUI_STATIC__ __IPGUI_INLINE__ void anim_pool_ensure(void)
{
    if (!anim_box) {
        anim_box = ipgui_membox_create(sizeof(ipgui_anim_t), IPGUI_ANIM_POOL_SIZE);
    }
}

__IPGUI_STATIC__ void anim_free_to_pool(ipgui_anim_t * anim)
{
    list_del(&anim->node);
    ipgui_membox_free(anim_box, anim);
}

__IPGUI_API__ ipgui_anim_t * ipgui_anim_create(const ipgui_anim_dsc_t * dsc)
{
    if (!dsc || !dsc->path_cb || !dsc->anim_func)
        return (ipgui_anim_t *)0;
    if (dsc->t2 <= dsc->t1)
        return (ipgui_anim_t *)0;

    anim_pool_ensure();
    if (!anim_box) return (ipgui_anim_t *)0;

    ipgui_anim_t * anim = (ipgui_anim_t *)ipgui_membox_alloc(anim_box);
    if (!anim) return (ipgui_anim_t *)0;

    anim->dsc      = *dsc;
    anim->state    = IPGUI_ANIM_ST_READY;
    anim->duration = anim->dsc.t2 - anim->dsc.t1 + 1;
    anim->start    = 0;

    list_head_init(&anim->node);
    list_add_tail(&anim->node, &anim_ready_list);
    return anim;
}

__IPGUI_API__ ipgui_err_t ipgui_anim_start(ipgui_anim_t * anim)
{
    if (!anim) return IPGUI_ERR_PARAM;
    if (anim->state != IPGUI_ANIM_ST_READY)
        return IPGUI_ERR_ANIM_ALREADY_RUNNING;

    list_del(&anim->node);
    anim->state = IPGUI_ANIM_ST_RUNNING;
    anim->start = ipgui_tick_now() + anim->dsc.start_delay;
    list_add_tail(&anim->node, &anim_running_list);

    /* 推初始值 */
    anim->dsc.path_cb(anim, anim->dsc.anim_func(anim->dsc.t1));

    return IPGUI_ERR_OK;
}

__IPGUI_API__ void ipgui_anim_update_all(void)
{
    ipgui_anim_t * anim;
    ipgui_tick_t now = ipgui_tick_now();

    struct list_head * pos, * head;
    list_for_each_safe(pos, head, &anim_running_list) {
        anim = ipgui_container_of(pos, ipgui_anim_t, node);

        /* 仍在延迟等待中 */
        if ((s32_t)(now - anim->start) < 0)
            continue;

        ipgui_tick_t elapsed = now - anim->start;

        if (IPGUI_ANIM_LOOP_DEFAULT == anim->dsc.loop_type) {
            ipgui_tick_t cycle  = elapsed / anim->duration;
            ipgui_tick_t pos_in = elapsed % anim->duration;

            /* 有限循环，已完成 */
            if (anim->dsc.loop_count > 0
                && cycle >= anim->dsc.loop_count) {
                /* 跳帧可能漏过终点帧，补推确保终值送达 */
                anim->dsc.path_cb(anim,
                    anim->dsc.anim_func(anim->dsc.t2));
                if (anim->dsc.finish_cb)
                    anim->dsc.finish_cb(anim, anim->dsc.user_data);
                anim_free_to_pool(anim);
                continue;
            }

            anim->dsc.path_cb(anim,
                anim->dsc.anim_func(anim->dsc.t1 + pos_in));

        } else {
            /* PING_PONG
             * 往返周期 = 2 * duration - 1，顶点 t2 不重复
             * 正向: pos ∈ [0, duration-1]   → t1+pos
             * 反向: pos ∈ [duration, period-1] → t2-(pos-duration+1)
             */
            ipgui_tick_t period = anim->duration * 2 - 1;
            ipgui_tick_t cycle  = elapsed / period;
            ipgui_tick_t pos    = elapsed % period;

            /* 有限循环，已完成 */
            if (anim->dsc.loop_count > 0
                && cycle >= anim->dsc.loop_count) {
                /* 乒乓完整往返终点 = t1，补推确保终值送达 */
                anim->dsc.path_cb(anim,
                    anim->dsc.anim_func(anim->dsc.t1));
                if (anim->dsc.finish_cb)
                    anim->dsc.finish_cb(anim, anim->dsc.user_data);
                anim_free_to_pool(anim);
                continue;
            }

            ipgui_anim_value_t v;
            if (pos < anim->duration) {
                v = anim->dsc.anim_func(anim->dsc.t1 + pos);
            } else {
                v = anim->dsc.anim_func(
                    anim->dsc.t2 - (pos - anim->duration + 1));
            }
            anim->dsc.path_cb(anim, v);
        }
    }
}
