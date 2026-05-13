// #ifndef IPGUI_STYLE_H
// #define IPGUI_STYLE_H

// #include "ipgui_types.h"
// #include "ipgui_box_ras.h"
// #include "ipgui_draw_builtin_font.h"
// #include "ipgui_image_ras.h"

// #ifdef __cplusplus
// extern "C" {
// #endif

// /*============================================================================
//  * 样式属性ID（每个属性对应一个bit，用于标记哪些属性被显式设置了）
//  * 分为若干组，每组32个属性
//  *===========================================================================*/

// /* --- 第0组：背景 --- */
// #define IPGUI_STYLE_PROP_BG_COLOR           (0)
// #define IPGUI_STYLE_PROP_BG_ALPHA           (1)
// #define IPGUI_STYLE_PROP_BG_GRAD_COLOR      (2)   /* 渐变终止色 */
// #define IPGUI_STYLE_PROP_BG_GRAD_DIR        (3)   /* 渐变方向 */
// #define IPGUI_STYLE_PROP_BG_IMAGE           (4)   /* 背景图片指针 */
// #define IPGUI_STYLE_PROP_BG_IMAGE_MODE      (5)
// #define IPGUI_STYLE_PROP_BG_IMAGE_OPACITY   (6)
// #define IPGUI_STYLE_PROP_BG_IMAGE_LERP      (7)

// /* --- 第1组：盒子形状 --- */
// #define IPGUI_STYLE_PROP_RADIUS             (8)
// #define IPGUI_STYLE_PROP_PADDING_TOP        (9)
// #define IPGUI_STYLE_PROP_PADDING_BOTTOM     (10)
// #define IPGUI_STYLE_PROP_PADDING_LEFT       (11)
// #define IPGUI_STYLE_PROP_PADDING_RIGHT      (12)

// /* --- 第2组：边框 --- */
// #define IPGUI_STYLE_PROP_BORDER_WIDTH       (13)
// #define IPGUI_STYLE_PROP_BORDER_COLOR       (14)
// #define IPGUI_STYLE_PROP_BORDER_ALPHA       (15)

// /* --- 第3组：外阴影 --- */
// #define IPGUI_STYLE_PROP_SHADOW_OUT_COLOR   (16)
// #define IPGUI_STYLE_PROP_SHADOW_OUT_ALPHA   (17)
// #define IPGUI_STYLE_PROP_SHADOW_OUT_BLUR    (18)
// #define IPGUI_STYLE_PROP_SHADOW_OUT_SPREAD  (19)
// #define IPGUI_STYLE_PROP_SHADOW_OUT_OFFX    (20)
// #define IPGUI_STYLE_PROP_SHADOW_OUT_OFFY    (21)

// /* --- 第4组：内阴影 --- */
// #define IPGUI_STYLE_PROP_SHADOW_IN_COLOR    (22)
// #define IPGUI_STYLE_PROP_SHADOW_IN_ALPHA    (23)
// #define IPGUI_STYLE_PROP_SHADOW_IN_BLUR     (24)
// #define IPGUI_STYLE_PROP_SHADOW_IN_SPREAD   (25)
// #define IPGUI_STYLE_PROP_SHADOW_IN_OFFX     (26)
// #define IPGUI_STYLE_PROP_SHADOW_IN_OFFY     (27)

// /* --- 第5组：文字 --- */
// #define IPGUI_STYLE_PROP_TEXT_COLOR         (28)
// #define IPGUI_STYLE_PROP_TEXT_ALPHA         (29)
// #define IPGUI_STYLE_PROP_TEXT_FONT          (30)
// #define IPGUI_STYLE_PROP_TEXT_ALIGN         (31)

