/**
 * @file    ipgui_animation.h
 * @brief   动画模块 — 嵌入式GUI系统通用动画引擎
 *
 * ## 模块概述
 * 提供完整的UI动画支持，包括淡入淡出、位移、缩放、旋转、颜色渐变等效果。
 * 采用纯整数运算，支持定点数插值，适用于无FPU的嵌入式平台。
 *
 * ## 设计目标
 *  - 零动态内存分配，支持静态初始化
 *  - 纯整数运算，避免浮点依赖
 *  - 可配置缓动曲线，支持自定义插值
 *  - 支持动画队列和优先级管理
 *  - 高效的时间线更新，最小化CPU开销
 *
 * ## 动画类型
 *   - 透明度动画 (Fade)
 *   - 位移动画 (Translate)
 *   - 缩放动画 (Scale)
 *   - 旋转动画 (Rotate)
 *   - 颜色动画 (Color)
 *   - 尺寸动画 (Size)
 *   - 组合动画 (Group)
 *
 * ## 使用示例
 * @code
 *   // 1. 定义动画描述符
 *   ipgui_anim_dsc_t fade_out = {
 *       .type      = IPGUI_ANIM_TYPE_FADE,
 *       .from.u8_  = 255,
 *       .to.u8_    = 0,
 *       .duration  = 300,    // 300ms
 *       .ease      = IPGUI_ANIM_EASE_OUT_QUAD,
 *       .loop_mode = IPGUI_ANIM_LOOP_NONE,
 *   };
 *
 *   // 2. 创建动画
 *   ipgui_anim_handle_t h = ipgui_anim_create(&fade_out);
 *
 *   // 3. 启动动画
 *   ipgui_anim_start(h);
 *
 *   // 4. 在主循环中更新
 *   ipgui_anim_update_all(ipgui_tick_get());
 *
 *   // 5. 销毁动画
 *   ipgui_anim_destroy(h);
 * @endcode
 */

#ifndef IPGUI_ANIMATION_H
#define IPGUI_ANIMATION_H

#include "ipgui_types.h"
#include "ipgui_utils.h"
#include "ipgui_prim.h"
#include "ipgui_defs.h"

IPGUI_HEADER_BEGIN _______________MARKER_______________

/*============================================================================
 * 常量与宏定义
 *===========================================================================*/

/** 动画系统支持的最大动画数量 */
#ifndef IPGUI_ANIM_MAX_COUNT
#define IPGUI_ANIM_MAX_COUNT         32
#endif

/** 动画ID无效值 */
#define IPGUI_ANIM_INVALID_HANDLE    0

/** 默认动画时长 (ms) */
#define IPGUI_ANIM_DEFAULT_DURATION  300

/** 最小动画时长 (ms)，小于此值将被忽略 */
#define IPGUI_ANIM_MIN_DURATION      10

/** 动画更新精度 (ms) */
#define IPGUI_ANIM_UPDATE_TICK_MS    16

/*============================================================================
 * 动画类型枚举
 *===========================================================================*/

