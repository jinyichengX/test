#ifndef IPGUI_EVENT_H
#define IPGUI_EVENT_H

#include "ipgui_types.h"
#include "ipgui_utils.h"
#include "ipgui_list.h"
#include "ipgui_defs.h"
#include "ipgui_event_defs.h"
#include "ipgui_timer.h"
#include "ipgui_coord.h"

IPGUI_HEADER_BEGIN _______________MARKER_______________

typedef struct ipgui_input_handle_st ipgui_input_handle_t;
typedef struct ipgui_input_handler_st ipgui_input_handler_t;
/* event unit */
typedef struct{
    unsigned int type;
    unsigned int code;
    unsigned int value;
}ipgui_input_event_t;

typedef struct{
    unsigned char key_btn_last_pressed : 1;
    unsigned char key_btn_pressed : 1;
    unsigned char key_code_last_pressed : 1;
    unsigned char key_code_pressed : 1;
    unsigned char reserved : 3;
    ipgui_coord_t x;
    ipgui_coord_t y;
    unsigned int pressure;
    unsigned char key_code;
    ipgui_tick_t timestamp;
}ipgui_post_event_t;

/* 多点触摸事件 */
struct input_mt_slot {
	// int abs[ABS_MT_LAST - ABS_MT_FIRST + 1];
	unsigned int frame;
	unsigned int key;
};
typedef struct input_mt {
	int trkid;
	int num_slots;
	int slot;
	unsigned int flags;
	unsigned int frame;
	int *red;
	struct input_mt_slot slots[];
}ipgui_input_mt_t;

typedef void (*event_poll)(struct ipgui_input_dev_ctx * dev, void * args);

typedef struct ipgui_input_dev_ctx{
    void * priv;
    const char * name;
    unsigned int prio;
    void * scr_owner;
    ipgui_node_t node;
    ipgui_list_t handle_list;
#if defined (IPGUI_BASETYPE_64BIT)
    unsigned long evtbits[IPGUI_BITS_TO_BTYPE(IPGUI_EVT_TYPE_CNT)];
    unsigned long keybits[IPGUI_BITS_TO_BTYPE(IPGUI_EVT_KEY_CNT)];
    unsigned long relbits[IPGUI_BITS_TO_BTYPE(IPGUI_EVT_REL_CNT)];
    unsigned long absbits[IPGUI_BITS_TO_BTYPE(IPGUI_EVT_ABS_CNT)];
    unsigned long miscbits[IPGUI_BITS_TO_BTYPE(IPGUI_EVT_MISC_CNT)];
    unsigned long swbits[IPGUI_BITS_TO_BTYPE(IPGUI_EVT_SW_CNT)];
#else
    unsigned int evtbits[IPGUI_BITS_TO_BTYPE(IPGUI_EVT_TYPE_CNT)];
    unsigned int keybits[IPGUI_BITS_TO_BTYPE(IPGUI_EVT_KEY_CNT)];
    unsigned int relbits[IPGUI_BITS_TO_BTYPE(IPGUI_EVT_REL_CNT)];
    unsigned int absbits[IPGUI_BITS_TO_BTYPE(IPGUI_EVT_ABS_CNT)];
    unsigned int miscbits[IPGUI_BITS_TO_BTYPE(IPGUI_EVT_MSC_CNT)];
    unsigned int swbits[IPGUI_BITS_TO_BTYPE(IPGUI_EVT_SW_CNT)];
#endif
    ipgui_input_mt_t * mt;
    unsigned int max_evt_nums;
    unsigned int evt_nums;
    ipgui_input_event_t * evts;
    //epoll需要用户自己实现，并在epoll中调用input_event上报，如果是中断方式，推荐用队列来存储事件，需要在中断中将事件放入队列中, 再在epoll中取出来
    event_poll epoll;
    void * epoll_args;

    ipgui_post_event_t event;
}ipgui_input_dev_t;

