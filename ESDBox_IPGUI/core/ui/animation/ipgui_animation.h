#ifndef IPGUI_ANIMATION_H
#define IPGUI_ANIMATION_H

#include "ipgui_time.h"
#include "ipgui_list.h"

/* f(t), t ∈ [t1, t2], 纯函数无副作用，返回值由调用方解释 */
typedef ipgui_anim_value_t (* ipgui_anim_func_t)(struct ipgui_anim_t * anim, ipgui_tick_t t, void * data);

/* 每帧推送值回调，anim = 动画对象，value = 当前动画值 */
typedef void (* ipgui_anim_path_cb_t)(struct ipgui_anim_t * anim, ipgui_anim_value_t value, void * path_cb_user_data);

/* 动画完成回调（归还池前调用），user_data 来自 dsc */
typedef void (* ipgui_anim_finish_cb_t)(struct ipgui_anim_t * anim, void * finish_cb_user_data);

typedef enum {
    IPGUI_ANIM_LOOP_DEFAULT = 0,   /* t1→t2（默认） */
    IPGUI_ANIM_LOOP_PING_PONG,     /* 往返t1→t2→t1→... */
} ipgui_anim_loop_type_t;

typedef struct {
    /* 动画曲线函数 */
    ipgui_anim_func_t      anim_func;
    void                 * data;

    ipgui_tick_t           t1, t2;        /* 函数定义区间 */

    /* 循环 */
    ipgui_anim_loop_type_t loop_type;     /* 循环类型 */
    u32_t                  loop_count;    /* 循环次数，0=无限 */

    /* 延迟，只在第一次循环有效 */
    ipgui_tick_t           start_delay;

    /* 推模式回调：每帧自动调用，传入当前动画值 */
    ipgui_anim_path_cb_t   path_cb;
    void                 * path_cb_user_data;

    /* 动画完成回调 */
    ipgui_anim_finish_cb_t finish_cb;
    void                 * finish_cb_user_data;     /* 透传给完成动画完成回调，可以为NULL */
} ipgui_anim_dsc_t;

typedef struct ipgui_anim_t ipgui_anim_t;

struct ipgui_anim_t {
    struct list_head   node;
    ipgui_anim_dsc_t   dsc;
    u8_t               state;       /* READY / RUNNING */
    ipgui_tick_t       duration;    /* t2 - t1 + 1 */
    ipgui_tick_t       start;       /* 启动时刻 + start_delay 锚点 */
};

/*
 * ==========================================================================
 * 调用规则
 * ==========================================================================
 *
 * 创建与启动:
 *   anim = ipgui_anim_create(&dsc);  // 从内存池分配
 *   ipgui_anim_start(anim);          // 启动，每段动画只能 start 一次
 *   重复 start 返回 IPGUI_ERR_ANIM_ALREADY_RUNNING
 *
 * 主循环顺序:
 *   while (1) {
 *       ipgui_dispatch_input_event(...);  // 事件分发
 *       ipgui_anim_update_all();          // 动画驱动（推送值、回收）
 *       ipgui_screen_render(...);         // 渲染
 *       ipgui_tick_inc();                 // tick++
 *   }
 *
 * path_cb 回调（每帧自动调用，框架负责）:
 *   - 只在 ipgui_anim_update_all 上下文中被调用
 *   - 同一个值可能被推送多次（启动时、结束时各多一次），
 *     回调必须幂等：widget->x = v 写同值两次无影响
 *   - 若有副作用（写寄存器、发 CAN 报文、自增计数器），
 *     需自行判断值是否变化再触发，框架不保证去重
 *   - 禁止调用动画创建、启动、销毁接口，禁止修改动画链表结构；
 *   - 禁止执行耗时、阻塞操作，禁止调用会引起任务调度切换的阻塞函数；
 *   - 禁止修改动画对象内部的私有成员。
 *
 * finish_cb 回调（动画播完归还池前调用一次，可选）:
 *   - 用于通知调用方动画已结束，清理 own flag、切换状态等
 *   - 无限循环（loop_count=0）不会调用 finish_cb
 *   - 不要在 finish_cb 内 create / start / 销毁动画
 * 
 * anim_func 曲线函数:
 *   - 纯函数，输入 t 输出值，不能有副作用
 *   - t ∈ [t1, t2] 闭区间，duration 包含了 t2 这一帧
 *   - tick 定义为帧步长（1ms/tick），不是连续时间
 *
 * 生命周期:
 *   - 动画播完自动回收回内存池，用户无需手动释放
 *   - 内存池满时 create 返回 NULL，需检查返回值
 *   - 不要长期持有 anim 指针（回收后可能被新动画复用）
 *
 * 内存池大小:
 *   通过 ipgui_conf.h 中 IPGUI_ANIM_POOL_SIZE 配置，默认 16
 */

/* 创建动画（从内存池分配），池满返回 NULL */
__IPGUI_API__ ipgui_anim_t * ipgui_anim_create(const ipgui_anim_dsc_t * dsc);

/* 启动动画，每段动画只能调用一次 */
__IPGUI_API__ ipgui_err_t    ipgui_anim_start(ipgui_anim_t * anim);

/* 驱动所有运行动画，每帧调用，内部自动推送值并回收已完成动画 */
__IPGUI_API__ void           ipgui_anim_update_all(void);

#endif
