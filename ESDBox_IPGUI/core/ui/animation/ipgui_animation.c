#include "ipgui_animation.h"
#include "ipgui_memory.h"
#include "ipgui_list.h"

__IPGUI_STATIC__ LIST_HEAD(anim_ready_list);
__IPGUI_STATIC__ LIST_HEAD(anim_running_list);
__IPGUI_STATIC__ LIST_HEAD(anim_paused_list);
__IPGUI_STATIC__ LIST_HEAD(anim_stopped_list);

typedef enum {
    IPGUI_ANIM_STATE_READY,
    IPGUI_ANIM_STATE_RUNNING,
    IPGUI_ANIM_STATE_PAUSED,
    IPGUI_ANIM_STATE_STOPPED,
}ipgui_anim_state_t;

typedef struct {
    struct list_head node;
    ipgui_anim_dsc_t dsc;
    ipgui_anim_state_t state;
} ipgui_anim_t;

__IPGUI_STATIC__ __IPGUI_INLINE__ 
void set_anim_state(ipgui_anim_t * anim, ipgui_anim_state_t state)
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
    case IPGUI_ANIM_STATE_STOPPED:
        list_add(&anim->node, &anim_stopped_list);
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

__IPGUI_API__ ipgui_anim_t * ipgui_anim_create(const ipgui_anim_dsc_t * desc)
{
    ipgui_anim_t * anim = anim_alloc(sizeof(ipgui_anim_t));
    if (!anim) {
        return (ipgui_anim_t *)0;
    }
    anim->dsc = *desc;
    list_add(&anim->node, &anim_ready_list);
    return anim;
}

__IPGUI_API__ ipgui_err_t ipgui_anim_start(ipgui_anim_t * anim)
{
    if (!anim) return;

    /* 检查状态 */
    switch (anim->state) {
        case IPGUI_ANIM_STATE_READY:    break;
        case IPGUI_ANIM_STATE_RUNNING:  break;
        case IPGUI_ANIM_STATE_PAUSED:   break;
        case IPGUI_ANIM_STATE_STOPPED:  break;
    }
}

__IPGUI_API__ ipgui_err_t ipgui_anim_restart(ipgui_anim_t * anim)
{
    if (!anim) return;

    /* 检查状态 */
    switch (anim->state) {
        case IPGUI_ANIM_STATE_READY:    break;
        case IPGUI_ANIM_STATE_RUNNING:  break;
        case IPGUI_ANIM_STATE_PAUSED:   break;
        case IPGUI_ANIM_STATE_STOPPED:  break;
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
        case IPGUI_ANIM_STATE_STOPPED:  break;
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
        case IPGUI_ANIM_STATE_STOPPED:  break;
    }
}

__IPGUI_API__ ipgui_err_t ipgui_anim_stop(ipgui_anim_t * anim)
{
    if (!anim) return;

    /* 检查状态 */
    switch (anim->state) {
        case IPGUI_ANIM_STATE_READY:    break;
        case IPGUI_ANIM_STATE_RUNNING:  break;
        case IPGUI_ANIM_STATE_PAUSED:   break;
        case IPGUI_ANIM_STATE_STOPPED:  break;
    }
}

__IPGUI_API__ void ipgui_anim_update_all(void)
{
    struct list_head * pos, * head;
    list_for_each_safe(pos, head, &anim_running_list) {
        ipgui_anim_t * anim = container_of(pos, ipgui_anim_t, node);

        /* 更新动画 */
    }
}