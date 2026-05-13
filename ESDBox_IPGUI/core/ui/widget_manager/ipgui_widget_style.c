// #include "ipgui_widget_style.h"
// #include "ipgui_memory.h"
// #include "ipgui_debug.h"

// /*============================================================================
//  * 默认值定义
//  *===========================================================================*/
// #define DEFAULT_BG_ALPHA            255
// #define DEFAULT_BG_GRAD_DIR         IPGUI_GRAD_DIR_NONE
// #define DEFAULT_BG_IMAGE_MODE       IPGUI_IMAGE_MODE_NONE
// #define DEFAULT_BG_IMAGE_OPACITY    255
// #define DEFAULT_BG_IMAGE_LERP       IPGUI_IMAGE_LERP_NEAREST
// #define DEFAULT_RADIUS              0
// #define DEFAULT_PADDING             0
// #define DEFAULT_BORDER_WIDTH        0
// #define DEFAULT_BORDER_ALPHA        255
// #define DEFAULT_SHADOW_ALPHA        0       /* alpha=0即不显示 */
// #define DEFAULT_SHADOW_BLUR         0
// #define DEFAULT_SHADOW_SPREAD       0
// #define DEFAULT_SHADOW_OFFSET       0
// #define DEFAULT_TEXT_ALPHA          255
// #define DEFAULT_TEXT_ALIGN          IPGUI_TEXT_ALIGN_LEFT
// #define DEFAULT_TEXT_LINE_SPACING   0
// #define DEFAULT_LAYOUT_SIZE         0
// #define DEFAULT_LAYOUT_MARGIN       0
// #define DEFAULT_LAYOUT_POS          0

// /*============================================================================
//  * 全局样式表（静态分配，嵌入式友好）
//  *===========================================================================*/
// __IPGUI_STATIC__ ipgui_stylesheet_t g_stylesheet;

// __IPGUI_API__ void ipgui_stylesheet_init(void)
// {
//     ipgui_memset(&g_stylesheet, 0, sizeof(g_stylesheet));
// }

// __IPGUI_API__ ipgui_style_t * ipgui_stylesheet_register(int id)
// {
//     if (id < 0 || id >= IPGUI_STYLESHEET_MAX) {
//         ipgui_dbg_error("style id out of range\r\n");
//         return (ipgui_style_t *)0;
//     }
//     ipgui_style_init(&g_stylesheet.styles[id]);
//     g_stylesheet.used[id] = 1;
//     return &g_stylesheet.styles[id];
// }

// __IPGUI_API__ ipgui_style_t * ipgui_stylesheet_get(int id)
// {
//     if (id < 0 || id >= IPGUI_STYLESHEET_MAX) return (ipgui_style_t *)0;
//     if (!g_stylesheet.used[id]) return (ipgui_style_t *)0;
//     return &g_stylesheet.styles[id];
// }

// __IPGUI_API__ void ipgui_stylesheet_unregister(int id)
// {
//     if (id < 0 || id >= IPGUI_STYLESHEET_MAX) return;
//     ipgui_style_init(&g_stylesheet.styles[id]);
//     g_stylesheet.used[id] = 0;
// }

// /*============================================================================
//  * 样式初始化 / 拷贝 / 合并
//  *===========================================================================*/
// __IPGUI_API__ void ipgui_style_init(ipgui_style_t * style)
// {
//     if (!style) return;
//     ipgui_memset(style, 0, sizeof(ipgui_style_t));
// }

// __IPGUI_API__ void ipgui_style_copy(ipgui_style_t * dst, const ipgui_style_t * src)
// {
//     if (!dst || !src) return;
//     ipgui_memcpy(dst, src, sizeof(ipgui_style_t));
// }

