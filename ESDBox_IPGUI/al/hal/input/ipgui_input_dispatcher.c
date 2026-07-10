#include "ipgui_input_dispatcher.h"
#include "ipgui_memory.h"
#include "ipgui_debug.h"

static ipgui_err_t ipgui_default_event_converter(
    void * param,
    ipgui_input_src_evt_t * raw_evt,
    ipgui_widget_evt_t * widget_evt);

/* find lowest bit set(1 ~ 32) */
__IPGUI_STATIC__ __IPGUI_INLINE__ u32_t generic_ffs(u32_t v)
{
    u32_t ret = 1;
    if (!v) return 0;
    if (!(v & 0x0000ffffu)) { ret += 16; v >>= 16; }
    if (!(v & 0x000000ffu)) { ret += 8;  v >>= 8;  }
    if (!(v & 0x0000000fu)) { ret += 4;  v >>= 4;  }
    if (!(v & 0x00000003u)) { ret += 2;  v >>= 2;  }
    if (!(v & 0x00000001u)) { ret += 1;  v >>= 1;  }
    return ret;
}

/*
 * bitmap 末字掩码：当 INPUT_SRC_MAX / SCREEN_MAX 不是 32 的整数倍时，
 * 末字中仅有低 N 位有效，其余位在 init 时被置 0。
 * alloc 函数中 generic_ffs 天然不会检索到 0 bit，无需运行时分支。
 */
__IPGUI_STATIC__ __IPGUI_INLINE__ u32_t bmp_last_mask(u32_t max)
{
    u32_t rem = max & 31;
    return rem ? ((1u << rem) - 1) : ~0U;
}

__IPGUI_STATIC__ ipgui_input_src_id_t ipgui_alloc_id_for_input_src(
    ipgui_input_dispatcher_t * dispatcher)
{
    u32_t bit, temp;
    u32_t iter = 0, id = 0;

    if (!dispatcher) return -1;

    for (; iter < dispatcher->input_src_bmp_iter_max; ++ iter) {
        temp = dispatcher->input_src_bmp[iter];
        bit = generic_ffs(temp);
        if (bit) {
            dispatcher->input_src_bmp[iter] &= ~(1u << (bit - 1));
            id = (iter << 5) + bit;
            break;
        }
    }

    return id ? (id - 1) : -1;
}

__IPGUI_STATIC__ ipgui_scr_id_t ipgui_alloc_id_for_screen(
    ipgui_input_dispatcher_t * dispatcher)
{
    u32_t bit, temp;
    u32_t iter = 0, id = 0;

    if (!dispatcher) return -1;

    for (; iter < dispatcher->scr_bmp_iter_max; ++ iter) {
        temp = dispatcher->scr_bmp[iter];
        bit = generic_ffs(temp);
        if (bit) {
            dispatcher->scr_bmp[iter] &= ~(1u << (bit - 1));
            id = (iter << 5) + bit;
            break;
        }
    }

    return id ? (id - 1) : -1;
}

__IPGUI_API__ void ipgui_input_dispatcher_init(ipgui_input_dispatcher_t * dispatcher)
{
    ipgui_memset(dispatcher, 0, sizeof(ipgui_input_dispatcher_t));

    s32_t i;
    u32_t last_idx;

    /* ---- init input src bitmap ---- */
    dispatcher->input_src_bmp_iter_max = IPGUI_ARRAY_LEN(dispatcher->input_src_bmp);
    for (i = 0; i < dispatcher->input_src_bmp_iter_max; i ++) {
        dispatcher->input_src_bmp[i] = ~0U;
    }
    /* 末字未用 bit 置 0：generic_ffs 只检索有效 bit */
    last_idx = dispatcher->input_src_bmp_iter_max - 1;
    dispatcher->input_src_bmp[last_idx] &= bmp_last_mask(INPUT_SRC_MAX);

    /* ---- init screen bitmap ---- */
    dispatcher->scr_bmp_iter_max = IPGUI_ARRAY_LEN(dispatcher->scr_bmp);
    for (i = 0; i < dispatcher->scr_bmp_iter_max; i ++) {
        dispatcher->scr_bmp[i] = ~0U;
    }
    last_idx = dispatcher->scr_bmp_iter_max - 1;
    dispatcher->scr_bmp[last_idx] &= bmp_last_mask(SCREEN_MAX);

    /* init input source event queue */
    ipgui_norm_queue_init(
        &dispatcher->evt_queue,
        dispatcher->input_evt_pool,
        IPGUI_ARRAY_LEN(dispatcher->input_evt_pool),
        sizeof(ipgui_input_src_evt_t));
    
    list_head_init(&(dispatcher->input_src_list));
    list_head_init(&(dispatcher->screen_list));

    /* set default event convert function */
    for (i = 0; i < INPUT_SRC_MAX; i ++) {
        for (s32_t j = 0; j < SCREEN_MAX; j ++) {
            dispatcher->convert_event_cb[i][j] = ipgui_default_event_converter;
        }
    }
}