// /* --- 第6组：文字（续） + 布局 --- */
// #define IPGUI_STYLE_PROP_TEXT_LINE_SPACING  (32)
// #define IPGUI_STYLE_PROP_LAYOUT_WIDTH       (33)
// #define IPGUI_STYLE_PROP_LAYOUT_HEIGHT      (34)
// #define IPGUI_STYLE_PROP_LAYOUT_MARGIN_TOP  (35)
// #define IPGUI_STYLE_PROP_LAYOUT_MARGIN_BOT  (36)
// #define IPGUI_STYLE_PROP_LAYOUT_MARGIN_L    (37)
// #define IPGUI_STYLE_PROP_LAYOUT_MARGIN_R    (38)
// #define IPGUI_STYLE_PROP_LAYOUT_POS_X       (39)
// #define IPGUI_STYLE_PROP_LAYOUT_POS_Y       (40)

// #define IPGUI_STYLE_PROP_MAX                (41)
// #define IPGUI_STYLE_PROP_WORDS              ((IPGUI_STYLE_PROP_MAX + 31) / 32)

// /*============================================================================
//  * 辅助宏：属性标记位操作
//  *===========================================================================*/
// #define IPGUI_STYLE_PROP_SET(map, prop)   ((map)[(prop) >> 5] |=  (1u << ((prop) & 31)))
// #define IPGUI_STYLE_PROP_CLR(map, prop)   ((map)[(prop) >> 5] &= ~(1u << ((prop) & 31)))
// #define IPGUI_STYLE_PROP_GET(map, prop)   (!!((map)[(prop) >> 5] &  (1u << ((prop) & 31))))

// /*============================================================================
//  * 渐变方向
//  *===========================================================================*/
// typedef enum {
//     IPGUI_GRAD_DIR_NONE = 0,
//     IPGUI_GRAD_DIR_HOR,         /* 水平：左→右 */
//     IPGUI_GRAD_DIR_VER,         /* 垂直：上→下 */
//     IPGUI_GRAD_DIR_DIAG,        /* 对角：左上→右下 */
// } ipgui_grad_dir_t;

// /*============================================================================
//  * 样式结构体
//  *
//  * 设计原则：
//  *  - prop_map 标记哪些属性被显式设置，未设置的属性查父样式
//  *  - parent 指针实现任意深度链式继承
//  *  - 所有属性值直接存储，无动态分配
//  *===========================================================================*/
// typedef struct ipgui_style_t {
//     /* 属性标记位图 */
//     unsigned int prop_map[IPGUI_STYLE_PROP_WORDS];

//     /* 父样式指针（继承链，可为NULL） */
//     const struct ipgui_style_t * parent;

//     /* ---- 背景 ---- */
//     color_test_t           bg_color;
//     unsigned char           bg_alpha;
//     color_test_t           bg_grad_color;
//     ipgui_grad_dir_t        bg_grad_dir;
//     const ipgui_image_t *   bg_image;
//     ipgui_image_mode_t      bg_image_mode;
//     unsigned char           bg_image_opacity;
//     ipgui_image_lerp_t      bg_image_lerp;

//     /* ---- 盒子形状 ---- */
//     ipgui_coord_t           radius;
//     ipgui_coord_t           top_padding;
//     ipgui_coord_t           bottom_padding;
//     ipgui_coord_t           left_padding;
//     ipgui_coord_t           right_padding;

//     /* ---- 边框 ---- */
//     ipgui_coord_t           border_width;
//     color_test_t           border_color;
//     unsigned char           border_alpha;

//     /* ---- 外阴影 ---- */
//     color_test_t           shadow_out_color;
//     unsigned char           shadow_out_alpha;
//     ipgui_coord_t           shadow_out_blur;
//     ipgui_coord_t           shadow_out_spread;
//     ipgui_coord_t           shadow_out_offset_x;
//     ipgui_coord_t           shadow_out_offset_y;

//     /* ---- 内阴影 ---- */
//     color_test_t           shadow_in_color;
//     unsigned char           shadow_in_alpha;
//     ipgui_coord_t           shadow_in_blur;
//     ipgui_coord_t           shadow_in_spread;
//     ipgui_coord_t           shadow_in_offset_x;
//     ipgui_coord_t           shadow_in_offset_y;

//     /* ---- 文字 ---- */
//     color_test_t           text_color;
//     unsigned char           text_alpha;
//     const ipgui_font_t *    text_font;
//     ipgui_text_align_t      text_align;
//     ipgui_coord_t           text_line_spacing;