// __IPGUI_API__ void ipgui_style_merge(ipgui_style_t * dst, const ipgui_style_t * src)
// {
//     /* 将src中已设置的属性，合并到dst中尚未设置的属性上 */
//     if (!dst || !src) return;
//     for (int i = 0; i < IPGUI_STYLE_PROP_WORDS; i++) {
//         /* 找出src已设置但dst未设置的属性位 */
//         unsigned int to_copy = src->prop_map[i] & ~dst->prop_map[i];
//         if (!to_copy) continue;
//         /* 逐位处理 */
//         for (int bit = 0; bit < 32; bit++) {
//             if (!(to_copy & (1u << bit))) continue;
//             int prop_id = i * 32 + bit;
//             if (prop_id >= IPGUI_STYLE_PROP_MAX) break;
//             /* 根据prop_id拷贝对应字段 */
//             switch (prop_id) {
//             case IPGUI_STYLE_PROP_BG_COLOR:          dst->bg_color           = src->bg_color;           break;
//             case IPGUI_STYLE_PROP_BG_ALPHA:          dst->bg_alpha           = src->bg_alpha;           break;
//             case IPGUI_STYLE_PROP_BG_GRAD_COLOR:     dst->bg_grad_color      = src->bg_grad_color;      break;
//             case IPGUI_STYLE_PROP_BG_GRAD_DIR:       dst->bg_grad_dir        = src->bg_grad_dir;        break;
//             case IPGUI_STYLE_PROP_BG_IMAGE:          dst->bg_image           = src->bg_image;           break;
//             case IPGUI_STYLE_PROP_BG_IMAGE_MODE:     dst->bg_image_mode      = src->bg_image_mode;      break;
//             case IPGUI_STYLE_PROP_BG_IMAGE_OPACITY:  dst->bg_image_opacity   = src->bg_image_opacity;   break;
//             case IPGUI_STYLE_PROP_BG_IMAGE_LERP:     dst->bg_image_lerp      = src->bg_image_lerp;      break;
//             case IPGUI_STYLE_PROP_RADIUS:            dst->radius             = src->radius;             break;
//             case IPGUI_STYLE_PROP_PADDING_TOP:       dst->top_padding        = src->top_padding;        break;
//             case IPGUI_STYLE_PROP_PADDING_BOTTOM:    dst->bottom_padding     = src->bottom_padding;     break;
//             case IPGUI_STYLE_PROP_PADDING_LEFT:      dst->left_padding       = src->left_padding;       break;
//             case IPGUI_STYLE_PROP_PADDING_RIGHT:     dst->right_padding      = src->right_padding;      break;
//             case IPGUI_STYLE_PROP_BORDER_WIDTH:      dst->border_width       = src->border_width;       break;
//             case IPGUI_STYLE_PROP_BORDER_COLOR:      dst->border_color       = src->border_color;       break;
//             case IPGUI_STYLE_PROP_BORDER_ALPHA:      dst->border_alpha       = src->border_alpha;       break;
//             case IPGUI_STYLE_PROP_SHADOW_OUT_COLOR:  dst->shadow_out_color   = src->shadow_out_color;   break;
//             case IPGUI_STYLE_PROP_SHADOW_OUT_ALPHA:  dst->shadow_out_alpha   = src->shadow_out_alpha;   break;
//             case IPGUI_STYLE_PROP_SHADOW_OUT_BLUR:   dst->shadow_out_blur    = src->shadow_out_blur;    break;
//             case IPGUI_STYLE_PROP_SHADOW_OUT_SPREAD: dst->shadow_out_spread  = src->shadow_out_spread;  break;
//             case IPGUI_STYLE_PROP_SHADOW_OUT_OFFX:   dst->shadow_out_offset_x= src->shadow_out_offset_x;break;
//             case IPGUI_STYLE_PROP_SHADOW_OUT_OFFY:   dst->shadow_out_offset_y= src->shadow_out_offset_y;break;
//             case IPGUI_STYLE_PROP_SHADOW_IN_COLOR:   dst->shadow_in_color    = src->shadow_in_color;    break;
//             case IPGUI_STYLE_PROP_SHADOW_IN_ALPHA:   dst->shadow_in_alpha    = src->shadow_in_alpha;    break;
//             case IPGUI_STYLE_PROP_SHADOW_IN_BLUR:    dst->shadow_in_blur     = src->shadow_in_blur;     break;
//             case IPGUI_STYLE_PROP_SHADOW_IN_SPREAD:  dst->shadow_in_spread   = src->shadow_in_spread;   break;
//             case IPGUI_STYLE_PROP_SHADOW_IN_OFFX:    dst->shadow_in_offset_x = src->shadow_in_offset_x; break;
//             case IPGUI_STYLE_PROP_SHADOW_IN_OFFY:    dst->shadow_in_offset_y = src->shadow_in_offset_y; break;
//             case IPGUI_STYLE_PROP_TEXT_COLOR:        dst->text_color         = src->text_color;         break;
//             case IPGUI_STYLE_PROP_TEXT_ALPHA:        dst->text_alpha         = src->text_alpha;         break;
//             case IPGUI_STYLE_PROP_TEXT_FONT:         dst->text_font          = src->text_font;          break;
//             case IPGUI_STYLE_PROP_TEXT_ALIGN:        dst->text_align         = src->text_align;         break;
//             case IPGUI_STYLE_PROP_TEXT_LINE_SPACING: dst->text_line_spacing  = src->text_line_spacing;  break;
//             case IPGUI_STYLE_PROP_LAYOUT_WIDTH:      dst->layout_width       = src->layout_width;       break;
//             case IPGUI_STYLE_PROP_LAYOUT_HEIGHT:     dst->layout_height      = src->layout_height;      break;
//             case IPGUI_STYLE_PROP_LAYOUT_MARGIN_TOP: dst->margin_top         = src->margin_top;         break;
//             case IPGUI_STYLE_PROP_LAYOUT_MARGIN_BOT: dst->margin_bottom      = src->margin_bottom;      break;
//             case IPGUI_STYLE_PROP_LAYOUT_MARGIN_L:   dst->margin_left        = src->margin_left;        break;
//             case IPGUI_STYLE_PROP_LAYOUT_MARGIN_R:   dst->margin_right       = src->margin_right;       break;
//             case IPGUI_STYLE_PROP_LAYOUT_POS_X:      dst->pos_x              = src->pos_x;              break;
//             case IPGUI_STYLE_PROP_LAYOUT_POS_Y:      dst->pos_y              = src->pos_y;              break;
//             default: break;
//             }
//             IPGUI_STYLE_PROP_SET(dst->prop_map, prop_id);
//         }
//     }
// }

// /*============================================================================
//  * 继承链操作
//  *===========================================================================*/
// __IPGUI_API__ void ipgui_style_set_parent(ipgui_style_t * style, const ipgui_style_t * parent)
// {
//     if (!style) return;
//     /* 防止循环继承 */
//     const ipgui_style_t * p = parent;
//     while (p) {
//         if (p == style) {
//             ipgui_dbg_error("style: circular inheritance detected\r\n");
//             return;
//         }
//         p = p->parent;
//     }
//     style->parent = parent;
// }

// __IPGUI_API__ const ipgui_style_t * ipgui_style_get_parent(const ipgui_style_t * style)
// {
//     if (!style) return (const ipgui_style_t *)0;
//     return style->parent;
// }

