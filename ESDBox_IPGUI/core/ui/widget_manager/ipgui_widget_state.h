// #ifndef IPGUI_WIDGET_STATE_H
// #define IPGUI_WIDGET_STATE_H

// #include "ipgui_types.h"
// #include "ipgui_style.h"

// #ifdef __cplusplus
// extern "C" {
// #endif

// /*============================================================================
//  * 状态定义（bitmask，可同时处于多个状态）
//  *===========================================================================*/
// typedef unsigned int ipgui_state_flags_t;

// typedef enum {
//     IPGUI_STATE_DEFAULT     = 0x0000,   /* 默认状态，始终存在 */
//     IPGUI_STATE_PRESSED     = 0x0001,   /* 按下 */
//     IPGUI_STATE_FOCUSED     = 0x0002,   /* 获得焦点 */
//     IPGUI_STATE_CHECKED     = 0x0004,   /* 选中/激活（如checkbox） */
//     IPGUI_STATE_DISABLED    = 0x0008,   /* 禁用 */
//     IPGUI_STATE_HOVERED     = 0x0010,   /* 悬停（触摸屏通常不用） */
//     IPGUI_STATE_EDITED      = 0x0020,   /* 编辑中（如输入框） */
//     IPGUI_STATE_USER1       = 0x0100,   /* 用户自定义状态1 */
//     IPGUI_STATE_USER2       = 0x0200,   /* 用户自定义状态2 */
//     IPGUI_STATE_USER3       = 0x0400,   /* 用户自定义状态3 */
//     IPGUI_STATE_USER4       = 0x0800,   /* 用户自定义状态4 */
// } ipgui_state_t;

// /*============================================================================
//  * 过渡动画配置（预留，当前不实现）
//  * 当状态发生切换时，指定某个属性从旧值过渡到新值的方式
//  *===========================================================================*/
// typedef enum {
//     IPGUI_TRANS_PATH_LINEAR = 0,    /* 线性 */
//     IPGUI_TRANS_PATH_EASE_IN,       /* 先慢后快 */
//     IPGUI_TRANS_PATH_EASE_OUT,      /* 先快后慢 */
//     IPGUI_TRANS_PATH_EASE_IN_OUT,   /* 两头慢中间快 */
//     IPGUI_TRANS_PATH_BOUNCE,        /* 弹跳 */
// } ipgui_trans_path_t;

// typedef struct {
//     int                 prop_id;        /* 哪个样式属性需要过渡，IPGUI_STYLE_PROP_xxx */
//     unsigned int        duration_ms;    /* 过渡时长（毫秒） */
//     unsigned int        delay_ms;       /* 延迟开始（毫秒） */
//     ipgui_trans_path_t  path;           /* 过渡曲线 */
// } ipgui_state_trans_t;

// /*============================================================================
//  * 状态-样式绑定条目
//  * 表示"在某个状态下，应用某个样式"
//  *===========================================================================*/
// typedef struct {
//     ipgui_state_flags_t     state;          /* 触发此条目的状态bitmask */
//     int                     style_id;       /* 对应的样式ID（样式表中的枚举ID） */
//     /* 过渡动画列表（预留，当前不生效） */
//     const ipgui_state_trans_t * trans;      /* 指向过渡配置数组，NULL表示无过渡 */
//     unsigned char               trans_num;  /* 过渡配置数组长度 */
// } ipgui_state_style_entry_t;

// /*============================================================================
//  * 控件状态管理器
//  * 每个控件持有一个此结构体
//  *===========================================================================*/
// #ifndef IPGUI_STATE_STYLE_ENTRY_MAX
// #define IPGUI_STATE_STYLE_ENTRY_MAX     8   /* 每个控件最多绑定几条状态-样式规则 */
// #endif

// typedef struct {
//     /* 当前状态bitmask */
//     ipgui_state_flags_t         current;

//     /* 上一次状态（用于过渡动画，预留） */
//     ipgui_state_flags_t         previous;

//     /* 状态-样式绑定表 */
//     ipgui_state_style_entry_t   entries[IPGUI_STATE_STYLE_ENTRY_MAX];
//     unsigned char               entry_count;

//     /* 过渡动画是否正在进行（预留） */
//     unsigned char               trans_running;  /* 0=无动画，1=动画进行中 */

//     /* 过渡动画回调（预留，当前传NULL即可） */
//     /* 当状态切换且有过渡配置时，框架调用此回调驱动动画 */
//     void (* trans_start_cb)(void * widget, ipgui_state_flags_t old_state,
//                             ipgui_state_flags_t new_state,
//                             const ipgui_state_trans_t * trans, unsigned char trans_num);
// } ipgui_widget_state_t;

// /*============================================================================
//  * 初始化
//  *===========================================================================*/

// /* 初始化控件状态管理器，初始状态为DEFAULT */
// __IPGUI_API__ void ipgui_widget_state_init(ipgui_widget_state_t * ws);