typedef enum {
    IPGUI_ANIM_TYPE_NONE         = 0x0000,  /**< 无效类型 */

    /* 单属性动画 */
    IPGUI_ANIM_TYPE_FADE         = 0x0001,  /**< 透明度动画 */
    IPGUI_ANIM_TYPE_TRANSLATE_X  = 0x0002,  /**< X轴位移动画 */
    IPGUI_ANIM_TYPE_TRANSLATE_Y  = 0x0004,  /**< Y轴位移动画 */
    IPGUI_ANIM_TYPE_TRANSLATE    = 0x0006,  /**< 二维位移动画 (TRANSLATE_X | TRANSLATE_Y) */
    IPGUI_ANIM_TYPE_SCALE_X      = 0x0008,  /**< X轴缩放动画 */
    IPGUI_ANIM_TYPE_SCALE_Y      = 0x0010,  /**< Y轴缩放动画 */
    IPGUI_ANIM_TYPE_SCALE        = 0x0018,  /**< 均匀缩放动画 (SCALE_X | SCALE_Y) */
    IPGUI_ANIM_TYPE_ROTATE       = 0x0020,  /**< 旋转动画 */
    IPGUI_ANIM_TYPE_WIDTH        = 0x0040,  /**< 宽度动画 */
    IPGUI_ANIM_TYPE_HEIGHT       = 0x0080,  /**< 高度动画 */
    IPGUI_ANIM_TYPE_SIZE         = 0x00C0,  /**< 尺寸动画 (WIDTH | HEIGHT) */

    /* 颜色动画 */
    IPGUI_ANIM_TYPE_COLOR_R      = 0x0100,  /**< 红色分量动画 */
    IPGUI_ANIM_TYPE_COLOR_G      = 0x0200,  /**< 绿色分量动画 */
    IPGUI_ANIM_TYPE_COLOR_B      = 0x0400,  /**< 蓝色分量动画 */
    IPGUI_ANIM_TYPE_COLOR_A      = 0x0800,  /**< Alpha分量动画 */
    IPGUI_ANIM_TYPE_COLOR_RGB    = 0x0700,  /**< RGB颜色动画 */
    IPGUI_ANIM_TYPE_COLOR        = 0x0F00,  /**< 完整颜色动画 */

    /* 复合动画 */
    IPGUI_ANIM_TYPE_GROUP        = 0x1000,  /**< 动画组 */

    /* 用户自定义类型起始值 */
    IPGUI_ANIM_TYPE_USER          = 0x10000,
} ipgui_anim_type_t;

/*============================================================================
 * 缓动曲线枚举
 *===========================================================================*/

/**
 * 缓动曲线类型
 * @note 所有曲线均为纯整数实现，使用定点数运算
 */
typedef enum {
    /* 线性 */
    IPGUI_ANIM_EASE_LINEAR,           /**< 线性匀速 */

    /* Quad 系列 */
    IPGUI_ANIM_EASE_IN_QUAD,         /**< 二次方加速入 */
    IPGUI_ANIM_EASE_OUT_QUAD,        /**< 二次方减速出 */
    IPGUI_ANIM_EASE_IN_OUT_QUAD,     /**< 二次方先加后减 */

    /* Cubic 系列 */
    IPGUI_ANIM_EASE_IN_CUBIC,        /**< 三次方加速入 */
    IPGUI_ANIM_EASE_OUT_CUBIC,       /**< 三次方减速出 */
    IPGUI_ANIM_EASE_IN_OUT_CUBIC,    /**< 三次方先加后减 */

    /* Quart 系列 */
    IPGUI_ANIM_EASE_IN_QUART,        /**< 四次方加速入 */
    IPGUI_ANIM_EASE_OUT_QUART,       /**< 四次方减速出 */
    IPGUI_ANIM_EASE_IN_OUT_QUART,    /**< 四次方先加后减 */

    /* Sine 系列 */
    IPGUI_ANIM_EASE_IN_SINE,         /**< 正弦加速入 */
    IPGUI_ANIM_EASE_OUT_SINE,        /**< 正弦减速出 */
    IPGUI_ANIM_EASE_IN_OUT_SINE,     /**< 正弦先加后减 */

    /* Expo 系列 */
    IPGUI_ANIM_EASE_IN_EXPO,         /**< 指数加速入 */
    IPGUI_ANIM_EASE_OUT_EXPO,        /**< 指数减速出 */
    IPGUI_ANIM_EASE_IN_OUT_EXPO,     /**< 指数先加后减 */

    /* Back 系列 (带回退效果) */
    IPGUI_ANIM_EASE_IN_BACK,         /**< 回退加速入 */
    IPGUI_ANIM_EASE_OUT_BACK,        /**< 回退减速出 */
    IPGUI_ANIM_EASE_IN_OUT_BACK,     /**< 回退先加后减 */

    /* Bounce 系列 (带弹跳效果) */
    IPGUI_ANIM_EASE_IN_BOUNCE,       /**< 弹跳加速入 */
    IPGUI_ANIM_EASE_OUT_BOUNCE,      /**< 弹跳减速出 */
    IPGUI_ANIM_EASE_IN_OUT_BOUNCE,   /**< 弹跳先加后减 */

    IPGUI_ANIM_EASE_MAX,
} ipgui_anim_ease_t;