// /*============================================================================
//  * 控件样式槽操作
//  *===========================================================================*/
// __IPGUI_API__ void ipgui_widget_style_init(ipgui_widget_style_t * ws)
// {
//     if (!ws) return;
//     for (int i = 0; i < IPGUI_WIDGET_STYLE_SLOTS; i++)
//         ws->ids[i] = -1;
//     ws->count = 0;
// }

// __IPGUI_API__ int ipgui_widget_style_apply(ipgui_widget_style_t * ws, int style_id)
// {
//     if (!ws) return -1;
//     /* 如果已经存在，先移除再重新追加到末尾（保证优先级正确） */
//     ipgui_widget_style_remove(ws, style_id);
//     if (ws->count >= IPGUI_WIDGET_STYLE_SLOTS) {
//         ipgui_dbg_error("style: widget style slots full\r\n");
//         return -1;
//     }
//     ws->ids[ws->count++] = style_id;
//     return 0;
// }

// __IPGUI_API__ void ipgui_widget_style_remove(ipgui_widget_style_t * ws, int style_id)
// {
//     if (!ws) return;
//     for (int i = 0; i < ws->count; i++) {
//         if (ws->ids[i] == style_id) {
//             /* 后面的往前移 */
//             for (int j = i; j < ws->count - 1; j++)
//                 ws->ids[j] = ws->ids[j + 1];
//             ws->ids[--ws->count] = -1;
//             return;
//         }
//     }
// }

// __IPGUI_API__ void ipgui_widget_style_clear(ipgui_widget_style_t * ws)
// {
//     if (!ws) return;
//     for (int i = 0; i < IPGUI_WIDGET_STYLE_SLOTS; i++)
//         ws->ids[i] = -1;
//     ws->count = 0;
// }

// /*============================================================================
//  * 属性查找核心
//  *
//  * 在单个样式及其继承链中查找prop_id，找到返回1并将找到的样式填充到out
//  *===========================================================================*/
// __IPGUI_STATIC__ int ipgui_style_find_prop(const ipgui_style_t * style, int prop_id,
//                                             ipgui_style_t * out)
// {
//     /* 沿继承链向上查找 */
//     const ipgui_style_t * cur = style;
//     while (cur) {
//         if (IPGUI_STYLE_PROP_GET(cur->prop_map, prop_id)) {
//             if (out) *out = *cur;
//             return 1;
//         }
//         cur = cur->parent;
//     }
//     return 0;
// }

// __IPGUI_API__ int ipgui_widget_style_find_prop(const ipgui_widget_style_t * ws, int prop_id,
//                                                 ipgui_style_t * out)
// {
//     if (!ws) return 0;
//     /* 从高优先级（末尾）往低优先级（开头）查找 */
//     for (int i = ws->count - 1; i >= 0; i--) {
//         const ipgui_style_t * s = ipgui_stylesheet_get(ws->ids[i]);
//         if (!s) continue;
//         if (ipgui_style_find_prop(s, prop_id, out))
//             return 1;
//     }
//     return 0;
// }

// /*============================================================================
//  * setter 实现
//  *===========================================================================*/

// /* 背景 */
// __IPGUI_API__ void ipgui_style_set_bg_color(ipgui_style_t * s, color_test_t color)
// {
//     if (!s) return;
//     s->bg_color = color;
//     IPGUI_STYLE_PROP_SET(s->prop_map, IPGUI_STYLE_PROP_BG_COLOR);
// }

// __IPGUI_API__ void ipgui_style_set_bg_alpha(ipgui_style_t * s, unsigned char alpha)
// {
//     if (!s) return;
//     s->bg_alpha = alpha;
//     IPGUI_STYLE_PROP_SET(s->prop_map, IPGUI_STYLE_PROP_BG_ALPHA);
// }

// __IPGUI_API__ void ipgui_style_set_bg_grad(ipgui_style_t * s, color_test_t end_color, ipgui_grad_dir_t dir)
// {
//     if (!s) return;
//     s->bg_grad_color = end_color;
//     s->bg_grad_dir   = dir;
//     IPGUI_STYLE_PROP_SET(s->prop_map, IPGUI_STYLE_PROP_BG_GRAD_COLOR);
//     IPGUI_STYLE_PROP_SET(s->prop_map, IPGUI_STYLE_PROP_BG_GRAD_DIR);
// }

// __IPGUI_API__ void ipgui_style_set_bg_image(ipgui_style_t * s, const ipgui_image_t * img, ipgui_image_mode_t mode)
// {
//     if (!s) return;
//     s->bg_image      = img;
//     s->bg_image_mode = mode;
//     IPGUI_STYLE_PROP_SET(s->prop_map, IPGUI_STYLE_PROP_BG_IMAGE);
//     IPGUI_STYLE_PROP_SET(s->prop_map, IPGUI_STYLE_PROP_BG_IMAGE_MODE);
// }

// __IPGUI_API__ void ipgui_style_set_bg_image_opacity(ipgui_style_t * s, unsigned char opacity)
// {
//     if (!s) return;
//     s->bg_image_opacity = opacity;
//     IPGUI_STYLE_PROP_SET(s->prop_map, IPGUI_STYLE_PROP_BG_IMAGE_OPACITY);
// }

// __IPGUI_API__ void ipgui_style_set_bg_image_lerp(ipgui_style_t * s, ipgui_image_lerp_t lerp)
// {
//     if (!s) return;
//     s->bg_image_lerp = lerp;
//     IPGUI_STYLE_PROP_SET(s->prop_map, IPGUI_STYLE_PROP_BG_IMAGE_LERP);
// }

