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

typedef struct {
    // 指针设备状态
    ipgui_input_evt_type_t last_state;
    ipgui_widget_t* grabbed;     /* 当前抓取的控件（按下后锁定，直到释放） */
    ipgui_coord_t last_pressed_x;               /* 上次按下的全局X坐标 */
    ipgui_coord_t last_pressed_y;               /* 上次按下的全局Y坐标 */
    

    // 按键设备状态
    u32_t key_press_start_time;         // 按键按下开始时间戳
    bool key_long_press_triggered;      // 按键长按是否已经触发
} converter_state_t;

typedef struct {
    struct list_head       map_list;
    struct list_head       node;
    ipgui_input_src_t      input_src;
}ipgui_input_src_node_t;

typedef struct {
    struct list_head       node;
    ipgui_scr_t            scr;
}ipgui_scr_node_t;

typedef struct {
    struct list_head       node;
    u32_t                  input_src_id;
    u32_t                  scr_id;
    u8_t                   used;
    u8_t                   conv_state_idx;
}map_node_t;

typedef struct {
    /* the list to link input sources and screens */
    struct list_head       input_src_list;
    struct list_head       screen_list;

    /* bitmap for allocate input source id and screen id
     * it is used to manage input source and screen
     */
    ipgui_input_src_node_t input_src_node_arr[INPUT_SRC_MAX];
    u32_t                  input_src_bmp[(INPUT_SRC_MAX + 31) >> 5];
    u32_t                  input_src_bmp_iter_max;
    u32_t                  input_src_bmp_last_mask;

    ipgui_scr_node_t       scr_node_arr[SCREEN_MAX];
    u32_t                  scr_bmp[(SCREEN_MAX + 31) >> 5];
    u32_t                  scr_bmp_iter_max;
    u32_t                  scr_bmp_last_mask;

    map_node_t             map_arr[INPUT_SRC_MAX * SCREEN_MAX];

    /* queue for input event */
    ipgui_norm_queue_t     evt_queue;
    ipgui_input_src_evt_t  input_evt_pool[EVENT_POOL_SIZE];

    /* input event convert default callback and default input source states */
    convert_event_cb_t     convert_event_cb;
    converter_state_t      converter_states[INPUT_SRC_MAX * SCREEN_MAX];
}ipgui_input_dispatcher_t;

extern __IPGUI_API__ void ipgui_input_dispatcher_init(ipgui_input_dispatcher_t * dispatcher);
extern __IPGUI_API__ ipgui_input_src_id_t ipgui_dispatcher_register_input_src(ipgui_input_dispatcher_t * dispatcher, ipgui_input_src_t * input_src);
extern __IPGUI_API__ ipgui_scr_id_t ipgui_dispatcher_register_screen(ipgui_input_dispatcher_t * dispatcher, ipgui_scr_t * screen);
extern __IPGUI_API__ ipgui_err_t ipgui_bind_input_src_with_screen(ipgui_input_dispatcher_t * dispatcher, ipgui_input_src_id_t input_src_id, ipgui_scr_id_t screen_id);
extern __IPGUI_API__ void ipgui_dispatch_input_event(ipgui_input_dispatcher_t * dispatcher);

#endif