/*============================================================================
 * 循环模式枚举
 *===========================================================================*/

typedef enum {
    IPGUI_ANIM_LOOP_NONE,           /**< 不循环 */
    IPGUI_ANIM_LOOP_FORWARD,        /**< 正向循环 */
    IPGUI_ANIM_LOOP_PING_PONG,      /**< 乒乓循环 (往返) */
} ipgui_anim_loop_mode_t;

/*============================================================================
 * 动画状态枚举
 *===========================================================================*/

typedef enum {
    IPGUI_ANIM_STATE_IDLE,          /**< 空闲/未启动 */
    IPGUI_ANIM_STATE_DELAY,         /**< 等待延迟 */
    IPGUI_ANIM_STATE_RUNNING,       /**< 运行中 */
    IPGUI_ANIM_STATE_PAUSED,        /**< 暂停 */
    IPGUI_ANIM_STATE_COMPLETED,     /**< 已完成 */
    IPGUI_ANIM_STATE_STOPPED,       /**< 已停止 */
} ipgui_anim_state_t;

/*============================================================================
 * 动画值联合体
 *
 * 支持多种数据类型，便于统一动画接口
 *===========================================================================*/

typedef union {
    u8_t    u8_;                     /**< 8位无符号 (透明度等) */
    s8_t    s8_;                     /**< 8位有符号 */
    u16_t   u16_;                    /**< 16位无符号 */
    s16_t   s16_;                    /**< 16位有符号 */
    u32_t   u32_;                    /**< 32位无符号 */
    s32_t   s32_;                    /**< 32位有符号 */
    s64_t   s64_;                    /**< 64位有符号 */
    float   f32_;                    /**< 32位浮点 */
    /* 定点数 16.16 */
    s32_t   fp16_16_;
    /* 坐标类型 */
    ipgui_coord_t  coord_;          /**< 坐标类型 */
    ipgui_scoord_t scoord_;         /**< 子像素坐标 (26.6) */
    /* 颜色分量 */
    struct {
        u8_t r, g, b, a;
    } color_;
    /* 点类型 */
    ipgui_point_t    point_;        /**< 二维点 */
    ipgui_spoint_t   spoint_;       /**< 子像素点 */
    /* 尺寸类型 */
    struct {
        ipgui_coord_t w, h;
    } size_;
    /* 用户数据指针 */
    void * ptr_;
} ipgui_anim_value_t;

/*============================================================================
 * 动画值范围结构
 *===========================================================================*/

typedef struct {
    ipgui_anim_value_t  start;      /**< 起始值 */
    ipgui_anim_value_t  end;       /**< 结束值 */
} ipgui_anim_range_t;

/*============================================================================
 * 动画句柄前向声明（回调函数定义依赖此类型）
 *===========================================================================*/

typedef u32_t ipgui_anim_handle_t;

/*============================================================================
 * 缓动函数类型定义
 *
 * @param t   当前时间 (0 ~ duration)
 * @param b   起始值
 * @param c   变化量 (end - start)
 * @param d   总时长
 * @return    当前插值结果
 *===========================================================================*/

typedef s32_t (* ipgui_anim_ease_func_t)(s32_t t, s32_t b, s32_t c, s32_t d);

/*============================================================================
 * 动画回调函数类型
 *===========================================================================*/

/**
 * 动画更新回调
 * @param handle   动画句柄
 * @param value    当前插值结果
 * @param user_data 用户数据
 */
typedef void (* ipgui_anim_update_cb_t)(ipgui_anim_handle_t handle,
                                         ipgui_anim_value_t * value,
                                         void * user_data);

/**
 * 动画完成回调
 * @param handle   动画句柄
 * @param user_data 用户数据
 */
typedef void (* ipgui_anim_complete_cb_t)(ipgui_anim_handle_t handle,
                                           void * user_data);

/**
 * 动画状态改变回调
 * @param handle   动画句柄
 * @param old_state 旧状态
 * @param new_state 新状态
 * @param user_data 用户数据
 */