// /* 形状 */
// __IPGUI_API__ void ipgui_style_set_radius(ipgui_style_t * s, ipgui_coord_t radius)
// {
//     if (!s) return;
//     s->radius = radius;
//     IPGUI_STYLE_PROP_SET(s->prop_map, IPGUI_STYLE_PROP_RADIUS);
// }

// __IPGUI_API__ void ipgui_style_set_padding(ipgui_style_t * s,
//     ipgui_coord_t top, ipgui_coord_t bottom,
//     ipgui_coord_t left, ipgui_coord_t right)
// {
//     if (!s) return;
//     s->top_padding    = top;
//     s->bottom_padding = bottom;
//     s->left_padding   = left;
//     s->right_padding  = right;
//     IPGUI_STYLE_PROP_SET(s->prop_map, IPGUI_STYLE_PROP_PADDING_TOP);
//     IPGUI_STYLE_PROP_SET(s->prop_map, IPGUI_STYLE_PROP_PADDING_BOTTOM);
//     IPGUI_STYLE_PROP_SET(s->prop_map, IPGUI_STYLE_PROP_PADDING_LEFT);
//     IPGUI_STYLE_PROP_SET(s->prop_map, IPGUI_STYLE_PROP_PADDING_RIGHT);
// }

// __IPGUI_API__ void ipgui_style_set_padding_all(ipgui_style_t * s, ipgui_coord_t padding)
// {
//     ipgui_style_set_padding(s, padding, padding, padding, padding);
// }

// /* 边框 */
// __IPGUI_API__ void ipgui_style_set_border(ipgui_style_t * s, ipgui_coord_t width,
//                                            color_test_t color, unsigned char alpha)
// {
//     ipgui_style_set_border_width(s, width);
//     ipgui_style_set_border_color(s, color);
//     ipgui_style_set_border_alpha(s, alpha);
// }

// __IPGUI_API__ void ipgui_style_set_border_width(ipgui_style_t * s, ipgui_coord_t width)
// {
//     if (!s) return;
//     s->border_width = width;
//     IPGUI_STYLE_PROP_SET(s->prop_map, IPGUI_STYLE_PROP_BORDER_WIDTH);
// }

// __IPGUI_API__ void ipgui_style_set_border_color(ipgui_style_t * s, color_test_t color)
// {
//     if (!s) return;
//     s->border_color = color;
//     IPGUI_STYLE_PROP_SET(s->prop_map, IPGUI_STYLE_PROP_BORDER_COLOR);
// }

// __IPGUI_API__ void ipgui_style_set_border_alpha(ipgui_style_t * s, unsigned char alpha)
// {
//     if (!s) return;
//     s->border_alpha = alpha;
//     IPGUI_STYLE_PROP_SET(s->prop_map, IPGUI_STYLE_PROP_BORDER_ALPHA);
// }

// /* 外阴影 */
// __IPGUI_API__ void ipgui_style_set_shadow_out(ipgui_style_t * s,
//     color_test_t color, unsigned char alpha,
//     ipgui_coord_t blur, ipgui_coord_t spread,
//     ipgui_coord_t offset_x, ipgui_coord_t offset_y)
// {
//     if (!s) return;
//     s->shadow_out_color    = color;
//     s->shadow_out_alpha    = alpha;
//     s->shadow_out_blur     = blur;
//     s->shadow_out_spread   = spread;
//     s->shadow_out_offset_x = offset_x;
//     s->shadow_out_offset_y = offset_y;
//     IPGUI_STYLE_PROP_SET(s->prop_map, IPGUI_STYLE_PROP_SHADOW_OUT_COLOR);
//     IPGUI_STYLE_PROP_SET(s->prop_map, IPGUI_STYLE_PROP_SHADOW_OUT_ALPHA);
//     IPGUI_STYLE_PROP_SET(s->prop_map, IPGUI_STYLE_PROP_SHADOW_OUT_BLUR);
//     IPGUI_STYLE_PROP_SET(s->prop_map, IPGUI_STYLE_PROP_SHADOW_OUT_SPREAD);
//     IPGUI_STYLE_PROP_SET(s->prop_map, IPGUI_STYLE_PROP_SHADOW_OUT_OFFX);
//     IPGUI_STYLE_PROP_SET(s->prop_map, IPGUI_STYLE_PROP_SHADOW_OUT_OFFY);
// }

// /* 内阴影 */
// __IPGUI_API__ void ipgui_style_set_shadow_in(ipgui_style_t * s,
//     color_test_t color, unsigned char alpha,
//     ipgui_coord_t blur, ipgui_coord_t spread,
//     ipgui_coord_t offset_x, ipgui_coord_t offset_y)
// {
//     if (!s) return;
//     s->shadow_in_color    = color;
//     s->shadow_in_alpha    = alpha;
//     s->shadow_in_blur     = blur;
//     s->shadow_in_spread   = spread;
//     s->shadow_in_offset_x = offset_x;
//     s->shadow_in_offset_y = offset_y;
//     IPGUI_STYLE_PROP_SET(s->prop_map, IPGUI_STYLE_PROP_SHADOW_IN_COLOR);
//     IPGUI_STYLE_PROP_SET(s->prop_map, IPGUI_STYLE_PROP_SHADOW_IN_ALPHA);
//     IPGUI_STYLE_PROP_SET(s->prop_map, IPGUI_STYLE_PROP_SHADOW_IN_BLUR);
//     IPGUI_STYLE_PROP_SET(s->prop_map, IPGUI_STYLE_PROP_SHADOW_IN_SPREAD);
//     IPGUI_STYLE_PROP_SET(s->prop_map, IPGUI_STYLE_PROP_SHADOW_IN_OFFX);
//     IPGUI_STYLE_PROP_SET(s->prop_map, IPGUI_STYLE_PROP_SHADOW_IN_OFFY);
// }