__IPGUI_API__ ipgui_input_src_id_t ipgui_dispatcher_register_input_src(
    ipgui_input_dispatcher_t * dispatcher,
    ipgui_input_src_t * input_src)
{
    /* alloc id for input source */
    ipgui_input_src_id_t id = ipgui_alloc_id_for_input_src(dispatcher);
    if (id == -1) {
        return - 1;
    }
    
    /* add to input source list of dispatcher */
    dispatcher->input_src_node_arr[id].input_src = * input_src;
    list_head_init(&dispatcher->input_src_node_arr[id].node);
    list_add_tail (&dispatcher->input_src_node_arr[id].node, &dispatcher->input_src_list);

    /* init map list */
    list_head_init(&dispatcher->input_src_node_arr[id].map_list);

    return id;
}

__IPGUI_API__ ipgui_scr_id_t ipgui_dispatcher_register_screen(
    ipgui_input_dispatcher_t * dispatcher,
    ipgui_scr_t * screen)
{
    /* alloc id for screen */
    ipgui_scr_id_t id = ipgui_alloc_id_for_screen(dispatcher);
    if (id == -1) {
        return - 1;
    }
    
    dispatcher->scr_node_arr[id].scr = screen;
    list_head_init(&dispatcher->scr_node_arr[id].node);
    list_add_tail (&dispatcher->scr_node_arr[id].node, &dispatcher->screen_list);
    return id;
}

/*
 * 注销输入源：
 *   1. 校验 id 有效性
 *   2. 清除与该输入源关联的所有 screen 绑定
 *   3. 从 input_src_list 中摘除
 *   4. 归还 bitmap bit
 */
__IPGUI_API__ ipgui_input_src_id_t ipgui_dispatcher_unregister_input_src(
    ipgui_input_dispatcher_t * dispatcher,
    ipgui_input_src_id_t input_src_id)
{
    s32_t i;
    u32_t word_idx, bit_idx;

    if (!dispatcher || input_src_id < 0 || input_src_id >= INPUT_SRC_MAX)
        return -1;

    /* 清除所有关联到该 input_src 的 map_arr 绑定 */
    for (i = 0; i < INPUT_SRC_MAX * SCREEN_MAX; i ++) {
        if (dispatcher->map_arr[i].used &&
            dispatcher->map_arr[i].input_src_id == (u32_t)input_src_id) {
            ipgui_memset(&dispatcher->converter_states[dispatcher->map_arr[i].input_src_id][dispatcher->map_arr[i].scr_id], 0, sizeof(converter_state_t));
            list_del_init(&dispatcher->map_arr[i].node);
            dispatcher->map_arr[i].used = 0;
        }
    }

    /* 从输入源链表中摘除 */
    list_del_init(&dispatcher->input_src_node_arr[input_src_id].node);

    /* 归还 bitmap bit */
    word_idx = (u32_t)input_src_id >> 5;
    bit_idx  = (u32_t)input_src_id & 31;
    dispatcher->input_src_bmp[word_idx] |= (1u << bit_idx);

    return input_src_id;
}

/*
 * 注销屏幕：
 *   1. 校验 id 有效性
 *   2. 清除与该屏幕关联的所有 input_src 绑定
 *   3. 从 screen_list 中摘除
 *   4. 归还 bitmap bit
 */