typedef void (* ipgui_anim_state_change_cb_t)(ipgui_anim_handle_t handle,
                                               ipgui_anim_state_t old_state,
                                               ipgui_anim_state_t new_state,
                                               void * user_data);

/**
 * 动画每帧回调 (用于自定义渲染)
 * @param handle   动画句柄
 * @param progress 当前进度 (0.0 ~ 1.0)
 * @param user_data 用户数据
 */
typedef void (* ipgui_anim_frame_cb_t)(ipgui_anim_handle_t handle,
                                        float progress,
                                        void * user_data);

/*============================================================================
 * 动画描述符 (创建动画时的配置)
 *===========================================================================*/

typedef struct {
    ipgui_anim_type_t      type;       /**< 动画类型 */
    ipgui_anim_value_t     from;       /**< 起始值 */
    ipgui_anim_value_t     to;         /**< 结束值 */
    u32_t                  duration;   /**< 动画时长 (ms) */
    u32_t                  delay;      /**< 延迟启动时间 (ms) */
    ipgui_anim_ease_t      ease;       /**< 缓动曲线类型 */
    ipgui_anim_loop_mode_t loop_mode; /**< 循环模式 */
    u16_t                  loop_count; /**< 循环次数 (0=无限) */

    /* 回调函数 */
    ipgui_anim_update_cb_t     update_cb;      /**< 更新回调 */
    ipgui_anim_complete_cb_t   complete_cb;    /**< 完成回调 */
    ipgui_anim_state_change_cb_t state_cb;     /**< 状态改变回调 */
    ipgui_anim_frame_cb_t      frame_cb;       /**< 每帧回调 */

    /* 用户数据 */
    void                  * user_data;

    /* 优先级 (数值越大优先级越高) */
    u8_t                   priority;
} ipgui_anim_dsc_t;

/*============================================================================
 * 动画控制命令
 *===========================================================================*/

typedef enum {
    IPGUI_ANIM_CMD_START,     /**< 开始/恢复 */
    IPGUI_ANIM_CMD_PAUSE,     /**< 暂停 */
    IPGUI_ANIM_CMD_STOP,      /**< 停止 */
    IPGUI_ANIM_CMD_RESTART,   /**< 重新开始 */
    IPGUI_ANIM_CMD_REVERSE,   /**< 反向播放 */
    IPGUI_ANIM_CMD_SET_SPEED, /**< 设置速度 */
} ipgui_anim_cmd_t;

/*============================================================================
 * 动画内部状态
 *===========================================================================*/

typedef struct ipgui_anim ipgui_anim_t;

struct ipgui_anim {
    /* 基础信息 */
    u32_t                  id;             /**< 唯一ID */
    ipgui_anim_type_t      type;           /**< 动画类型 */
    ipgui_anim_state_t     state;          /**< 当前状态 */

    /* 时间和进度 */
    u32_t                  start_time;     /**< 开始时间戳 */
    u32_t                  duration;       /**< 实际时长 (ms) */
    u32_t                  delay;          /**< 延迟 (ms) */
    u32_t                  elapsed;        /**< 已消耗时间 */
    float                  progress;       /**< 当前进度 (0.0 ~ 1.0) */

    /* 缓动 */
    ipgui_anim_ease_t      ease;           /**< 缓动曲线 */
    ipgui_anim_ease_func_t ease_func;      /**< 缓动函数指针 */

    /* 值范围 */
    ipgui_anim_value_t     from;           /**< 起始值 */
    ipgui_anim_value_t     to;             /**< 结束值 */
    ipgui_anim_value_t     current;        /**< 当前值 */

    /* 循环控制 */
    ipgui_anim_loop_mode_t loop_mode;     /**< 循环模式 */
    u16_t                  loop_count;     /**< 循环次数 */
    u16_t                  current_loop;   /**< 当前循环计数 */
    s32_t                  direction;      /**< 播放方向 (+1/-1) */

    /* 速度控制 */
    float                  speed;          /**< 播放速度倍率 */

    /* 回调函数 */
    ipgui_anim_update_cb_t     update_cb;
    ipgui_anim_complete_cb_t   complete_cb;
    ipgui_anim_state_change_cb_t state_cb;
    ipgui_anim_frame_cb_t      frame_cb;