// /* 文字 */
// __IPGUI_API__ void ipgui_style_set_text_color(ipgui_style_t * s, color_test_t color)
// {
//     if (!s) return;
//     s->text_color = color;
//     IPGUI_STYLE_PROP_SET(s->prop_map, IPGUI_STYLE_PROP_TEXT_COLOR);
// }

// __IPGUI_API__ void ipgui_style_set_text_alpha(ipgui_style_t * s, unsigned char alpha)
// {
//     if (!s) return;
//     s->text_alpha = alpha;
//     IPGUI_STYLE_PROP_SET(s->prop_map, IPGUI_STYLE_PROP_TEXT_ALPHA);
// }

// __IPGUI_API__ void ipgui_style_set_text_font(ipgui_style_t * s, const ipgui_font_t * font)
// {
//     if (!s) return;
//     s->text_font = font;
//     IPGUI_STYLE_PROP_SET(s->prop_map, IPGUI_STYLE_PROP_TEXT_FONT);
// }

// __IPGUI_API__ void ipgui_style_set_text_align(ipgui_style_t * s, ipgui_text_align_t align)
// {
//     if (!s) return;
//     s->text_align = align;
//     IPGUI_STYLE_PROP_SET(s->prop_map, IPGUI_STYLE_PROP_TEXT_ALIGN);
// }

// __IPGUI_API__ void ipgui_style_set_text_line_spacing(ipgui_style_t * s, ipgui_coord_t spacing)
// {
//     if (!s) return;
//     s->text_line_spacing = spacing;
//     IPGUI_STYLE_PROP_SET(s->prop_map, IPGUI_STYLE_PROP_TEXT_LINE_SPACING);
// }

// /* 布局 */
// __IPGUI_API__ void ipgui_style_set_size(ipgui_style_t * s, ipgui_coord_t w, ipgui_coord_t h)
// {
//     if (!s) return;
//     s->layout_width  = w;
//     s->layout_height = h;
//     IPGUI_STYLE_PROP_SET(s->prop_map, IPGUI_STYLE_PROP_LAYOUT_WIDTH);
//     IPGUI_STYLE_PROP_SET(s->prop_map, IPGUI_STYLE_PROP_LAYOUT_HEIGHT);
// }

// __IPGUI_API__ void ipgui_style_set_pos(ipgui_style_t * s, ipgui_coord_t x, ipgui_coord_t y)
// {
//     if (!s) return;
//     s->pos_x = x;
//     s->pos_y = y;
//     IPGUI_STYLE_PROP_SET(s->prop_map, IPGUI_STYLE_PROP_LAYOUT_POS_X);
//     IPGUI_STYLE_PROP_SET(s->prop_map, IPGUI_STYLE_PROP_LAYOUT_POS_Y);
// }

// __IPGUI_API__ void ipgui_style_set_margin(ipgui_style_t * s,
//     ipgui_coord_t top, ipgui_coord_t bottom,
//     ipgui_coord_t left, ipgui_coord_t right)
// {
//     if (!s) return;
//     s->margin_top    = top;
//     s->margin_bottom = bottom;
//     s->margin_left   = left;
//     s->margin_right  = right;
//     IPGUI_STYLE_PROP_SET(s->prop_map, IPGUI_STYLE_PROP_LAYOUT_MARGIN_TOP);
//     IPGUI_STYLE_PROP_SET(s->prop_map, IPGUI_STYLE_PROP_LAYOUT_MARGIN_BOT);
//     IPGUI_STYLE_PROP_SET(s->prop_map, IPGUI_STYLE_PROP_LAYOUT_MARGIN_L);
//     IPGUI_STYLE_PROP_SET(s->prop_map, IPGUI_STYLE_PROP_LAYOUT_MARGIN_R);
// }

// __IPGUI_API__ void ipgui_style_set_margin_all(ipgui_style_t * s, ipgui_coord_t margin)
// {
//     ipgui_style_set_margin(s, margin, margin, margin, margin);
// }

// /*============================================================================
//  * getter 实现（统一模式：找到返回值，找不到返回默认值）
//  *===========================================================================*/

// /* 内部辅助宏，减少重复代码 */
// #define STYLE_GET_IMPL(ws, prop_id, field, default_val) \
//     do { \
//         ipgui_style_t _found; \
//         if (ipgui_widget_style_find_prop(ws, prop_id, &_found)) \
//             return _found.field; \
//         return (default_val); \
//     } while(0)