// /*============================================================================
//  * 状态-样式绑定
//  *===========================================================================*/

// /* 绑定一条状态-样式规则
//  * state    : 状态bitmask，可以是单个状态或多个状态的组合
//  *            例如 IPGUI_STATE_DEFAULT 表示默认状态
//  *                 IPGUI_STATE_PRESSED | IPGUI_STATE_FOCUSED 表示同时按下且聚焦
//  * style_id : 该状态下激活的样式ID
//  * trans    : 过渡动画配置数组，预留传NULL
//  * trans_num: 过渡配置数量，预留传0
//  * 返回0成功，-1失败（槽位满）
//  */
// __IPGUI_API__ int ipgui_widget_state_bind(ipgui_widget_state_t * ws,
//                                           ipgui_state_flags_t state,
//                                           int style_id,
//                                           const ipgui_state_trans_t * trans,
//                                           unsigned char trans_num);

// /* 解绑某个状态下的样式（按state精确匹配） */
// __IPGUI_API__ void ipgui_widget_state_unbind(ipgui_widget_state_t * ws,
//                                              ipgui_state_flags_t state);

// /* 清除所有绑定 */
// __IPGUI_API__ void ipgui_widget_state_unbind_all(ipgui_widget_state_t * ws);

// /*============================================================================
//  * 状态切换
//  *===========================================================================*/

// /* 添加状态标志（不影响其他已有状态） */
// __IPGUI_API__ void ipgui_widget_state_add(ipgui_widget_state_t * ws,
//                                           ipgui_state_flags_t state);

// /* 移除状态标志 */
// __IPGUI_API__ void ipgui_widget_state_remove(ipgui_widget_state_t * ws,
//                                              ipgui_state_flags_t state);

// /* 直接设置状态（完全覆盖当前状态） */
// __IPGUI_API__ void ipgui_widget_state_set(ipgui_widget_state_t * ws,
//                                           ipgui_state_flags_t state);

// /* 查询当前是否包含某个状态 */
// __IPGUI_API__ int  ipgui_widget_state_has(const ipgui_widget_state_t * ws,
//                                           ipgui_state_flags_t state);

// /* 获取当前完整状态bitmask */
// __IPGUI_API__ ipgui_state_flags_t ipgui_widget_state_get(const ipgui_widget_state_t * ws);

// /*============================================================================
//  * 过渡动画回调注册（预留）
//  * 当实现了动画系统后，将动画驱动函数注册到此处
//  * widget: 控件指针，透传给回调，状态系统本身不使用
//  *===========================================================================*/
// __IPGUI_API__ void ipgui_widget_state_set_trans_cb(
//     ipgui_widget_state_t * ws,
//     void (* cb)(void * widget,
//                 ipgui_state_flags_t old_state,
//                 ipgui_state_flags_t new_state,
//                 const ipgui_state_trans_t * trans,
//                 unsigned char trans_num));

// /*============================================================================
//  * 样式解析
//  * 根据当前状态，将匹配的样式按优先级合并到控件的样式槽中
//  *
//  * 匹配规则（优先级从低到高）：
//  *   1. DEFAULT状态的样式（始终作为基础）
//  *   2. 与当前状态有交集的单状态规则（按绑定顺序）
//  *   3. 匹配位数越多的规则优先级越高（更精确的状态组合优先）
//  *
//  * 调用时机：每次状态发生变化后，在渲染前调用此函数刷新控件样式槽
//  *===========================================================================*/
// __IPGUI_API__ void ipgui_widget_state_resolve(const ipgui_widget_state_t * ws,
//                                               ipgui_widget_style_t * style_out);

// /*============================================================================
//  * 便捷宏：常用状态操作
//  *===========================================================================*/
// #define IPGUI_WIDGET_PRESS(ws)      ipgui_widget_state_add((ws),    IPGUI_STATE_PRESSED)
// #define IPGUI_WIDGET_RELEASE(ws)    ipgui_widget_state_remove((ws), IPGUI_STATE_PRESSED)
// #define IPGUI_WIDGET_FOCUS(ws)      ipgui_widget_state_add((ws),    IPGUI_STATE_FOCUSED)
// #define IPGUI_WIDGET_UNFOCUS(ws)    ipgui_widget_state_remove((ws), IPGUI_STATE_FOCUSED)
// #define IPGUI_WIDGET_DISABLE(ws)    ipgui_widget_state_add((ws),    IPGUI_STATE_DISABLED)
// #define IPGUI_WIDGET_ENABLE(ws)     ipgui_widget_state_remove((ws), IPGUI_STATE_DISABLED)
// #define IPGUI_WIDGET_CHECK(ws)      ipgui_widget_state_add((ws),    IPGUI_STATE_CHECKED)
// #define IPGUI_WIDGET_UNCHECK(ws)    ipgui_widget_state_remove((ws), IPGUI_STATE_CHECKED)

// #ifdef __cplusplus
// }
// #endif

// #endif /* IPGUI_WIDGET_STATE_H */