struct input_device_id{
    unsigned int flags;
	unsigned int driver_info;
#if defined (IPGUI_BASETYPE_64BIT)
    unsigned long evtbits[IPGUI_BITS_TO_BTYPE(IPGUI_EVT_TYPE_CNT)];
    unsigned long keybits[IPGUI_BITS_TO_BTYPE(IPGUI_EVT_KEY_CNT)];
    unsigned long relbits[IPGUI_BITS_TO_BTYPE(IPGUI_EVT_REL_CNT)];
    unsigned long absbits[IPGUI_BITS_TO_BTYPE(IPGUI_EVT_ABS_CNT)];
    unsigned long miscbits[IPGUI_BITS_TO_BTYPE(IPGUI_EVT_MISC_CNT)];
    unsigned long swbits[IPGUI_BITS_TO_BTYPE(IPGUI_EVT_SW_CNT)];
#else
    unsigned int evtbits[IPGUI_BITS_TO_BTYPE(IPGUI_EVT_TYPE_CNT)];
    unsigned int keybits[IPGUI_BITS_TO_BTYPE(IPGUI_EVT_KEY_CNT)];
    unsigned int relbits[IPGUI_BITS_TO_BTYPE(IPGUI_EVT_REL_CNT)];
    unsigned int absbits[IPGUI_BITS_TO_BTYPE(IPGUI_EVT_ABS_CNT)];
    unsigned int miscbits[IPGUI_BITS_TO_BTYPE(IPGUI_EVT_MSC_CNT)];
    unsigned int swbits[IPGUI_BITS_TO_BTYPE(IPGUI_EVT_SW_CNT)];
#endif
};



typedef struct ipgui_input_handler_st
{
    void * priv_data;

    void (* event)(ipgui_input_handle_t * handle, unsigned int type, unsigned int code, unsigned int value);
    void (* events)(ipgui_input_handle_t * handle, ipgui_input_event_t * evts, unsigned int count);
    int (* match)(ipgui_input_handler_t * handler, ipgui_input_dev_t * dev);
    int (* filter)(ipgui_input_handle_t * handle, unsigned int type, unsigned int code, unsigned int value); /* 手动过滤一些事件 */
    const struct input_device_id *id_table; /* 用于匹配device */

    ipgui_list_t handle_list;
    ipgui_node_t node;
}ipgui_input_handler_t;

typedef struct ipgui_input_handle_st{
    ipgui_input_dev_t * dev;
    ipgui_input_handler_t * handler;
    ipgui_node_t dev_node;/* 用于链入所指向的input_dev的handle链表 */
    ipgui_node_t handler_node;/* 用于链入所指向的input_handler的handle链表 */
}ipgui_input_handle_t;

extern __IPGUI_API__ void ipgui_input_set_capability(ipgui_input_dev_t * dev, unsigned int type, unsigned int code);

extern __IPGUI_API__ ipgui_err_t ipgui_input_event(ipgui_input_dev_t * dev, unsigned int type, unsigned int code, int value);

extern __IPGUI_API__ ipgui_err_t ipgui_input_register_handler(ipgui_input_handler_t * handler, ipgui_list_t * dev_list, ipgui_list_t * handler_list);

extern __IPGUI_API__ ipgui_input_dev_t * ipgui_input_allocate_device(const char * name);

extern __IPGUI_API__ ipgui_err_t ipgui_input_register_device(ipgui_input_dev_t * dev, ipgui_list_t * dev_list, ipgui_list_t * handler_list);

extern __IPGUI_API__ ipgui_err_t ipgui_input_sync(ipgui_input_dev_t * dev);

extern __IPGUI_API__ void ipgui_register_event_poll(ipgui_input_dev_t * dev, event_poll epoll, void * args);

extern void ipgui_event_moudle_init(void);

#define IPGUI_INPUT_DEV_OWNER(dev) (ipgui_scr_t *)((dev)->scr_owner)

/* 宏展开可能会和其他变量重名 */
#define ipgui_event_loop(scr) __IPGUI_MACRO_START\
        void * args;\
        struct list_head * pos, * next;\
        ipgui_input_dev_t * input_dev;\
        list_for_each_safe(pos, next, &scr->inputs)\
        {\
            input_dev = list_entry(pos, ipgui_input_dev_t, node);\
            if(input_dev->epoll)\
            {\
                args = input_dev->epoll_args;\
                input_dev->epoll(input_dev, args);\
            }\
        }\
        __IPGUI_MACRO_END

#define ipgui_event_loop_all() __IPGUI_MACRO_START\
        ipgui_scr_t * scr;\
        ipgui_event_loop(scr)\
__IPGUI_MACRO_END

IPGUI_HEADER_END   _______________MARKER_______________
#endif