// /* 背景 */
// __IPGUI_API__ color_test_t ipgui_widget_style_get_bg_color(const ipgui_widget_style_t * ws)
// {
//     color_test_t def; ipgui_memset(&def, 0, sizeof(def));
//     STYLE_GET_IMPL(ws, IPGUI_STYLE_PROP_BG_COLOR, bg_color, def);
// }
// __IPGUI_API__ unsigned char ipgui_widget_style_get_bg_alpha(const ipgui_widget_style_t * ws)
// {
//     STYLE_GET_IMPL(ws, IPGUI_STYLE_PROP_BG_ALPHA, bg_alpha, DEFAULT_BG_ALPHA);
// }
// __IPGUI_API__ color_test_t ipgui_widget_style_get_bg_grad_color(const ipgui_widget_style_t * ws)
// {
//     color_test_t def; ipgui_memset(&def, 0, sizeof(def));
//     STYLE_GET_IMPL(ws, IPGUI_STYLE_PROP_BG_GRAD_COLOR, bg_grad_color, def);
// }
// __IPGUI_API__ ipgui_grad_dir_t ipgui_widget_style_get_bg_grad_dir(const ipgui_widget_style_t * ws)
// {
//     STYLE_GET_IMPL(ws, IPGUI_STYLE_PROP_BG_GRAD_DIR, bg_grad_dir, DEFAULT_BG_GRAD_DIR);
// }
// __IPGUI_API__ const ipgui_image_t * ipgui_widget_style_get_bg_image(const ipgui_widget_style_t * ws)
// {
//     STYLE_GET_IMPL(ws, IPGUI_STYLE_PROP_BG_IMAGE, bg_image, (const ipgui_image_t *)0);
// }
// __IPGUI_API__ ipgui_image_mode_t ipgui_widget_style_get_bg_image_mode(const ipgui_widget_style_t * ws)
// {
//     STYLE_GET_IMPL(ws, IPGUI_STYLE_PROP_BG_IMAGE_MODE, bg_image_mode, DEFAULT_BG_IMAGE_MODE);
// }
// __IPGUI_API__ unsigned char ipgui_widget_style_get_bg_image_opacity(const ipgui_widget_style_t * ws)
// {
//     STYLE_GET_IMPL(ws, IPGUI_STYLE_PROP_BG_IMAGE_OPACITY, bg_image_opacity, DEFAULT_BG_IMAGE_OPACITY);
// }
// __IPGUI_API__ ipgui_image_lerp_t ipgui_widget_style_get_bg_image_lerp(const ipgui_widget_style_t * ws)
// {
//     STYLE_GET_IMPL(ws, IPGUI_STYLE_PROP_BG_IMAGE_LERP, bg_image_lerp, DEFAULT_BG_IMAGE_LERP);
// }

// /* 形状 */
// __IPGUI_API__ ipgui_coord_t ipgui_widget_style_get_radius(const ipgui_widget_style_t * ws)
// {
//     STYLE_GET_IMPL(ws, IPGUI_STYLE_PROP_RADIUS, radius, DEFAULT_RADIUS);
// }
// __IPGUI_API__ ipgui_coord_t ipgui_widget_style_get_padding_top(const ipgui_widget_style_t * ws)
// {
//     STYLE_GET_IMPL(ws, IPGUI_STYLE_PROP_PADDING_TOP, top_padding, DEFAULT_PADDING);
// }
// __IPGUI_API__ ipgui_coord_t ipgui_widget_style_get_padding_bottom(const ipgui_widget_style_t * ws)
// {
//     STYLE_GET_IMPL(ws, IPGUI_STYLE_PROP_PADDING_BOTTOM, bottom_padding, DEFAULT_PADDING);
// }
// __IPGUI_API__ ipgui_coord_t ipgui_widget_style_get_padding_left(const ipgui_widget_style_t * ws)
// {
//     STYLE_GET_IMPL(ws, IPGUI_STYLE_PROP_PADDING_LEFT, left_padding, DEFAULT_PADDING);
// }
// __IPGUI_API__ ipgui_coord_t ipgui_widget_style_get_padding_right(const ipgui_widget_style_t * ws)
// {
//     STYLE_GET_IMPL(ws, IPGUI_STYLE_PROP_PADDING_RIGHT, right_padding, DEFAULT_PADDING);
// }

// /* 边框 */
// __IPGUI_API__ ipgui_coord_t ipgui_widget_style_get_border_width(const ipgui_widget_style_t * ws)
// {
//     STYLE_GET_IMPL(ws, IPGUI_STYLE_PROP_BORDER_WIDTH, border_width, DEFAULT_BORDER_WIDTH);
// }
// __IPGUI_API__ color_test_t ipgui_widget_style_get_border_color(const ipgui_widget_style_t * ws)
// {
//     color_test_t def; ipgui_memset(&def, 0, sizeof(def));
//     STYLE_GET_IMPL(ws, IPGUI_STYLE_PROP_BORDER_COLOR, border_color, def);
// }
// __IPGUI_API__ unsigned char ipgui_widget_style_get_border_alpha(const ipgui_widget_style_t * ws)
// {
//     STYLE_GET_IMPL(ws, IPGUI_STYLE_PROP_BORDER_ALPHA, border_alpha, DEFAULT_BORDER_ALPHA);
// }