//     /* ---- 布局 ---- */
//     ipgui_coord_t           layout_width;
//     ipgui_coord_t           layout_height;
//     ipgui_coord_t           margin_top;
//     ipgui_coord_t           margin_bottom;
//     ipgui_coord_t           margin_left;
//     ipgui_coord_t           margin_right;
//     ipgui_coord_t           pos_x;
//     ipgui_coord_t           pos_y;

// } ipgui_style_t;

// /*============================================================================
//  * 样式表
//  * 用枚举ID统一管理所有样式，避免到处传指针
//  *===========================================================================*/
// #ifndef IPGUI_STYLESHEET_MAX
// #define IPGUI_STYLESHEET_MAX    64      /* 最多注册64个样式，可在外部重定义 */
// #endif

// typedef struct {
//     ipgui_style_t   styles[IPGUI_STYLESHEET_MAX];
//     unsigned char   used[IPGUI_STYLESHEET_MAX];     /* 该槽位是否已被注册 */
// } ipgui_stylesheet_t;

// /*============================================================================
//  * 控件样式槽
//  * 一个控件可以同时应用多个样式（class叠加），后applyed的优先级更高
//  *===========================================================================*/
// #ifndef IPGUI_WIDGET_STYLE_SLOTS
// #define IPGUI_WIDGET_STYLE_SLOTS    4   /* 每个控件最多叠加几个class，可重定义 */
// #endif

// typedef struct {
//     int     ids[IPGUI_WIDGET_STYLE_SLOTS];  /* 样式ID，-1表示空槽 */
//     int     count;                          /* 当前已叠加的样式数量 */
// } ipgui_widget_style_t;

// /*============================================================================
//  * 全局样式表操作
//  *===========================================================================*/

// /* 初始化全局样式表（系统启动时调用一次） */
// __IPGUI_API__ void ipgui_stylesheet_init(void);

// /* 注册一个样式槽，返回该槽的指针，失败返回NULL */
// __IPGUI_API__ ipgui_style_t * ipgui_stylesheet_register(int id);

// /* 通过ID获取样式指针（未注册返回NULL） */
// __IPGUI_API__ ipgui_style_t * ipgui_stylesheet_get(int id);

// /* 注销一个样式槽 */
// __IPGUI_API__ void ipgui_stylesheet_unregister(int id);

// /*============================================================================
//  * 样式初始化 / 重置
//  *===========================================================================*/

// /* 清空一个样式（所有属性标记清零，断开继承链） */
// __IPGUI_API__ void ipgui_style_init(ipgui_style_t * style);

// /* 拷贝src的所有已设置属性到dst（不覆盖dst中已设置的属性） */
// __IPGUI_API__ void ipgui_style_merge(ipgui_style_t * dst, const ipgui_style_t * src);

// /* 完整拷贝（覆盖dst所有属性） */
// __IPGUI_API__ void ipgui_style_copy(ipgui_style_t * dst, const ipgui_style_t * src);

// /*============================================================================
//  * 继承链操作
//  *===========================================================================*/
// __IPGUI_API__ void ipgui_style_set_parent(ipgui_style_t * style, const ipgui_style_t * parent);
// __IPGUI_API__ const ipgui_style_t * ipgui_style_get_parent(const ipgui_style_t * style);

// /*============================================================================
//  * 控件样式槽操作
//  *===========================================================================*/

// /* 初始化控件样式槽 */
// __IPGUI_API__ void ipgui_widget_style_init(ipgui_widget_style_t * ws);

// /* 向控件叠加一个样式class（越晚加的优先级越高） */
// __IPGUI_API__ int  ipgui_widget_style_apply(ipgui_widget_style_t * ws, int style_id);

// /* 移除控件上的某个样式class */
// __IPGUI_API__ void ipgui_widget_style_remove(ipgui_widget_style_t * ws, int style_id);

// /* 清除控件上所有样式class */
// __IPGUI_API__ void ipgui_widget_style_clear(ipgui_widget_style_t * ws);

