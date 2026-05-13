// /* 可见/隐藏

// 启用/禁用

// 焦点状态

// 按下/悬停/选中

// 脏标记 (需要重绘) */
// #include "ipgui_widget_state.h"
// #include "ipgui_debug.h"

// /*============================================================================
//  * 内部辅助：统计bitmask中1的个数（用于优先级排序）
//  *===========================================================================*/
// __IPGUI_STATIC__ __IPGUI_INLINE__ int popcount(ipgui_state_flags_t x)
// {
//     int count = 0;
//     while (x) {
//         count += (x & 1);
//         x >>= 1;
//     }
//     return count;
// }

// /*============================================================================
//  * 初始化
//  *===========================================================================*/
// __IPGUI_API__ void ipgui_widget_state_init(ipgui_widget_state_t * ws)
// {
//     if (!ws) return;
//     ws->current       = IPGUI_STATE_DEFAULT;
//     ws->previous      = IPGUI_STATE_DEFAULT;
//     ws->entry_count   = 0;
//     ws->trans_running = 0;
//     ws->trans_start_cb= (void *)0;
//     for (int i = 0; i < IPGUI_STATE_STYLE_ENTRY_MAX; i++) {
//         ws->entries[i].state     = IPGUI_STATE_DEFAULT;
//         ws->entries[i].style_id  = -1;
//         ws->entries[i].trans     = (const ipgui_state_trans_t *)0;
//         ws->entries[i].trans_num = 0;
//     }
// }

// /*============================================================================
//  * 状态-样式绑定
//  *===========================================================================*/
// __IPGUI_API__ int ipgui_widget_state_bind(ipgui_widget_state_t * ws,
//                                           ipgui_state_flags_t state,
//                                           int style_id,
//                                           const ipgui_state_trans_t * trans,
//                                           unsigned char trans_num)
// {
//     if (!ws) return -1;

//     /* 如果该state已存在绑定，直接覆盖 */
//     for (int i = 0; i < ws->entry_count; i++) {
//         if (ws->entries[i].state == state) {
//             ws->entries[i].style_id  = style_id;
//             ws->entries[i].trans     = trans;
//             ws->entries[i].trans_num = trans_num;
//             return 0;
//         }
//     }

//     /* 新增 */
//     if (ws->entry_count >= IPGUI_STATE_STYLE_ENTRY_MAX) {
//         ipgui_dbg_error("widget_state: entry slots full\r\n");
//         return -1;
//     }
//     ws->entries[ws->entry_count].state     = state;
//     ws->entries[ws->entry_count].style_id  = style_id;
//     ws->entries[ws->entry_count].trans     = trans;
//     ws->entries[ws->entry_count].trans_num = trans_num;
//     ws->entry_count++;
//     return 0;
// }

// __IPGUI_API__ void ipgui_widget_state_unbind(ipgui_widget_state_t * ws,
//                                              ipgui_state_flags_t state)
// {
//     if (!ws) return;
//     for (int i = 0; i < ws->entry_count; i++) {
//         if (ws->entries[i].state == state) {
//             /* 后面的往前移 */
//             for (int j = i; j < ws->entry_count - 1; j++)
//                 ws->entries[j] = ws->entries[j + 1];
//             ws->entry_count--;
//             ws->entries[ws->entry_count].state    = IPGUI_STATE_DEFAULT;
//             ws->entries[ws->entry_count].style_id = -1;
//             return;
//         }
//     }
// }

// __IPGUI_API__ void ipgui_widget_state_unbind_all(ipgui_widget_state_t * ws)
// {
//     if (!ws) return;
//     ws->entry_count = 0;
//     for (int i = 0; i < IPGUI_STATE_STYLE_ENTRY_MAX; i++) {
//         ws->entries[i].state     = IPGUI_STATE_DEFAULT;
//         ws->entries[i].style_id  = -1;
//         ws->entries[i].trans     = (const ipgui_state_trans_t *)0;
//         ws->entries[i].trans_num = 0;
//     }
// }

// /*============================================================================
//  * 状态切换（内部统一走这个函数，方便以后接入动画）
//  *===========================================================================*/
// __IPGUI_STATIC__ void state_transition(ipgui_widget_state_t * ws,
//                                        ipgui_state_flags_t new_state)
// {
//     if (ws->current == new_state) return;

//     ws->previous = ws->current;
//     ws->current  = new_state;

//     /* 过渡动画预留入口：
//      * 如果注册了trans_start_cb，找出新旧状态变化涉及的条目，
//      * 将有过渡配置的条目触发回调
//      * 当前不实现，仅预留调用点
//      */
//     if (ws->trans_start_cb) {
//         /* 找出当前状态匹配的条目中有trans配置的 */
//         for (int i = 0; i < ws->entry_count; i++) {
//             ipgui_state_style_entry_t * e = &ws->entries[i];
//             if (e->trans == (const ipgui_state_trans_t *)0) continue;
//             if (e->trans_num == 0) continue;