    /* 用户数据 */
    void                  * user_data;

    /* 优先级 */
    u8_t                   priority;

    /* 链表节点 */
    struct ipgui_anim      * next;
};

/*============================================================================
 * 动画系统配置
 *===========================================================================*/

typedef struct {
    u32_t  max_anim_count;        /**< 最大动画数量 */
    u32_t  default_duration;      /**< 默认动画时长 */
    u8_t   enable_auto_tick;      /**< 是否自动更新时间戳 */
} ipgui_anim_sys_config_t;

/*============================================================================
 * 动画系统句柄
 *===========================================================================*/

typedef struct {
    /* 动画池 */
    ipgui_anim_t           anim_pool[IPGUI_ANIM_MAX_COUNT];

    /* 活跃动画链表 */
    ipgui_anim_t         * active_list;

    /* 空闲动画链表 */
    ipgui_anim_t         * free_list;

    /* ID分配位图 */
    u32_t                  id_bitmap[(IPGUI_ANIM_MAX_COUNT + 31) >> 5];

    /* 系统状态 */
    u32_t                  last_update_time;
    u32_t                  running_anim_count;
    u8_t                   initialized;

    /* 全局回调 */
    ipgui_anim_frame_cb_t  global_frame_cb;
    void                  * global_user_data;
} ipgui_anim_sys_t;

/*============================================================================
 * 动画组结构
 *===========================================================================*/

typedef struct {
    ipgui_anim_handle_t handles[IPGUI_ANIM_MAX_COUNT];  /**< 子动画句柄数组 */
    u32_t               count;                           /**< 子动画数量 */
    ipgui_anim_loop_mode_t loop_mode;                    /**< 组循环模式 */
} ipgui_anim_group_t;

/*============================================================================
 * API 函数声明
 *===========================================================================*/

/**
 * @name 系统级函数
 * @{
 */

/**
 * @brief 初始化动画系统
 * @param config 系统配置 (可为NULL使用默认配置)
 * @return 成功返回IPGUI_ERR_OK
 */
extern __IPGUI_API__ ipgui_err_t ipgui_anim_sys_init(const ipgui_anim_sys_config_t * config);

/**
 * @brief 反初始化动画系统
 * @note 会自动销毁所有活跃动画
 */
extern __IPGUI_API__ void ipgui_anim_sys_deinit(void);

/**
 * @brief 获取动画系统句柄
 * @return 动画系统全局句柄
 */
extern __IPGUI_API__ ipgui_anim_sys_t * ipgui_anim_sys_get(void);

/**
 * @brief 更新所有活跃动画
 * @param current_time 当前时间戳 (ms)
 * @return 剩余活跃动画数量
 */
extern __IPGUI_API__ u32_t ipgui_anim_update_all(u32_t current_time);

/**
 * @brief 暂停所有动画
 */
extern __IPGUI_API__ void ipgui_anim_pause_all(void);

/**
 * @brief 恢复所有动画
 */
extern __IPGUI_API__ void ipgui_anim_resume_all(void);

/**
 * @brief 停止所有动画
 */
extern __IPGUI_API__ void ipgui_anim_stop_all(void);

/**
 * @brief 获取当前活跃动画数量
 * @return 活跃动画数量
 */
extern __IPGUI_API__ u32_t ipgui_anim_get_active_count(void);

/** @} */

/**
 * @name 单动画管理函数
 * @{
 */

/**
 * @brief 创建动画
 * @param dsc      动画描述符
 * @return 成功返回动画句柄，失败返回IPGUI_ANIM_INVALID_HANDLE
 * @note 创建后需要调用ipgui_anim_start()启动
 */
extern __IPGUI_API__ ipgui_anim_handle_t ipgui_anim_create(const ipgui_anim_dsc_t * dsc);

/**
 * @brief 销毁动画
 * @param handle 动画句柄
 * @return 成功返回IPGUI_ERR_OK
 */
extern __IPGUI_API__ ipgui_err_t ipgui_anim_destroy(ipgui_anim_handle_t handle);

/**
 * @brief 启动/恢复动画
 * @param handle 动画句柄
 * @return 成功返回IPGUI_ERR_OK
 */