// /*============================================================================
//  * 属性查询（自动走继承链 + class叠加优先级）
//  *
//  * 查找顺序：
//  *   控件的class列表（从后往前，高优先级在后）
//  *   → 每个class的继承链（从子到父）
//  *   → 未找到则返回默认值
//  *===========================================================================*/

// /* 内部查找函数：在单个样式及其继承链中查找属性 */
// __IPGUI_STATIC__ int ipgui_style_find_prop(const ipgui_style_t * style, int prop_id,
//                                             ipgui_style_t * out);

// /* 通过控件样式槽查找属性，找到返回1并填充out，否则返回0 */
// __IPGUI_API__ int ipgui_widget_style_find_prop(const ipgui_widget_style_t * ws, int prop_id,
//                                                 ipgui_style_t * out);

// /*============================================================================
//  * 属性 setter（设置某个属性并标记prop_map）
//  *===========================================================================*/

// /* 背景 */
// __IPGUI_API__ void ipgui_style_set_bg_color        (ipgui_style_t * s, color_test_t color);
// __IPGUI_API__ void ipgui_style_set_bg_alpha        (ipgui_style_t * s, unsigned char alpha);
// __IPGUI_API__ void ipgui_style_set_bg_grad         (ipgui_style_t * s, color_test_t end_color, ipgui_grad_dir_t dir);
// __IPGUI_API__ void ipgui_style_set_bg_image        (ipgui_style_t * s, const ipgui_image_t * img, ipgui_image_mode_t mode);
// __IPGUI_API__ void ipgui_style_set_bg_image_opacity(ipgui_style_t * s, unsigned char opacity);
// __IPGUI_API__ void ipgui_style_set_bg_image_lerp   (ipgui_style_t * s, ipgui_image_lerp_t lerp);

// /* 形状 */
// __IPGUI_API__ void ipgui_style_set_radius          (ipgui_style_t * s, ipgui_coord_t radius);
// __IPGUI_API__ void ipgui_style_set_padding         (ipgui_style_t * s, ipgui_coord_t top, ipgui_coord_t bottom,
//                                                      ipgui_coord_t left, ipgui_coord_t right);
// __IPGUI_API__ void ipgui_style_set_padding_all     (ipgui_style_t * s, ipgui_coord_t padding);

// /* 边框 */
// __IPGUI_API__ void ipgui_style_set_border          (ipgui_style_t * s, ipgui_coord_t width,
//                                                      color_test_t color, unsigned char alpha);
// __IPGUI_API__ void ipgui_style_set_border_width    (ipgui_style_t * s, ipgui_coord_t width);
// __IPGUI_API__ void ipgui_style_set_border_color    (ipgui_style_t * s, color_test_t color);
// __IPGUI_API__ void ipgui_style_set_border_alpha    (ipgui_style_t * s, unsigned char alpha);

// /* 外阴影 */
// __IPGUI_API__ void ipgui_style_set_shadow_out      (ipgui_style_t * s, color_test_t color, unsigned char alpha,
//                                                      ipgui_coord_t blur, ipgui_coord_t spread,
//                                                      ipgui_coord_t offset_x, ipgui_coord_t offset_y);

// /* 内阴影 */
// __IPGUI_API__ void ipgui_style_set_shadow_in       (ipgui_style_t * s, color_test_t color, unsigned char alpha,
//                                                      ipgui_coord_t blur, ipgui_coord_t spread,
//                                                      ipgui_coord_t offset_x, ipgui_coord_t offset_y);

// /* 文字 */
// __IPGUI_API__ void ipgui_style_set_text_color      (ipgui_style_t * s, color_test_t color);
// __IPGUI_API__ void ipgui_style_set_text_alpha      (ipgui_style_t * s, unsigned char alpha);
// __IPGUI_API__ void ipgui_style_set_text_font       (ipgui_style_t * s, const ipgui_font_t * font);
// __IPGUI_API__ void ipgui_style_set_text_align      (ipgui_style_t * s, ipgui_text_align_t align);
// __IPGUI_API__ void ipgui_style_set_text_line_spacing(ipgui_style_t * s, ipgui_coord_t spacing);