// /* 外阴影 */
// __IPGUI_API__ color_test_t ipgui_widget_style_get_shadow_out_color(const ipgui_widget_style_t * ws)
// {
//     color_test_t def; ipgui_memset(&def, 0, sizeof(def));
//     STYLE_GET_IMPL(ws, IPGUI_STYLE_PROP_SHADOW_OUT_COLOR, shadow_out_color, def);
// }
// __IPGUI_API__ unsigned char ipgui_widget_style_get_shadow_out_alpha(const ipgui_widget_style_t * ws)
// {
//     STYLE_GET_IMPL(ws, IPGUI_STYLE_PROP_SHADOW_OUT_ALPHA, shadow_out_alpha, DEFAULT_SHADOW_ALPHA);
// }
// __IPGUI_API__ ipgui_coord_t ipgui_widget_style_get_shadow_out_blur(const ipgui_widget_style_t * ws)
// {
//     STYLE_GET_IMPL(ws, IPGUI_STYLE_PROP_SHADOW_OUT_BLUR, shadow_out_blur, DEFAULT_SHADOW_BLUR);
// }
// __IPGUI_API__ ipgui_coord_t ipgui_widget_style_get_shadow_out_spread(const ipgui_widget_style_t * ws)
// {
//     STYLE_GET_IMPL(ws, IPGUI_STYLE_PROP_SHADOW_OUT_SPREAD, shadow_out_spread, DEFAULT_SHADOW_SPREAD);
// }
// __IPGUI_API__ ipgui_coord_t ipgui_widget_style_get_shadow_out_offx(const ipgui_widget_style_t * ws)
// {
//     STYLE_GET_IMPL(ws, IPGUI_STYLE_PROP_SHADOW_OUT_OFFX, shadow_out_offset_x, DEFAULT_SHADOW_OFFSET);
// }
// __IPGUI_API__ ipgui_coord_t ipgui_widget_style_get_shadow_out_offy(const ipgui_widget_style_t * ws)
// {
//     STYLE_GET_IMPL(ws, IPGUI_STYLE_PROP_SHADOW_OUT_OFFY, shadow_out_offset_y, DEFAULT_SHADOW_OFFSET);
// }

// /* 内阴影 */
// __IPGUI_API__ color_test_t ipgui_widget_style_get_shadow_in_color(const ipgui_widget_style_t * ws)
// {
//     color_test_t def; ipgui_memset(&def, 0, sizeof(def));
//     STYLE_GET_IMPL(ws, IPGUI_STYLE_PROP_SHADOW_IN_COLOR, shadow_in_color, def);
// }
// __IPGUI_API__ unsigned char ipgui_widget_style_get_shadow_in_alpha(const ipgui_widget_style_t * ws)
// {
//     STYLE_GET_IMPL(ws, IPGUI_STYLE_PROP_SHADOW_IN_ALPHA, shadow_in_alpha, DEFAULT_SHADOW_ALPHA);
// }
// __IPGUI_API__ ipgui_coord_t ipgui_widget_style_get_shadow_in_blur(const ipgui_widget_style_t * ws)
// {
//     STYLE_GET_IMPL(ws, IPGUI_STYLE_PROP_SHADOW_IN_BLUR, shadow_in_blur, DEFAULT_SHADOW_BLUR);
// }
// __IPGUI_API__ ipgui_coord_t ipgui_widget_style_get_shadow_in_spread(const ipgui_widget_style_t * ws)
// {
//     STYLE_GET_IMPL(ws, IPGUI_STYLE_PROP_SHADOW_IN_SPREAD, shadow_in_spread, DEFAULT_SHADOW_SPREAD);
// }
// __IPGUI_API__ ipgui_coord_t ipgui_widget_style_get_shadow_in_offx(const ipgui_widget_style_t * ws)
// {
//     STYLE_GET_IMPL(ws, IPGUI_STYLE_PROP_SHADOW_IN_OFFX, shadow_in_offset_x, DEFAULT_SHADOW_OFFSET);
// }
// __IPGUI_API__ ipgui_coord_t ipgui_widget_style_get_shadow_in_offy(const ipgui_widget_style_t * ws)
// {
//     STYLE_GET_IMPL(ws, IPGUI_STYLE_PROP_SHADOW_IN_OFFY, shadow_in_offset_y, DEFAULT_SHADOW_OFFSET);
// }

// /* 文字 */
// __IPGUI_API__ color_test_t ipgui_widget_style_get_text_color(const ipgui_widget_style_t * ws)
// {
//     color_test_t def; ipgui_memset(&def, 0, sizeof(def));
//     STYLE_GET_IMPL(ws, IPGUI_STYLE_PROP_TEXT_COLOR, text_color, def);
// }
// __IPGUI_API__ unsigned char ipgui_widget_style_get_text_alpha(const ipgui_widget_style_t * ws)
// {
//     STYLE_GET_IMPL(ws, IPGUI_STYLE_PROP_TEXT_ALPHA, text_alpha, DEFAULT_TEXT_ALPHA);
// }
// __IPGUI_API__ const ipgui_font_t * ipgui_widget_style_get_text_font(const ipgui_widget_style_t * ws)
// {
//     STYLE_GET_IMPL(ws, IPGUI_STYLE_PROP_TEXT_FONT, text_font, (const ipgui_font_t *)0);
// }
// __IPGUI_API__ ipgui_text_align_t ipgui_widget_style_get_text_align(const ipgui_widget_style_t * ws)
// {
//     STYLE_GET_IMPL(ws, IPGUI_STYLE_PROP_TEXT_ALIGN, text_align, DEFAULT_TEXT_ALIGN);
// }
// __IPGUI_API__ ipgui_coord_t ipgui_widget_style_get_text_line_spacing(const ipgui_widget_style_t * ws)
// {
//     STYLE_GET_IMPL(ws, IPGUI_STYLE_PROP_TEXT_LINE_SPACING, text_line_spacing, DEFAULT_TEXT_LINE_SPACING);
// }

