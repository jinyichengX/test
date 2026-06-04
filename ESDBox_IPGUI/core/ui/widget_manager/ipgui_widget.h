#ifndef IPGUI_WIDGET_H
#define IPGUI_WIDGET_H

#include "ipgui_utils.h"
#include "ipgui_types.h"
#include "ipgui_prim.h"
#include "ipgui_darray.h"

struct ipgui_widget_ctx;

#define IPGUI_WIDG_STATE_HOVERED 0x00
#define IPGUI_WIDG_STATE_CLICKED 0x01

typedef int (* wid_ops_t)(struct ipgui_widget_ctx *, void * args);

typedef enum
{
    IPGUI_FOCUS_CODE_NONE       = 0,
    IPGUI_FOCUS_BY_KEY_TAB      = 0x01,
    IPGUI_FOCUS_BY_MOUSE_WHEEL  = 0x02,
    IPGUI_FOCUS_BY_CLICK        = 0x04,
    IPGUI_FOCUS_BY_ALL          = 0x07,
}ipgui_focus_policy_e;

typedef struct ipgui_widget_skin_ctx
{
    unsigned char opacity;              /* 整体不透明度 */
    //ipgui_gradient_t bg; //背景渐变
}ipgui_widget_skin_t;

typedef enum {
    IPGUI_WIDGET_FLAG_OVERVIEW_PARENT = 0x01, /* 被父控件包围 */
    IPGUI_WIDGET_FLAG_INVISIBLE       = 0x02, /* 是否可见 */
    IPGUI_WIDGET_FLAG_FIXED           = 0x04, /* 固定位置 */
};
typedef struct ipgui_widget_event_ctx ipgui_widget_event_handler_t;
typedef int (* ipgui_widget_event_cb_t)(unsigned int codes, void * args);

typedef enum {
    /* mouse/touch events */
    IPGUI_POINT_DOWN = 0x01,        /* 鼠标/触摸在控件内按下（可拖拽/固定控件都有效） */
    IPGUI_POINT_UP   = 0x02,        /* 鼠标/触摸在控件内抬起（可拖拽/固定控件都有效） */
    IPGUI_POINT_UP_OUT = 0x04,      /* 鼠标/触摸在（控件内按下）控件外抬起（仅对固定控件有效） */
    IPGUI_POINT_PRESSING = 0x07,    /* 鼠标/触摸在控件内按下后持续按下（仅对固定控件都有效） */
    IPGUI_POINT_PRESSING_OUT = 0x0F, /* 鼠标/触摸在控件内按下后移出控件并持续按下（仅对固定控件有效） */
    IPGUI_POINT_CLICKED = 0x08,       /* 鼠标/触摸点击（按下抬起都必须在控件内部完成）（仅对固定控件有效） */
    IPGUI_POINT_HOVWER_IN = 0x10,   /* 鼠标/触摸悬停于控件内，当悬停坐标改变时，继续触发这个事件（可拖拽/固定控件都有效） */
    IPGUI_POINT_HOVER_OUT = 0x20,   /* 鼠标/触摸悬停于控件时触发一次这个事件，控件之外悬停坐标改变不重复触发这个事件（可拖拽/固定控件都有效） */
    IPGUI_POINT_DRAGGING = 0x40,    /* 鼠标/触摸拖拽控件，当持续拖拽时，持续触发这个事件（仅对允许拖拽的控件有效） */

    /* keyboard events */
    IPGUI_KEY_PRESS = 0x80,         /* 键盘按下 */
    IPGUI_KEY_RELEASE = 0x100,       /* 键盘抬起 */

    /* redraw events */
    IPGUI_WIDGET_REDRAW_START = 0x200, /* 重绘开始 */
    IPGUI_WIDGET_REDRAWING = 0x300,   /* 重绘 */
    IPGUI_WIDGET_REDRAW_END = 0x400,   /* 重绘结束 */

    IPGUI_WIDGET_EVENT_ALL = 0xFFFFFFFF, /* 所有事件 */
}ipgui_widget_event_code_t;

