#include "ipgui_input_dispatcher.h"
#include "ipgui_memory.h"
#include "ipgui_debug.h"

static void ipgui_default_event_converter(
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
    dispatcher->convert_event_cb = ipgui_default_event_converter;
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
    
    dispatcher->scr_node_arr[id].scr = * screen;
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
            list_del_init(&dispatcher->map_arr[i].node);
            dispatcher->map_arr[i].used = 0;
            dispatcher->map_arr[i].conv_state_idx = 0;
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
            list_del_init(&dispatcher->map_arr[i].node);
            dispatcher->map_arr[i].used = 0;
            dispatcher->map_arr[i].conv_state_idx = 0;
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
    

    for (s32_t i = 0; i < INPUT_SRC_MAX * SCREEN_MAX; i ++) {
        if (dispatcher->map_arr[i].used == 0) {
            dispatcher->map_arr[i].input_src_id = input_src_id;
            dispatcher->map_arr[i].scr_id = screen_id;
            dispatcher->map_arr[i].used = 1;
            list_head_init(&dispatcher->map_arr[i].node);
            list_add_tail (&dispatcher->map_arr[i].node, &dispatcher->input_src_node_arr[input_src_id].map_list);
            dispatcher->map_arr[i].conv_state_idx = i;
            return IPGUI_ERR_OK;
        }
    }

    return IPGUI_ERR_NOK;
}

typedef struct {
    ipgui_input_dispatcher_t * dispatcher;
    u32_t conv_state_idx;
    ipgui_scr_id_t scr_id;
} param_t;

/* 分发所有事件，更新UI状态 */
__IPGUI_API__ void ipgui_dispatch_input_event(
    ipgui_input_dispatcher_t * dispatcher)
{
    if (!dispatcher) return;

    ipgui_input_src_evt_t ev;
    ipgui_widget_evt_t widget_evt;
    u32_t idx;
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
            if (input_src_node->input_src.convert_event_cb)
            { /* step1 : use user's convert callback function */    
                input_src_node->input_src.convert_event_cb(
                        input_src_node->input_src.priv_data,
                        &ev,
                        &widget_evt);
            } else
            {   /* step 2 : use default convert function */
                param_t param;
                param.dispatcher = dispatcher;
                param.conv_state_idx = map_node->conv_state_idx;
                param.scr_id = map_node->scr_id;
                dispatcher->convert_event_cb(
                    (void *)&param,
                    &ev,
                    &widget_evt);
            }
        
            /* handle UI event */
            ipgui_screen_handle_widget_event(&scr_node->scr, &widget_evt);
        }
    }
}
extern __IPGUI_API__ ipgui_widget_t * ipgui_widget_get_topest_at(
    struct widget_link_t * root, 
    ipgui_coord_t x, 
    ipgui_coord_t y);

__IPGUI_STATIC__ void ipgui_default_event_converter(
    void * param,
    ipgui_input_src_evt_t * raw_evt,
    ipgui_widget_evt_t * widget_evt)
{
    param_t * p = (param_t *)param;

    /* get screen */
    ipgui_scr_t * screen = &(p->dispatcher->scr_node_arr[p->scr_id].scr);

    /* 获取当前输入源的状态 */
    converter_state_t * cur_state = &(p->dispatcher->converter_states[p->conv_state_idx]);

    switch (raw_evt->input_src_evt)
    {
        /* pointer pressed */
        case IPGUI_INPUT_SRC_EVENT_POINTER_PRESS:
        {
            ipgui_coord_t x = raw_evt->evt_info.pointer_pos.x;
            ipgui_coord_t y = raw_evt->evt_info.pointer_pos.y;
            ipgui_widget_t * target;

            /* hit test */
            if(cur_state->last_state == IPGUI_INPUT_SRC_EVENT_POINTER_PRESS) {
                target = cur_state->grabbed;
            } else {
                target = ipgui_widget_get_topest_at(&screen->tree.root, x, y);
                if (!target) {
                    ipgui_dbg_error("hit test failed\r\n");
                    return;
                }
                cur_state->grabbed = target;
            }

            cur_state->last_pressed_x = x;
            cur_state->last_pressed_y = y;
            cur_state->last_state = IPGUI_INPUT_SRC_EVENT_POINTER_PRESS;
            return;
        }

        /* pointer released */
        case IPGUI_INPUT_SRC_EVENT_POINTER_RELEASE: 
        {
            /* handler */

            cur_state->last_state = IPGUI_INPUT_SRC_EVENT_POINTER_RELEASE;
            return;
        }

        case IPGUI_INPUT_SRC_EVENT_KEY_DOWN: 
        case IPGUI_INPUT_SRC_EVENT_KEY_UP: 
            return;
        default:
            return;
    }
}

//测试代码，通过
// int thread1(void) {

//     // 1. 初始化输入分发器
//     ipgui_input_dispatcher_t dispatcher;
//     ipgui_input_dispatcher_init(&dispatcher);

//     // 2. 注册输入源和屏幕
//     s32_t touch_id = ipgui_dispatcher_register_input_src(&dispatcher, &touch_src);
//     s32_t key_id = ipgui_dispatcher_register_input_src(&dispatcher, &key_src);
//     s32_t main_scr_id = ipgui_dispatcher_register_screen(&dispatcher, &main_screen);

//     // 3. 绑定映射
//     ipgui_bind_input_src_with_screen(&dispatcher, touch_id, main_scr_id);
//     ipgui_bind_input_src_with_screen(&dispatcher, key_id, main_scr_id);

//     // 4. 主循环
//     while (1) {
//         // 第一步：轮询所有输入设备，生成事件入队
//         ipgui_input_poll_devices(&dispatcher);
        
//         // 第二步：分发所有事件，更新UI状态
//         ipgui_dispatch_input_event(&dispatcher);
        
//         // 第三步：重绘脏区
//         ipgui_render();
        
//         // 第四步：空闲延时（可选，省电）
//         thread_delay(10);
//     }
// }