#include "ipgui_input_dispatcher.h"
#include "ipgui_memory.h"

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

__IPGUI_STATIC__ ipgui_input_src_id_t ipgui_alloc_id_for_input_src(
    ipgui_input_dispatcher_t * dispatcher)
{
    u32_t bit, temp;
    u32_t iter = 0, id = 0;

    if (!dispatcher) return -1;

    /* search for a free bit in bmp */
    for (; iter < dispatcher->input_src_bmp_iter_max; ++ iter) {
        if (iter < dispatcher->input_src_bmp_iter_max - 1)
            temp = dispatcher->input_src_bmp[iter] & (~0U);
        else
            temp = dispatcher->input_src_bmp[iter] & dispatcher->input_src_bmp_last_mask;
        bit = generic_ffs(temp);
        if (bit) {
            dispatcher->input_src_bmp[iter] &= ~(1 << (bit - 1));
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

    /* search for a free bit in bmp */
    for (; iter < dispatcher->scr_bmp_iter_max; ++ iter) {
        if (iter < dispatcher->scr_bmp_iter_max - 1)
            temp = dispatcher->scr_bmp[iter] & (~0U);
        else
            temp = dispatcher->scr_bmp[iter] & dispatcher->scr_bmp_last_mask;
        bit = generic_ffs(temp);
        if (bit) {
            dispatcher->scr_bmp[iter] &= ~(1 << (bit - 1));
            id = (iter << 5) + bit;
            break;
        }
    }

    return id ? (id - 1) : -1;
}

__IPGUI_API__ void ipgui_input_dispatcher_init(ipgui_input_dispatcher_t * dispatcher)
{
    /* init input src manager */
    u32_t i;
    for (i = 0; i < INPUT_SRC_MAX; i ++) {
        ipgui_memset(&dispatcher->input_src_node_arr[i], 0, sizeof(ipgui_input_src_node_t));
    }
    dispatcher->input_src_bmp_iter_max = IPGUI_ARRAY_LEN(dispatcher->input_src_bmp);

    for (i = 0; i < dispatcher->input_src_bmp_iter_max; i ++) {
        dispatcher->input_src_bmp[i] = ~0U;
    }

    dispatcher->input_src_bmp_last_mask = (~0U >> (32 - (INPUT_SRC_MAX - ((INPUT_SRC_MAX >> 5) << 5))));

    /* init screen manager */
    for (i = 0; i < SCREEN_MAX; i ++) {
        ipgui_memset(&dispatcher->scr_node_arr[i], 0, sizeof(ipgui_scr_node_t));
    }
    dispatcher->scr_bmp_iter_max = IPGUI_ARRAY_LEN(dispatcher->scr_bmp);

    for (i = 0; i < dispatcher->scr_bmp_iter_max; i ++) {
        dispatcher->scr_bmp[i] = ~0U;
    }

    dispatcher->scr_bmp_last_mask = (~0U >> (32 - (SCREEN_MAX - ((SCREEN_MAX >> 5) << 5))));

    /* init input event queue */
    ipgui_norm_queue_init(
        &dispatcher->evt_queue,
        dispatcher->input_evt_pool,
        IPGUI_ARRAY_LEN(dispatcher->input_evt_pool),
        sizeof(ipgui_input_evt_t));
    
    /* init map array */
    for (i = 0; i < INPUT_SRC_MAX * SCREEN_MAX; i ++) {
        ipgui_memset(&dispatcher->map_arr[i], 0, sizeof(map_node_t));
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
    
    dispatcher->scr_node_arr[id].scr = * screen;
    list_head_init(&dispatcher->scr_node_arr[id].node);
    list_add_tail (&dispatcher->scr_node_arr[id].node, &dispatcher->screen_list);
    return id;
}

__IPGUI_API__ ipgui_err_t ipgui_bind_input_src_with_screen(
    ipgui_input_dispatcher_t * dispatcher,
    s32_t input_src_id,
    s32_t screen_id)
{
    /* check if the map already exists */
    

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