//             /* 判断该条目是否与新状态有关联（新增或移除） */
//             int was_active = (e->state == IPGUI_STATE_DEFAULT) ||
//                              ((ws->previous & e->state) == e->state);
//             int now_active = (e->state == IPGUI_STATE_DEFAULT) ||
//                              ((ws->current  & e->state) == e->state);

//             if (was_active != now_active) {
//                 /* 状态发生了变化，触发过渡动画回调（预留，widget指针由外部传入，这里传NULL） */
//                 ws->trans_start_cb((void *)0,
//                                    ws->previous,
//                                    ws->current,
//                                    e->trans,
//                                    e->trans_num);
//             }
//         }
//     }
// }

// __IPGUI_API__ void ipgui_widget_state_add(ipgui_widget_state_t * ws,
//                                           ipgui_state_flags_t state)
// {
//     if (!ws) return;
//     state_transition(ws, ws->current | state);
// }

// __IPGUI_API__ void ipgui_widget_state_remove(ipgui_widget_state_t * ws,
//                                              ipgui_state_flags_t state)
// {
//     if (!ws) return;
//     /* DEFAULT状态不可移除 */
//     state_transition(ws, ws->current & ~state & ~IPGUI_STATE_DEFAULT);
//     /* 保证DEFAULT始终存在 */
//     ws->current |= IPGUI_STATE_DEFAULT;
// }

// __IPGUI_API__ void ipgui_widget_state_set(ipgui_widget_state_t * ws,
//                                           ipgui_state_flags_t state)
// {
//     if (!ws) return;
//     /* 始终保留DEFAULT */
//     state_transition(ws, state | IPGUI_STATE_DEFAULT);
// }

// __IPGUI_API__ int ipgui_widget_state_has(const ipgui_widget_state_t * ws,
//                                          ipgui_state_flags_t state)
// {
//     if (!ws) return 0;
//     return (ws->current & state) == state;
// }

// __IPGUI_API__ ipgui_state_flags_t ipgui_widget_state_get(const ipgui_widget_state_t * ws)
// {
//     if (!ws) return IPGUI_STATE_DEFAULT;
//     return ws->current;
// }

// /*============================================================================
//  * 过渡动画回调注册（预留）
//  *===========================================================================*/
// __IPGUI_API__ void ipgui_widget_state_set_trans_cb(
//     ipgui_widget_state_t * ws,
//     void (* cb)(void * widget,
//                 ipgui_state_flags_t old_state,
//                 ipgui_state_flags_t new_state,
//                 const ipgui_state_trans_t * trans,
//                 unsigned char trans_num))
// {
//     if (!ws) return;
//     ws->trans_start_cb = cb;
// }

// /*============================================================================
//  * 样式解析核心
//  *
//  * 匹配规则：
//  *   1. DEFAULT条目始终作为最低优先级基础
//  *   2. 遍历所有条目，筛选出与当前状态完全匹配（entry->state是current的子集）的条目
//  *   3. 按匹配的bit数从少到多排序后依次apply（bit数越多=越精确=优先级越高）
//  *   4. 最终style_out反映的是高优先级覆盖低优先级后的结果
//  *===========================================================================*/

// /* 用于临时排序的结构 */
// typedef struct {
//     int style_id;
//     int match_bits; /* 匹配的状态bit数，越多优先级越高 */
// } resolve_item_t;

// __IPGUI_API__ void ipgui_widget_state_resolve(const ipgui_widget_state_t * ws,
//                                               ipgui_widget_style_t * style_out)
// {
//     if (!ws || !style_out) return;

//     ipgui_widget_style_init(style_out);

//     /* step1: 收集所有匹配当前状态的条目 */
//     resolve_item_t matched[IPGUI_STATE_STYLE_ENTRY_MAX];
//     int matched_count = 0;

//     for (int i = 0; i < ws->entry_count; i++) {
//         const ipgui_state_style_entry_t * e = &ws->entries[i];
//         if (e->style_id < 0) continue;

//         int is_match = 0;
//         if (e->state == IPGUI_STATE_DEFAULT) {
//             /* DEFAULT条目始终匹配 */
//             is_match = 1;
//         } else {
//             /* entry->state必须是current的子集才算匹配 */
//             is_match = ((ws->current & e->state) == e->state);
//         }

//         if (is_match) {
//             matched[matched_count].style_id   = e->style_id;
//             matched[matched_count].match_bits = popcount(e->state);
//             matched_count++;
//         }
//     }

//     if (matched_count == 0) return;

//     /* step2: 按match_bits从小到大排序（插入排序，数量少性能够用） */
//     for (int i = 1; i < matched_count; i++) {
//         resolve_item_t key = matched[i];
//         int j = i - 1;
//         while (j >= 0 && matched[j].match_bits > key.match_bits) {
//             matched[j + 1] = matched[j];
//             j--;
//         }
//         matched[j + 1] = key;
//     }

//     /* step3: 按优先级从低到高依次apply到style_out
//      * 后apply的优先级高，会覆盖前面的
//      */
//     for (int i = 0; i < matched_count; i++) {
//         ipgui_widget_style_apply(style_out, matched[i].style_id);
//     }
// }