__IPGUI_API__ ipgui_scr_id_t ipgui_dispatcher_unregister_screen(
    ipgui_input_dispatcher_t * dispatcher,
    ipgui_scr_id_t screen_id)
{
    s32_t i;
    u32_t word_idx, bit_idx;

    if (!dispatcher || screen_id < 0 || screen_id >= SCREEN_MAX)
        return -1;

    /* 清除所有关联到该 screen 的 map_arr 绑定 */
    for (i = 0; i < INPUT_SRC_MAX * SCREEN_MAX; i ++) {
        if (dispatcher->map_arr[i].used &&
            dispatcher->map_arr[i].scr_id == (u32_t)screen_id) {
            ipgui_memset(&dispatcher->converter_states[dispatcher->map_arr[i].input_src_id][dispatcher->map_arr[i].scr_id], 0, sizeof(converter_state_t));
            list_del_init(&dispatcher->map_arr[i].node);
            dispatcher->map_arr[i].used = 0;
        }
    }

    /* 从屏幕链表中摘除 */
    list_del_init(&dispatcher->scr_node_arr[screen_id].node);

    /* 归还 bitmap bit */
    word_idx = (u32_t)screen_id >> 5;
    bit_idx  = (u32_t)screen_id & 31;
    dispatcher->scr_bmp[word_idx] |= (1u << bit_idx);

    return screen_id;
}

__IPGUI_API__ ipgui_err_t ipgui_bind_input_src_with_screen(
    ipgui_input_dispatcher_t * dispatcher,
    ipgui_input_src_id_t input_src_id,
    ipgui_scr_id_t screen_id)
{
    if (input_src_id < 0 || input_src_id >= INPUT_SRC_MAX)
        return IPGUI_ERR_INVALID_ID;
    if (screen_id < 0 || screen_id >= SCREEN_MAX)
        return IPGUI_ERR_INVALID_ID;

    /* 检查这两者的映射表是否已经存在 */
    for (s32_t i = 0; i < INPUT_SRC_MAX * SCREEN_MAX; i++) {
        if (dispatcher->map_arr[i].used &&
            dispatcher->map_arr[i].input_src_id == (u32_t)input_src_id &&
            dispatcher->map_arr[i].scr_id == (u32_t)screen_id) {
            return IPGUI_ERR_OK;  /* 已存在，不重复绑定 */
        }
    }

    for (s32_t i = 0; i < INPUT_SRC_MAX * SCREEN_MAX; i ++) {
        if (dispatcher->map_arr[i].used == 0) {
            dispatcher->map_arr[i].input_src_id = input_src_id;
            dispatcher->map_arr[i].scr_id = screen_id;
            dispatcher->map_arr[i].used = 1;
            list_head_init(&dispatcher->map_arr[i].node);
            list_add_tail (&dispatcher->map_arr[i].node, &dispatcher->input_src_node_arr[input_src_id].map_list);
            return IPGUI_ERR_OK;
        }
    }

    return IPGUI_ERR_NOK;
}

typedef struct {
    ipgui_input_dispatcher_t * dispatcher;
    map_node_t * map_node;
} param_t;

/* 分发所有事件，更新UI状态 */
__IPGUI_API__ void ipgui_dispatch_input_event(
    ipgui_input_dispatcher_t * dispatcher)
{
    if (!dispatcher) return;

    ipgui_input_src_evt_t ev;
    ipgui_widget_evt_t widget_evt;
    struct list_head * pos, * tmp;
    map_node_t * map_node;
    ipgui_scr_node_t * scr_node;
    ipgui_input_src_node_t * input_src_node;
    while (IPGUI_ERR_OK == ipgui_norm_queue_fetch(&dispatcher->evt_queue, &ev))
    {
        /* check input source id */
        if (ev.input_src_id < 0 || ev.input_src_id >= INPUT_SRC_MAX) {
            continue;
        }

        input_src_node = &(dispatcher->input_src_node_arr[ev.input_src_id]);
        list_for_each_safe(pos, tmp, &input_src_node->map_list)
        {
            map_node = list_entry(pos, map_node_t, node);
            scr_node = &(dispatcher->scr_node_arr[map_node->scr_id]);

            /* input source event to UI event */
            ipgui_memset(&widget_evt, 0, sizeof(widget_evt));

            if (!dispatcher->convert_event_cb[map_node->input_src_id][map_node->scr_id])
                continue;
            
            /* use convert function */
            param_t param;
            param.dispatcher = dispatcher;
            param.map_node = map_node;
            if(IPGUI_ERR_OK != dispatcher->convert_event_cb[map_node->input_src_id][map_node->scr_id](
                (void *)&param,
                &ev,
                &widget_evt)) {
                /* 没有控件可以处理事件，比如点在了屏幕没有被控件覆盖的空旷区域上 */
                continue;
            }
        
            /* handle UI event */
            ipgui_screen_handle_widget_event(scr_node->scr, &widget_evt);
        }
    }
}
extern __IPGUI_API__ ipgui_widget_t * ipgui_screen_point_on(
    ipgui_scr_t  * scr,
    ipgui_coord_t  x,
    ipgui_coord_t  y);