extern __IPGUI_API__ ipgui_err_t ipgui_anim_start(ipgui_anim_handle_t handle);

/**
 * @brief 暂停动画
 * @param handle 动画句柄
 * @return 成功返回IPGUI_ERR_OK
 */
extern __IPGUI_API__ ipgui_err_t ipgui_anim_pause(ipgui_anim_handle_t handle);

/**
 * @brief 停止动画
 * @param handle 动画句柄
 * @return 成功返回IPGUI_ERR_OK
 */
extern __IPGUI_API__ ipgui_err_t ipgui_anim_stop(ipgui_anim_handle_t handle);

/**
 * @brief 重新开始动画
 * @param handle 动画句柄
 * @return 成功返回IPGUI_ERR_OK
 */
extern __IPGUI_API__ ipgui_err_t ipgui_anim_restart(ipgui_anim_handle_t handle);

/**
 * @brief 反向播放动画
 * @param handle 动画句柄
 * @return 成功返回IPGUI_ERR_OK
 */
extern __IPGUI_API__ ipgui_err_t ipgui_anim_reverse(ipgui_anim_handle_t handle);

/** @} */

/**
 * @name 动画属性查询/设置
 * @{
 */

/**
 * @brief 获取动画状态
 * @param handle 动画句柄
 * @return 当前状态
 */
extern __IPGUI_API__ ipgui_anim_state_t ipgui_anim_get_state(ipgui_anim_handle_t handle);

/**
 * @brief 获取动画进度
 * @param handle 动画句柄
 * @return 进度值 (0.0 ~ 1.0)
 */
extern __IPGUI_API__ float ipgui_anim_get_progress(ipgui_anim_handle_t handle);

/**
 * @brief 设置动画速度
 * @param handle 动画句柄
 * @param speed  速度倍率 (1.0=正常速度, 2.0=2倍速, 0.5=半速)
 * @return 成功返回IPGUI_ERR_OK
 */
extern __IPGUI_API__ ipgui_err_t ipgui_anim_set_speed(ipgui_anim_handle_t handle, float speed);

/**
 * @brief 获取动画速度
 * @param handle 动画句柄
 * @return 当前速度倍率
 */
extern __IPGUI_API__ float ipgui_anim_get_speed(ipgui_anim_handle_t handle);

/**
 * @brief 设置动画目标值
 * @param handle 动画句柄
 * @param to     新的目标值
 * @return 成功返回IPGUI_ERR_OK
 * @note 动画进行中调用此函数会平滑过渡到新目标
 */
extern __IPGUI_API__ ipgui_err_t ipgui_anim_set_to(ipgui_anim_handle_t handle, ipgui_anim_value_t to);

/**
 * @brief 获取动画当前值
 * @param handle 动画句柄
 * @param value  输出参数，存储当前值
 * @return 成功返回IPGUI_ERR_OK
 */
extern __IPGUI_API__ ipgui_err_t ipgui_anim_get_current_value(ipgui_anim_handle_t handle,
                                                               ipgui_anim_value_t * value);

/** @} */

/**
 * @name 回调函数管理
 * @{
 */

/**
 * @brief 设置动画更新回调
 * @param handle 动画句柄
 * @param cb     更新回调函数
 * @return 成功返回IPGUI_ERR_OK
 */
extern __IPGUI_API__ ipgui_err_t ipgui_anim_set_update_cb(ipgui_anim_handle_t handle,
                                                           ipgui_anim_update_cb_t cb);

/**
 * @brief 设置动画完成回调
 * @param handle 动画句柄
 * @param cb     完成回调函数
 * @return 成功返回IPGUI_ERR_OK
 */
extern __IPGUI_API__ ipgui_err_t ipgui_anim_set_complete_cb(ipgui_anim_handle_t handle,
                                                            ipgui_anim_complete_cb_t cb);

/**
 * @brief 设置动画状态改变回调
 * @param handle 动画句柄
 * @param cb     状态改变回调函数
 * @return 成功返回IPGUI_ERR_OK
 */