// /* 布局 */
// __IPGUI_API__ ipgui_coord_t ipgui_widget_style_get_layout_width(const ipgui_widget_style_t * ws)
// {
//     STYLE_GET_IMPL(ws, IPGUI_STYLE_PROP_LAYOUT_WIDTH, layout_width, DEFAULT_LAYOUT_SIZE);
// }
// __IPGUI_API__ ipgui_coord_t ipgui_widget_style_get_layout_height(const ipgui_widget_style_t * ws)
// {
//     STYLE_GET_IMPL(ws, IPGUI_STYLE_PROP_LAYOUT_HEIGHT, layout_height, DEFAULT_LAYOUT_SIZE);
// }
// __IPGUI_API__ ipgui_coord_t ipgui_widget_style_get_margin_top(const ipgui_widget_style_t * ws)
// {
//     STYLE_GET_IMPL(ws, IPGUI_STYLE_PROP_LAYOUT_MARGIN_TOP, margin_top, DEFAULT_LAYOUT_MARGIN);
// }
// __IPGUI_API__ ipgui_coord_t ipgui_widget_style_get_margin_bottom(const ipgui_widget_style_t * ws)
// {
//     STYLE_GET_IMPL(ws, IPGUI_STYLE_PROP_LAYOUT_MARGIN_BOT, margin_bottom, DEFAULT_LAYOUT_MARGIN);
// }
// __IPGUI_API__ ipgui_coord_t ipgui_widget_style_get_margin_left(const ipgui_widget_style_t * ws)
// {
//     STYLE_GET_IMPL(ws, IPGUI_STYLE_PROP_LAYOUT_MARGIN_L, margin_left, DEFAULT_LAYOUT_MARGIN);
// }
// __IPGUI_API__ ipgui_coord_t ipgui_widget_style_get_margin_right(const ipgui_widget_style_t * ws)
// {
//     STYLE_GET_IMPL(ws, IPGUI_STYLE_PROP_LAYOUT_MARGIN_R, margin_right, DEFAULT_LAYOUT_MARGIN);
// }
// __IPGUI_API__ ipgui_coord_t ipgui_widget_style_get_pos_x(const ipgui_widget_style_t * ws)
// {
//     STYLE_GET_IMPL(ws, IPGUI_STYLE_PROP_LAYOUT_POS_X, pos_x, DEFAULT_LAYOUT_POS);
// }
// __IPGUI_API__ ipgui_coord_t ipgui_widget_style_get_pos_y(const ipgui_widget_style_t * ws)
// {
//     STYLE_GET_IMPL(ws, IPGUI_STYLE_PROP_LAYOUT_POS_Y, pos_y, DEFAULT_LAYOUT_POS);
// }

// /*============================================================================
//  * 样式→渲染属性结构体转换
//  *===========================================================================*/
// __IPGUI_API__ void ipgui_widget_style_to_box_shape(const ipgui_widget_style_t * ws,
//                                                     const ipgui_aabb_t * content,
//                                                     ipgui_box_shape_attr_t * out)
// {
//     if (!out) return;
//     out->content   = *content;
//     out->radius    = ipgui_widget_style_get_radius(ws);
//     out->padding   = ipgui_widget_style_get_padding_top(ws); /* 暂用top padding作为统一padding */
// }

// __IPGUI_API__ void ipgui_widget_style_to_bg_color(const ipgui_widget_style_t * ws,
//                                                    ipgui_box_bg_color_attr_t * out)
// {
//     if (!out) return;
//     out->color = ipgui_widget_style_get_bg_color(ws);
//     out->alpha = ipgui_widget_style_get_bg_alpha(ws);
// }

// __IPGUI_API__ void ipgui_widget_style_to_border(const ipgui_widget_style_t * ws,
//                                                  ipgui_box_border_attr_t * out)
// {
//     if (!out) return;
//     out->width = ipgui_widget_style_get_border_width(ws);
//     out->color = ipgui_widget_style_get_border_color(ws);
//     out->alpha = ipgui_widget_style_get_border_alpha(ws);
// }

// __IPGUI_API__ void ipgui_widget_style_to_shadow_out(const ipgui_widget_style_t * ws,
//                                                      ipgui_box_outer_shadow_attr_t * out)
// {
//     if (!out) return;
//     out->color    = ipgui_widget_style_get_shadow_out_color(ws);
//     out->alpha    = ipgui_widget_style_get_shadow_out_alpha(ws);
//     out->blur     = ipgui_widget_style_get_shadow_out_blur(ws);
//     out->spread   = ipgui_widget_style_get_shadow_out_spread(ws);
//     out->offset_x = ipgui_widget_style_get_shadow_out_offx(ws);
//     out->offset_y = ipgui_widget_style_get_shadow_out_offy(ws);
// }

// __IPGUI_API__ void ipgui_widget_style_to_shadow_in(const ipgui_widget_style_t * ws,
//                                                     ipgui_box_inner_shadow_attr_t * out)
// {
//     if (!out) return;
//     out->color    = ipgui_widget_style_get_shadow_in_color(ws);
//     out->alpha    = ipgui_widget_style_get_shadow_in_alpha(ws);
//     out->blur     = ipgui_widget_style_get_shadow_in_blur(ws);
//     out->spread   = ipgui_widget_style_get_shadow_in_spread(ws);
//     out->offset_x = ipgui_widget_style_get_shadow_in_offx(ws);
//     out->offset_y = ipgui_widget_style_get_shadow_in_offy(ws);
// }

// __IPGUI_API__ void ipgui_widget_style_to_font_attr(const ipgui_widget_style_t * ws,
//                                                     ipgui_font_ras_attr_t * out)
// {
//     if (!out) return;
//     out->color         = ipgui_widget_style_get_text_color(ws);
//     out->alpha         = ipgui_widget_style_get_text_alpha(ws);
//     out->font          = ipgui_widget_style_get_text_font(ws);
//     out->line_spacing  = ipgui_widget_style_get_text_line_spacing(ws);
// }