// /* 布局 */
// __IPGUI_API__ void ipgui_style_set_size            (ipgui_style_t * s, ipgui_coord_t w, ipgui_coord_t h);
// __IPGUI_API__ void ipgui_style_set_pos             (ipgui_style_t * s, ipgui_coord_t x, ipgui_coord_t y);
// __IPGUI_API__ void ipgui_style_set_margin          (ipgui_style_t * s, ipgui_coord_t top, ipgui_coord_t bottom,
//                                                      ipgui_coord_t left, ipgui_coord_t right);
// __IPGUI_API__ void ipgui_style_set_margin_all      (ipgui_style_t * s, ipgui_coord_t margin);

// /*============================================================================
//  * 属性 getter（从控件样式槽查找，走继承链，找不到返回默认值）
//  *===========================================================================*/

// /* 背景 */
// __IPGUI_API__ color_test_t         ipgui_widget_style_get_bg_color        (const ipgui_widget_style_t * ws);
// __IPGUI_API__ unsigned char         ipgui_widget_style_get_bg_alpha        (const ipgui_widget_style_t * ws);
// __IPGUI_API__ color_test_t         ipgui_widget_style_get_bg_grad_color   (const ipgui_widget_style_t * ws);
// __IPGUI_API__ ipgui_grad_dir_t      ipgui_widget_style_get_bg_grad_dir     (const ipgui_widget_style_t * ws);
// __IPGUI_API__ const ipgui_image_t * ipgui_widget_style_get_bg_image        (const ipgui_widget_style_t * ws);
// __IPGUI_API__ ipgui_image_mode_t    ipgui_widget_style_get_bg_image_mode   (const ipgui_widget_style_t * ws);
// __IPGUI_API__ unsigned char         ipgui_widget_style_get_bg_image_opacity(const ipgui_widget_style_t * ws);
// __IPGUI_API__ ipgui_image_lerp_t    ipgui_widget_style_get_bg_image_lerp   (const ipgui_widget_style_t * ws);

// /* 形状 */
// __IPGUI_API__ ipgui_coord_t         ipgui_widget_style_get_radius          (const ipgui_widget_style_t * ws);
// __IPGUI_API__ ipgui_coord_t         ipgui_widget_style_get_padding_top     (const ipgui_widget_style_t * ws);
// __IPGUI_API__ ipgui_coord_t         ipgui_widget_style_get_padding_bottom  (const ipgui_widget_style_t * ws);
// __IPGUI_API__ ipgui_coord_t         ipgui_widget_style_get_padding_left    (const ipgui_widget_style_t * ws);
// __IPGUI_API__ ipgui_coord_t         ipgui_widget_style_get_padding_right   (const ipgui_widget_style_t * ws);

// /* 边框 */
// __IPGUI_API__ ipgui_coord_t         ipgui_widget_style_get_border_width    (const ipgui_widget_style_t * ws);
// __IPGUI_API__ color_test_t         ipgui_widget_style_get_border_color    (const ipgui_widget_style_t * ws);
// __IPGUI_API__ unsigned char         ipgui_widget_style_get_border_alpha    (const ipgui_widget_style_t * ws);

// /* 外阴影 */
// __IPGUI_API__ color_test_t         ipgui_widget_style_get_shadow_out_color  (const ipgui_widget_style_t * ws);
// __IPGUI_API__ unsigned char         ipgui_widget_style_get_shadow_out_alpha  (const ipgui_widget_style_t * ws);
// __IPGUI_API__ ipgui_coord_t         ipgui_widget_style_get_shadow_out_blur   (const ipgui_widget_style_t * ws);
// __IPGUI_API__ ipgui_coord_t         ipgui_widget_style_get_shadow_out_spread (const ipgui_widget_style_t * ws);
// __IPGUI_API__ ipgui_coord_t         ipgui_widget_style_get_shadow_out_offx   (const ipgui_widget_style_t * ws);
// __IPGUI_API__ ipgui_coord_t         ipgui_widget_style_get_shadow_out_offy   (const ipgui_widget_style_t * ws);

