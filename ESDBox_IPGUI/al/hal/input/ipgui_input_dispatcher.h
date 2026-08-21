#ifndef IPGUI_INPUT_DISPATCHER_H
#define IPGUI_INPUT_DISPATCHER_H

#include "ipgui_input_src.h"
#include "ipgui_input_src_event.h"
#include "ipgui_screen.h"
#include "ipgui_norm_queue.h"
#include "ipgui_utils.h"
#include "ipgui_list.h"
#include "ipgui_conf.h"
#include "ipgui_widget.h"

/* 中间状态 */
typedef struct {
    /* 通用中间状态 */
    ipgui_tick_t           last_tick;                /* 上次产生事件的系统时基 */

    /* 指针事件中间状态 */
    ipgui_input_evt_type_t last_state;
    ipgui_input_evt_type_t current_state;
    ipgui_widget_t       * grabbed;     /* 当前抓取的控件（按下后锁定，直到鼠标释放才置0） */
    ipgui_coord_t          first_pressed_x;              /* 本次按压起点全局X坐标（PRESS释放前不动） */
    ipgui_coord_t          first_pressed_y;              /* 本次按压起点全局Y坐标 */
    ipgui_coord_t          last_pressed_x;               /* 上次按下的全局X坐标 */
    ipgui_coord_t          last_pressed_y;               /* 上次按下的全局Y坐标 */
    ipgui_coord_t          cur_x;                /* 当前指针全局X坐标 */
    ipgui_coord_t          cur_y;                /* 当前指针全局Y坐标 */

    /* 按键设备状态 */
    u32_t                  key_press_start_time;         // 按键按下开始时间戳
    u8_t                   key_long_press_triggered;      // 按键长按是否已经触发
} converter_state_t;

typedef struct {
    struct list_head       map_list;
    struct list_head       node;
    ipgui_input_src_t      input_src;
}ipgui_input_src_node_t;

typedef struct {
    struct list_head       node;
    ipgui_scr_t          * scr;
}ipgui_scr_node_t;

typedef struct {
    struct list_head       node;
    u32_t                  input_src_id;
    u32_t                  scr_id;
    u8_t                   used;
}map_node_t;

typedef struct {
    /* the list to link input sources and screens */
    struct list_head       input_src_list;
    struct list_head       screen_list;

    /*
     * bitmap 分配器：每个 u32 存 32 个 ID 的状态 (1=空闲, 0=已分配)
     * bmp 末字超出 INPUT_SRC_MAX/SCREEN_MAX 的未用 bit 在 init 时被置 0，
     * alloc 时无需特殊分支——generic_ffs 天然不会检索到 0 bit。
     */
    ipgui_input_src_node_t input_src_node_arr[INPUT_SRC_MAX];
    u32_t                  input_src_bmp[(INPUT_SRC_MAX + 31) >> 5];
    u32_t                  input_src_bmp_iter_max;

    ipgui_scr_node_t       scr_node_arr[SCREEN_MAX];
    u32_t                  scr_bmp[(SCREEN_MAX + 31) >> 5];
    u32_t                  scr_bmp_iter_max;

    map_node_t             map_arr[INPUT_SRC_MAX * SCREEN_MAX];

    /* queue for input event */
    ipgui_norm_queue_t     evt_queue;
    ipgui_input_src_evt_t  input_evt_pool[EVENT_POOL_SIZE];

    /* input event convert default callback and default input source states */
    convert_event_cb_t     convert_event_cb[INPUT_SRC_MAX][SCREEN_MAX];
    converter_state_t      converter_states[INPUT_SRC_MAX][SCREEN_MAX];
}ipgui_input_dispatcher_t;

extern __IPGUI_API__ void ipgui_input_dispatcher_init(ipgui_input_dispatcher_t * dispatcher);
extern __IPGUI_API__ ipgui_input_src_id_t ipgui_dispatcher_register_input_src(ipgui_input_dispatcher_t * dispatcher, ipgui_input_src_t * input_src);
extern __IPGUI_API__ ipgui_input_src_id_t ipgui_dispatcher_unregister_input_src(ipgui_input_dispatcher_t * dispatcher, ipgui_input_src_id_t input_src_id);
extern __IPGUI_API__ ipgui_scr_id_t ipgui_dispatcher_register_screen(ipgui_input_dispatcher_t * dispatcher, ipgui_scr_t * screen);
extern __IPGUI_API__ ipgui_scr_id_t ipgui_dispatcher_unregister_screen(ipgui_input_dispatcher_t * dispatcher, ipgui_scr_id_t screen_id);
extern __IPGUI_API__ ipgui_err_t ipgui_bind_input_src_with_screen(ipgui_input_dispatcher_t * dispatcher, ipgui_input_src_id_t input_src_id, ipgui_scr_id_t screen_id);
extern __IPGUI_API__ void ipgui_dispatch_input_event(ipgui_input_dispatcher_t * dispatcher);

#endif