typedef struct ipgui_widget_event_ctx{
    unsigned int codes;
    ipgui_widget_event_cb_t event_cb;
    void * args;
}ipgui_widget_event_handler_t;

typedef struct ipgui_widget_ctx
{
    const char * name;//测试用

    /* layer management */
    struct ipgui_widget_ctx * next;     /* 上层控件 */
    struct ipgui_widget_ctx * prev;     /* 下层控件 */
    struct ipgui_widget_ctx * parent;   /* 如果没有父控件，那么这个控件包围盒受屏幕制约 */
    struct ipgui_widget_ctx * childs;   /* 指向第一个子控件，先下层后上层 */
    unsigned char child_num;

    /* position management */
    ipgui_aabb_t rect;                  /* 控件包围盒（相对于父控件） */
    ipgui_aabb_t act_aabb;              /* 控件绝对包围盒（相对于屏幕） */
    char all_dirty;                     /* 是否需要重绘整个控件 */
    
    ipgui_darray_t dirty_region;        /* 需要重绘的脏区域 */
    unsigned char dirty : 1;
    
    /* attributes */
    int flags;                         /* 控件属性 */

    unsigned char visible : 1;
    unsigned char allow_drag : 1;
    unsigned char has_focus : 1;
    unsigned char focused : 1;
    unsigned char active : 1;           /* 控件是否处于激活状态, 未激活将其置灰等 */
    unsigned char resv : 2;
    
    unsigned char opacity;              /* 整体不透明度 */
    unsigned char focus_policy;         /* 焦点获取策略 */

    ipgui_color_t color;//测试用

    /* event handlers */
    wid_ops_t draw;
    wid_ops_t hit;
    wid_ops_t hover;
    wid_ops_t drag;
    wid_ops_t on_focus;
    wid_ops_t on_blur;

    void * args;

    /* user event callback */
    ipgui_darray_t user_event_cb;
}ipgui_widget_t;

__IPGUI_STATIC__ int ipgui_widget_is_flag_set(ipgui_widget_t * widget, int flag)
{
    return widget->flags & flag;
}

extern __IPGUI_API__ void ipgui_widget_set_visible(ipgui_widget_t * widget, int visible);
extern __IPGUI_API__ int ipgui_widget_is_parent_of(ipgui_widget_t * parent, ipgui_widget_t * child);
extern __IPGUI_API__ void ipgui_widget_detach_from_layer(ipgui_widget_t *);
extern __IPGUI_API__ void ipgui_widget_set_toplayer(ipgui_widget_t *);
extern __IPGUI_API__ void ipgui_widget_set_bottomlayer(ipgui_widget_t *);
extern __IPGUI_API__ void ipgui_widget_traverse_bfs(ipgui_widget_t * root, wid_ops_t ops);

extern __IPGUI_API__ int ipgui_widget_gen_visible_aabb_global(ipgui_widget_t * widget, ipgui_aabb_t * res, int flag);
extern __IPGUI_API__ int ipgui_widget_gen_visible_aabb_local(ipgui_widget_t * widget, ipgui_aabb_t * res);
extern __IPGUI_API__ int ipgui_widget_gen_act_aabb_local(ipgui_widget_t * widget, ipgui_aabb_t * res);
extern __IPGUI_API__ ipgui_widget_t * ipgui_widget_topest_on(ipgui_widget_t * root, ipgui_point_t * p);
extern __IPGUI_API__ ipgui_point_t ipgui_widget_global2_visible_local_offset(ipgui_widget_t * widget, ipgui_point_t * gp);
extern __IPGUI_API__ int ipgui_widget_event_handler(ipgui_widget_t * widget, unsigned int codes);
extern __IPGUI_API__ int ipgui_widget_register_event(ipgui_widget_t * widget, unsigned int codes, ipgui_widget_event_cb_t cb, void * args);

#endif