// /* 内阴影 */
// __IPGUI_API__ color_test_t         ipgui_widget_style_get_shadow_in_color   (const ipgui_widget_style_t * ws);
// __IPGUI_API__ unsigned char         ipgui_widget_style_get_shadow_in_alpha   (const ipgui_widget_style_t * ws);
// __IPGUI_API__ ipgui_coord_t         ipgui_widget_style_get_shadow_in_blur    (const ipgui_widget_style_t * ws);
// __IPGUI_API__ ipgui_coord_t         ipgui_widget_style_get_shadow_in_spread  (const ipgui_widget_style_t * ws);
// __IPGUI_API__ ipgui_coord_t         ipgui_widget_style_get_shadow_in_offx    (const ipgui_widget_style_t * ws);
// __IPGUI_API__ ipgui_coord_t         ipgui_widget_style_get_shadow_in_offy    (const ipgui_widget_style_t * ws);

// /* 文字 */
// __IPGUI_API__ color_test_t         ipgui_widget_style_get_text_color      (const ipgui_widget_style_t * ws);
// __IPGUI_API__ unsigned char         ipgui_widget_style_get_text_alpha      (const ipgui_widget_style_t * ws);
// __IPGUI_API__ const ipgui_font_t *  ipgui_widget_style_get_text_font       (const ipgui_widget_style_t * ws);
// __IPGUI_API__ ipgui_text_align_t    ipgui_widget_style_get_text_align      (const ipgui_widget_style_t * ws);
// __IPGUI_API__ ipgui_coord_t         ipgui_widget_style_get_text_line_spacing(const ipgui_widget_style_t * ws);

// /* 布局 */
// __IPGUI_API__ ipgui_coord_t         ipgui_widget_style_get_layout_width    (const ipgui_widget_style_t * ws);
// __IPGUI_API__ ipgui_coord_t         ipgui_widget_style_get_layout_height   (const ipgui_widget_style_t * ws);
// __IPGUI_API__ ipgui_coord_t         ipgui_widget_style_get_margin_top      (const ipgui_widget_style_t * ws);
// __IPGUI_API__ ipgui_coord_t         ipgui_widget_style_get_margin_bottom   (const ipgui_widget_style_t * ws);
// __IPGUI_API__ ipgui_coord_t         ipgui_widget_style_get_margin_left     (const ipgui_widget_style_t * ws);
// __IPGUI_API__ ipgui_coord_t         ipgui_widget_style_get_margin_right    (const ipgui_widget_style_t * ws);
// __IPGUI_API__ ipgui_coord_t         ipgui_widget_style_get_pos_x           (const ipgui_widget_style_t * ws);
// __IPGUI_API__ ipgui_coord_t         ipgui_widget_style_get_pos_y           (const ipgui_widget_style_t * ws);

// /*============================================================================
//  * 样式→渲染属性结构体转换（方便直接传给渲染器）
//  *===========================================================================*/
// __IPGUI_API__ void ipgui_widget_style_to_box_shape  (const ipgui_widget_style_t * ws, const ipgui_aabb_t * content,
//                                                       ipgui_box_shape_attr_t * out);
// __IPGUI_API__ void ipgui_widget_style_to_bg_color   (const ipgui_widget_style_t * ws, ipgui_box_bg_color_attr_t * out);
// __IPGUI_API__ void ipgui_widget_style_to_border     (const ipgui_widget_style_t * ws, ipgui_box_border_attr_t * out);
// __IPGUI_API__ void ipgui_widget_style_to_shadow_out (const ipgui_widget_style_t * ws, ipgui_box_outer_shadow_attr_t * out);
// __IPGUI_API__ void ipgui_widget_style_to_shadow_in  (const ipgui_widget_style_t * ws, ipgui_box_inner_shadow_attr_t * out);
// __IPGUI_API__ void ipgui_widget_style_to_font_attr  (const ipgui_widget_style_t * ws, ipgui_font_ras_attr_t * out);

// #ifdef __cplusplus
// }
// #endif

// #endif /* IPGUI_STYLE_H */

typedef enum {
    /* line style */
    ipgui_style_line_width,
    ipgui_style_line_opacity,
    ipgui_style_line_color,
    ipgui_style_line_cap,
    ipgui_style_line_gradient_color,

};