extern __IPGUI_API__ ipgui_err_t ipgui_anim_set_state_change_cb(ipgui_anim_handle_t handle,
                                                                 ipgui_anim_state_change_cb_t cb);

/** @} */

/**
 * @name 工具函数
 * @{
 */

/**
 * @brief 根据进度计算插值
 * @param from   起始值
 * @param to     结束值
 * @param ease   缓动曲线类型
 * @param t      当前时间
 * @param d      总时长
 * @return 插值结果
 */
extern __IPGUI_API__ s32_t ipgui_anim_interpolate(s32_t from, s32_t to,
                                                   ipgui_anim_ease_t ease,
                                                   s32_t t, s32_t d);

/**
 * @brief 获取缓动函数指针
 * @param ease 缓动曲线类型
 * @return 缓动函数指针
 */
extern __IPGUI_API__ ipgui_anim_ease_func_t ipgui_anim_get_ease_func(ipgui_anim_ease_t ease);

/**
 * @brief 检查句柄有效性
 * @param handle 动画句柄
 * @return 有效返回1，无效返回0
 */
extern __IPGUI_API__ int ipgui_anim_is_valid_handle(ipgui_anim_handle_t handle);

/** @} */

/**
 * @name 便捷创建函数
 * @{
 */

/**
 * @brief 创建淡入动画
 * @param duration 动画时长 (ms)
 * @param cb       更新回调
 * @param complete 完成回调
 * @param user_data 用户数据
 * @return 动画句柄
 */
extern __IPGUI_API__ ipgui_anim_handle_t ipgui_anim_create_fade_in(
    u32_t duration,
    ipgui_anim_update_cb_t cb,
    ipgui_anim_complete_cb_t complete,
    void * user_data);

/**
 * @brief 创建淡出动画
 * @param duration 动画时长 (ms)
 * @param cb       更新回调
 * @param complete 完成回调
 * @param user_data 用户数据
 * @return 动画句柄
 */
extern __IPGUI_API__ ipgui_anim_handle_t ipgui_anim_create_fade_out(
    u32_t duration,
    ipgui_anim_update_cb_t cb,
    ipgui_anim_complete_cb_t complete,
    void * user_data);

/**
 * @brief 创建位移动画
 * @param from_x, from_y 起始坐标
 * @param to_x, to_y     目标坐标
 * @param duration       动画时长 (ms)
 * @param cb             更新回调
 * @param complete       完成回调
 * @param user_data      用户数据
 * @return 动画句柄
 */
extern __IPGUI_API__ ipgui_anim_handle_t ipgui_anim_create_translate(
    ipgui_coord_t from_x, ipgui_coord_t from_y,
    ipgui_coord_t to_x, ipgui_coord_t to_y,
    u32_t duration,
    ipgui_anim_update_cb_t cb,
    ipgui_anim_complete_cb_t complete,
    void * user_data);

/**
 * @brief 创建缩放动画
 * @param from_scale 起始缩放值 (定点数 16.16)
 * @param to_scale   目标缩放值
 * @param duration   动画时长 (ms)
 * @param cb         更新回调
 * @param complete   完成回调
 * @param user_data  用户数据
 * @return 动画句柄
 */
extern __IPGUI_API__ ipgui_anim_handle_t ipgui_anim_create_scale(
    s32_t from_scale, s32_t to_scale,
    u32_t duration,
    ipgui_anim_update_cb_t cb,
    ipgui_anim_complete_cb_t complete,
    void * user_data);

/**
 * @brief 创建旋转动画
 * @param from_angle 起始角度 (1/64度)
 * @param to_angle   目标角度
 * @param duration    动画时长 (ms)
 * @param cb          更新回调
 * @param complete    完成回调
 * @param user_data   用户数据
 * @return 动画句柄
 */
extern __IPGUI_API__ ipgui_anim_handle_t ipgui_anim_create_rotate(
    s32_t from_angle, s32_t to_angle,
    u32_t duration,
    ipgui_anim_update_cb_t cb,
    ipgui_anim_complete_cb_t complete,
    void * user_data);

/** @} */

IPGUI_HEADER_END _______________MARKER_______________

#endif /* IPGUI_ANIMATION_H */
