#include "ipgui_animation.h"
#include "ipgui_memory.h"
#include "ipgui_list.h"

__IPGUI_STATIC__ LIST_HEAD(anim_ready_list);
__IPGUI_STATIC__ LIST_HEAD(anim_running_list);
__IPGUI_STATIC__ LIST_HEAD(anim_paused_list);

typedef enum {
    IPGUI_ANIM_STATE_READY,
    IPGUI_ANIM_STATE_RUNNING,
    IPGUI_ANIM_STATE_PAUSED,
}ipgui_anim_state_t;

typedef struct {
    struct list_head node;
    ipgui_anim_dsc_t dsc;
    ipgui_anim_state_t state;
    ipgui_tick_t anim_start_tick; /* animation start tick */
    ipgui_tick_t duration;        /* t2 - t1 */
    char forward;                 /* 实时状态，1=正向(t1->t2), 0=反向(t2->t1) */
    u32_t loop_count_orig;        /* 原始loop_count，start时恢复 */
} ipgui_anim_t;

__IPGUI_STATIC__ __IPGUI_INLINE__ 
void anim_state_set(ipgui_anim_t * anim, ipgui_anim_state_t state)
{
    /* remove from old list */
    list_del(&anim->node);

    /* set state */
    anim->state = state;

    /* add to new list */
    switch (state) {
    case IPGUI_ANIM_STATE_READY:
        list_add(&anim->node, &anim_ready_list);
        break;
    case IPGUI_ANIM_STATE_RUNNING:
        list_add(&anim->node, &anim_running_list);
        break;
    case IPGUI_ANIM_STATE_PAUSED:
        list_add(&anim->node, &anim_paused_list);
        break;
    }
}

__IPGUI_STATIC__ void * anim_alloc(u32_t size)
{
    return ipgui_mem_alloc_def(size);
}

__IPGUI_STATIC__ void anim_free(void * ptr)
{
    ipgui_mem_free_def(ptr);
}

__IPGUI_API__ ipgui_anim_t * ipgui_anim_create(const ipgui_anim_dsc_t * dsc)
{
    /* check description's parmeter */
    if (!dsc) {
        return (ipgui_anim_t *)0;
    }
    if ((dsc->t2 < dsc->t1) || (!dsc->anim_func)) {
        return (ipgui_anim_t *)0;
    }

    ipgui_anim_t * anim = anim_alloc(sizeof(ipgui_anim_t));
    if (!anim) {
        return (ipgui_anim_t *)0;
    }
    anim->dsc = * dsc;
    anim->state = IPGUI_ANIM_STATE_READY;
    anim->duration = anim->dsc.t2 - anim->dsc.t1;
    anim->forward = (dsc->loop_type != IPGUI_ANIM_LOOP_TYPE_BACKWARD);
    anim->loop_count_orig = dsc->loop_count;
    list_add(&anim->node, &anim_ready_list);
    return anim;
}

__IPGUI_API__ ipgui_err_t ipgui_anim_start(ipgui_anim_t * anim)
{
    if (!anim) return;

    /* 检查状态 */
    switch (anim->state) {
        case IPGUI_ANIM_STATE_READY:
            anim->anim_start_tick = ipgui_tick_now() + anim->dsc.start_delay;
            anim->forward = (anim->dsc.loop_type != IPGUI_ANIM_LOOP_TYPE_BACKWARD);
            anim->dsc.loop_count = anim->loop_count_orig;
            anim_state_set(anim, IPGUI_ANIM_STATE_RUNNING);
            break;
        case IPGUI_ANIM_STATE_RUNNING:
            return IPGUI_ERR_ANIM_ALREADY_RUNNING;
        case IPGUI_ANIM_STATE_PAUSED:
            return IPGUI_ERR_ANIM_ALREADY_PAUSED;
    }
}

__IPGUI_API__ ipgui_err_t ipgui_anim_pause(ipgui_anim_t * anim)
{
    if (!anim) return;

    /* 检查状态 */
    switch (anim->state) {
        case IPGUI_ANIM_STATE_READY:    break;
        case IPGUI_ANIM_STATE_RUNNING:  break;
        case IPGUI_ANIM_STATE_PAUSED:   break;
    }
}

__IPGUI_API__ ipgui_err_t ipgui_anim_resume(ipgui_anim_t * anim)
{
    if (!anim) return;

    /* 检查状态 */
    switch (anim->state) {
        case IPGUI_ANIM_STATE_READY:    break;
        case IPGUI_ANIM_STATE_RUNNING:  break;
        case IPGUI_ANIM_STATE_PAUSED:   break;
    }
}

// __IPGUI_API__ ipgui_err_t ipgui_anim_restart(ipgui_anim_t * anim)
// {
//     /* stop and start animation */
//     ipgui_anim_stop(anim);
//     ipgui_anim_start(anim);
// }

__IPGUI_API__ ipgui_err_t ipgui_anim_stop(ipgui_anim_t * anim)
{
    if (!anim) return;

    /* 检查状态 */
    switch (anim->state) {
        case IPGUI_ANIM_STATE_READY:    break;
        case IPGUI_ANIM_STATE_RUNNING:  break;
        case IPGUI_ANIM_STATE_PAUSED:   break;
    }
}

__IPGUI_STATIC__ __IPGUI_INLINE__
void anim_cycle_restart(ipgui_anim_t * anim, ipgui_tick_t now)
{
    anim->anim_start_tick = now;
    anim_state_set(anim, IPGUI_ANIM_STATE_RUNNING);
}

__IPGUI_STATIC__ __IPGUI_INLINE__
void anim_cycle_end(ipgui_anim_t * anim, ipgui_tick_t now)
{
    if (anim->dsc.loop_count > 0) {
        anim->dsc.loop_count --;
        if (anim->dsc.loop_count == 0) {
            anim_state_set(anim, IPGUI_ANIM_STATE_READY);
            return;
        }
    }
    anim_cycle_restart(anim, now);
}

__IPGUI_API__ void ipgui_anim_update_all(void)
{
    ipgui_anim_t * anim;
    ipgui_tick_t now = ipgui_tick_now();
    ipgui_tick_t passed;
    ipgui_anim_value_t value;

    struct list_head * pos, * head;
    list_for_each_safe(pos, head, &anim_running_list) {
        anim = ipgui_container_of(pos, ipgui_anim_t, node);

        /* still in delay time [start_time, start_time + start_delay] 
         * start time is the time when animation start
         */
        if (now < anim->anim_start_tick) {
            continue;
        }

        /* calc passed time */
        passed = now - anim->anim_start_tick;
        passed = IPGUI_MIN(passed, anim->duration);

        /* calc animation current value */
        if (anim->forward) {
            value = anim->dsc.anim_func(anim->dsc.t1 + passed);
        } else {
            value = anim->dsc.anim_func(anim->dsc.t2 - passed);
        }

        /* one cycle not end */
        if (passed < anim->duration) {
            continue;
        }

        /* one cycle end */
        switch (anim->dsc.loop_type) {
        case IPGUI_ANIM_LOOP_TYPE_FORWARD:
        case IPGUI_ANIM_LOOP_TYPE_BACKWARD:
            anim_cycle_end(anim, now);
            break;

        case IPGUI_ANIM_LOOP_TYPE_PING_PONG:
            if (anim->forward) {
                /* forward half ended, flip backward & restart */
                anim->forward = 0;
                anim_cycle_restart(anim, now);
            } else {
                /* backward half ended, flip forward & count one full round-trip */
                anim->forward = 1;
                anim_cycle_end(anim, now);
            }
            break;
        }
    }
}