/* ok:有控件可以处理事件 nok:没有控件可以处理事件，比如点在了屏幕背景上 */
__IPGUI_STATIC__ ipgui_err_t ipgui_default_event_converter(
    void * param,
    ipgui_input_src_evt_t * raw_evt,
    ipgui_widget_evt_t * widget_evt)
{
    param_t * p = (param_t *)param;

    /* 获取当前屏幕 */
    ipgui_scr_t * screen = p->dispatcher->scr_node_arr[p->map_node->scr_id].scr;    
    /* 获取当前输入源的状态 */
    converter_state_t * cur_state = &(p->dispatcher->converter_states[p->map_node->input_src_id][p->map_node->scr_id]);
    
    switch (raw_evt->input_src_evt)
    {
        /* pointer pressed */
        case IPGUI_INPUT_SRC_EVENT_POINTER_PRESS:
        {
            ipgui_aabb_t widget_global_aabb;

            ipgui_widget_t * target;

            /* 更新当前状态 */
            cur_state->last_state = cur_state->current_state;
            cur_state->current_state = IPGUI_INPUT_SRC_EVENT_POINTER_PRESS;

            /* hit test */
            if(cur_state->last_state == IPGUI_INPUT_SRC_EVENT_POINTER_PRESS) {
                cur_state->last_pressed_x = cur_state->cur_x;
                cur_state->last_pressed_y = cur_state->cur_y;
                cur_state->cur_x = raw_evt->evt_info.pointer_pos.x;
                cur_state->cur_y = raw_evt->evt_info.pointer_pos.y;

                target = cur_state->grabbed;

                /* 后处理，主要是填充事件和坐标转化，转化为控件的相对坐标 */
                widget_evt->target = cur_state->grabbed;
                widget_evt->type = IPGUI_WIDGET_EVENT_PRESSED;
                ipgui_widget_abs_pos(widget_evt->target, &widget_global_aabb);
                widget_evt->evt.pressed_evt.x = cur_state->cur_x - widget_global_aabb.start.x;
                widget_evt->evt.pressed_evt.y = cur_state->cur_y - widget_global_aabb.start.y;
                widget_evt->evt.pressed_evt.first_press_x = cur_state->first_pressed_x - widget_global_aabb.start.x;
                widget_evt->evt.pressed_evt.first_press_y = cur_state->first_pressed_y - widget_global_aabb.start.y;
                widget_evt->evt.pressed_evt.last_press_x = cur_state->last_pressed_x - widget_global_aabb.start.x;
                widget_evt->evt.pressed_evt.last_press_y = cur_state->last_pressed_y - widget_global_aabb.start.y;
            } else {
                cur_state->cur_x = raw_evt->evt_info.pointer_pos.x;
                cur_state->cur_y = raw_evt->evt_info.pointer_pos.y;
                cur_state->first_pressed_x = cur_state->cur_x;
                cur_state->first_pressed_y = cur_state->cur_y;
                cur_state->last_pressed_x = cur_state->cur_x;
                cur_state->last_pressed_y = cur_state->cur_y;
                target = ipgui_screen_point_on(
                    screen,
                    cur_state->cur_x,
                    cur_state->cur_y);

                if (!target) {
                    ipgui_dbg_error("hit test failed\r\n");
                    return IPGUI_ERR_NOK;
                }
                cur_state->grabbed = target;

                /* 后处理，主要是填充事件和坐标转化，转化为控件的相对坐标 */
                widget_evt->target = cur_state->grabbed;
                widget_evt->type = IPGUI_WIDGET_EVENT_PRESSED;
                ipgui_widget_abs_pos(widget_evt->target, &widget_global_aabb);
                widget_evt->evt.pressed_evt.x = cur_state->cur_x - widget_global_aabb.start.x;
                widget_evt->evt.pressed_evt.y = cur_state->cur_y - widget_global_aabb.start.y;
                widget_evt->evt.pressed_evt.first_press_x = cur_state->first_pressed_x - widget_global_aabb.start.x;
                widget_evt->evt.pressed_evt.first_press_y = cur_state->first_pressed_y - widget_global_aabb.start.y;
                widget_evt->evt.pressed_evt.last_press_x = cur_state->first_pressed_x - widget_global_aabb.start.x;
                widget_evt->evt.pressed_evt.last_press_y = cur_state->first_pressed_y - widget_global_aabb.start.y;
            }

            break;
        }

        /* pointer released */
        case IPGUI_INPUT_SRC_EVENT_POINTER_RELEASE: 
        {
            ipgui_aabb_t widget_global_aabb;
            u8_t hover_flag = 0;

            cur_state->last_state = cur_state->current_state;
            if (cur_state->last_state != IPGUI_INPUT_SRC_EVENT_POINTER_PRESS) {
                hover_flag = 1;
            }
            cur_state->current_state = IPGUI_INPUT_SRC_EVENT_POINTER_RELEASE;
            cur_state->cur_x = raw_evt->evt_info.pointer_pos.x;
            cur_state->cur_y = raw_evt->evt_info.pointer_pos.y;
            
            /* 后处理，主要是填充事件和坐标转化，转化为控件的相对坐标 */
            if (hover_flag == 1) {
                ipgui_widget_t * target = ipgui_screen_point_on(
                    screen,
                    raw_evt->evt_info.pointer_pos.x,
                    raw_evt->evt_info.pointer_pos.y);
                if (!target) {
                    ipgui_dbg_error("hit test failed, no widget hovered\r\n");
                    return IPGUI_ERR_NOK;
                }
                widget_evt->target = target;
                widget_evt->type = IPGUI_WIDGET_EVENT_HOVER;
                ipgui_widget_abs_pos(widget_evt->target, &widget_global_aabb);
                widget_evt->evt.hover_evt.x = cur_state->cur_x - widget_global_aabb.start.x;
                widget_evt->evt.hover_evt.y = cur_state->cur_y - widget_global_aabb.start.y;
            } else {
                widget_evt->target = cur_state->grabbed;
                widget_evt->type = IPGUI_WIDGET_EVENT_RELEASED;
                ipgui_widget_abs_pos(widget_evt->target, &widget_global_aabb);
                widget_evt->evt.released_evt.x = cur_state->cur_x - widget_global_aabb.start.x;
                widget_evt->evt.released_evt.y = cur_state->cur_y - widget_global_aabb.start.y;
                widget_evt->evt.released_evt.first_press_x = cur_state->first_pressed_x - widget_global_aabb.start.x;
                widget_evt->evt.released_evt.first_press_y = cur_state->first_pressed_y - widget_global_aabb.start.y;
                widget_evt->evt.released_evt.prev_press_x = cur_state->last_pressed_x - widget_global_aabb.start.x;
                widget_evt->evt.released_evt.prev_press_y = cur_state->last_pressed_y - widget_global_aabb.start.y;
            }
            cur_state->grabbed = (ipgui_widget_t *)0;/*  必须放在后处理后面，否则不知道在哪个控件中释放了 */
            break;
        }

        /* 暂时不支持pointer事件类型以外的事件 */
        case IPGUI_INPUT_SRC_EVENT_KEY_DOWN: 
        case IPGUI_INPUT_SRC_EVENT_KEY_UP: 
        default:
            return IPGUI_ERR_NOK;
    }

    return IPGUI_ERR_OK;
}