#ifndef IPGUI_INPUT_DISPATCHER_H
#define IPGUI_INPUT_DISPATCHER_H

#include "ipgui_input_src.h"
#include "ipgui_input_event.h"
#include "ipgui_screen.h"
#include "ipgui_norm_queue.h"
#include "ipgui_utils.h"
#include "ipgui_list.h"

typedef s32_t ipgui_input_src_id_t;
typedef s32_t ipgui_scr_id_t;

#define INPUT_SRC_MAX 2
#define SCREEN_MAX 2

typedef struct {
    struct list_head map_list;
    struct list_head node;
    ipgui_input_src_t input_src;
}ipgui_input_src_node_t;

typedef struct {
    struct list_head node;
    ipgui_scr_t scr;
}ipgui_scr_node_t;

typedef struct {
    struct list_head node;
    u32_t input_src_id;
    u32_t scr_id;
    u8_t used;
}map_node_t;

typedef struct {
    /* the list to link input sources and screens */
    struct list_head input_src_list;
    struct list_head screen_list;

    /* bitmap for allocate input source id and screen id
     * it is used to manage input source and screen
     */
    ipgui_input_src_node_t input_src_node_arr[INPUT_SRC_MAX];
    u32_t input_src_bmp[(INPUT_SRC_MAX + 31) >> 5];
    u32_t input_src_bmp_iter_max;
    u32_t input_src_bmp_last_mask;

    ipgui_scr_node_t scr_node_arr[SCREEN_MAX];
    u32_t scr_bmp[(SCREEN_MAX + 31) >> 5];
    u32_t scr_bmp_iter_max;
    u32_t scr_bmp_last_mask;

    map_node_t map_arr[INPUT_SRC_MAX * SCREEN_MAX];

    /* queue for input event */
    ipgui_norm_queue_t evt_queue;
    ipgui_input_evt_t  input_evt_pool[10];

}ipgui_input_dispatcher_t;

#endif