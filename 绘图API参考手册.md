# IPGUI 绘图 API 参考

## 1. 初始化

```c
#include "ipgui_core.h"

int main(void)
{
    ipgui_err_t err = ipgui_init();
    if (err != IPGUI_ERR_OK) {
        // 初始化失败（通常内存不足）
        while (1);
    }
    // ... 绘图 ...
}
```

---

## 2. 数据类型速查

### 2.1 坐标与几何

```c
// 点
ipgui_point_t p;
p.x = 100;
p.y = 50;

// 线段
ipgui_line_t line;
line.start.x = 0;   line.start.y = 0;
line.end.x   = 100; line.end.y   = 100;

// 矩形 / AABB
ipgui_aabb_t box;
box.start.x = 10;  box.start.y = 20;
box.end.x   = 200; box.end.y   = 150;
```

### 2.2 颜色

```c
ipgui_color_t color;

// 方式一：宏设置
IPGUI_COLOR_SET(color, 255, 0xFF0000);  // alpha=255, RGB=0xFF0000（红色）

// 方式二：逐通道设置
color.a = 255;
color.r = 0xFF;
color.g = 0x00;
color.b = 0x00;

// 方式三：RGBA 宏拼接
u32_t packed = IPGUI_COLOR_RGBA(255, 0, 0, 255);  // R,G,B,A
```

### 2.3 Surface（绘图目标）

```c
int w = 200, h = 200;
int stride = w * 4;

// 离屏缓冲（裸机可改为静态数组）
unsigned char * buf = malloc((size_t)stride * h);

ipgui_surf_t surf;
surf.surf.start.x = 0;
surf.surf.start.y = 0;
surf.surf.end.x   = w - 1;
surf.surf.end.y   = h - 1;
surf.color  = buf;                    // 指向像素数据
surf.stride = stride;                 // 每行字节数
surf.pix_fmt = PIX_FMT_RGBA8888;      // 像素格式
surf.pix_size = 4;                    // 每像素字节数
```

### 2.4 Paint（填充源）

#### 2.4.1 纯色填充

```c
ipgui_paint_t paint;
paint.type = IPGUI_PAINT_COLOR;
IPGUI_COLOR_SET(paint.src.color, 255, 0xFF0000);
```

#### 2.4.2 图像填充

```c
paint.type = IPGUI_PAINT_IMAGE;
paint.src.image_src = ...;  // 图像数据
```

#### 2.4.3 渐变填充

三种渐变类型：线性、径向、锥形。

```c
// 线性渐变
ipgui_liner_gradient_color_t liner;
ipgui_liner_gradient_init_direct(&liner, 0, 0, 100, 100);  // 从(0,0)到(100,100)

ipgui_gradient_color_stop_t stop0;
stop0.pos = 0;
IPGUI_COLOR_SET(stop0.color, 255, 0xFF0000);  // 起点：红色
ipgui_liner_gradient_add_stop(&liner, &stop0);

ipgui_gradient_color_stop_t stop1;
stop1.pos = 255;
IPGUI_COLOR_SET(stop1.color, 255, 0x0000FF);  // 终点：蓝色
ipgui_liner_gradient_add_stop(&liner, &stop1);

ipgui_paint_t grad_paint;
grad_paint.type = IPGUI_PAINT_GRADIENT;
grad_paint.src.grad_src.grad_type = IPGUI_GRADIENT_TYPE_LINEAR;
grad_paint.src.grad_src.grad.liner_grad = liner;
```

```c
// 径向渐变
ipgui_radial_gradient_color_t radial;
ipgui_radial_gradient_init(&radial, 100, 100, 80);  // 圆心(100,100), 半径80

ipgui_gradient_color_stop_t stop0;
stop0.pos = 0;
IPGUI_COLOR_SET(stop0.color, 255, 0xFF0000);  // 起点：红色
ipgui_radial_gradient_add_stop(&radial, &stop0);

ipgui_gradient_color_stop_t stop1;
stop1.pos = 255;
IPGUI_COLOR_SET(stop1.color, 255, 0x0000FF);  // 终点：蓝色
ipgui_radial_gradient_add_stop(&radial, &stop1);

grad_paint.type = IPGUI_PAINT_GRADIENT;
grad_paint.src.grad_src.grad_type = IPGUI_GRADIENT_TYPE_RADIAL;
grad_paint.src.grad_src.grad.radial_grad = radial;
```

```c
// 锥形（角度）渐变
ipgui_conic_gradient_color_t conic;
ipgui_conic_gradient_init(&conic, 100, 100, 0);  // 圆心(100,100), 起始角度0°

ipgui_gradient_color_stop_t stop0;
stop0.pos = 0;
IPGUI_COLOR_SET(stop0.color, 255, 0xFF0000);  // 起点：红色
ipgui_conic_gradient_add_stop(&conic, &stop0);

ipgui_gradient_color_stop_t stop1;
stop1.pos = 255;
IPGUI_COLOR_SET(stop1.color, 255, 0x0000FF);  // 终点：蓝色
ipgui_conic_gradient_add_stop(&conic, &stop1);

grad_paint.type = IPGUI_PAINT_GRADIENT;
grad_paint.src.grad_src.grad_type = IPGUI_GRADIENT_TYPE_CONIC;
grad_paint.src.grad_src.grad.conic_grad = conic;
```

> 每个渐变最多支持 `IPGUI_GRADIENT_STOP_MAX` 个停止点。

---

## 3. 绘图 API

### 3.1 三角形 `ipgui_draw_triangle`

含 halfplane mask 抗锯齿。

```c
#include "ipgui_draw_triangle.h"

ipgui_point_t p1 = {100, 20};
ipgui_point_t p2 = {20,  170};
ipgui_point_t p3 = {180, 170};

ipgui_triangle_style_t style;
style.blend_mode = IPGUI_BLEND_NORMAL;
style.opacity    = 255;
style.paint.type = IPGUI_PAINT_COLOR;
IPGUI_COLOR_SET(style.paint.src.color, 255, 0xFF0000);

ipgui_draw_triangle(&surf, NULL, &p1, &p2, &p3, &style);
```

**结构体：**

| 字段 | 类型 | 说明 |
|------|------|------|
| `paint` | `ipgui_paint_t` | 填充源 |
| `opacity` | `u8_t` | 不透明度 0~255 |
| `blend_mode` | `ipgui_blend_mode_t` | 混合模式 |

---

### 3.2 直线 `ipgui_draw_line_generic`

含 WDF mask 抗锯齿（斜线）或直接混合（水平/垂直线）。

```c
#include "ipgui_draw_line.h"

ipgui_line_t line;
line.start.x = 10;   line.start.y = 10;
line.end.x   = 190;  line.end.y   = 100;

ipgui_line_style_t style;
style.width      = 6;                       // 线宽（像素）
style.cap        = IPGUI_LINE_CAP_BUTT;     // 线帽：BUTT=平头, ROUND=圆头
style.blend_mode = IPGUI_BLEND_NORMAL;
style.opacity    = 255;
style.paint.type = IPGUI_PAINT_COLOR;
IPGUI_COLOR_SET(style.paint.src.color, 255, 0x0000FF);

ipgui_draw_line_generic(&surf, NULL, &line, &style);
```

**结构体：**

| 字段 | 类型 | 说明 |
|------|------|------|
| `width` | `ipgui_coord_t` | 线宽（像素） |
| `cap` | `ipgui_line_cap_t` | 线帽样式 |
| `paint` | `ipgui_paint_t` | 填充源 |
| `opacity` | `u8_t` | 不透明度 0~255 |
| `blend_mode` | `ipgui_blend_mode_t` | 混合模式 |

---

### 3.3 像素 `ipgui_draw_pixel`

```c
#include "ipgui_draw_pixel.h"

ipgui_color_t color;
IPGUI_COLOR_SET(color, 255, 0xFFFF00);

ipgui_draw_pixel(&surf, NULL, 50, 50, color,
                 255,         // mask (0~255, 255=完全不透明)
                 255,         // opacity
                 IPGUI_BLEND_NORMAL);
```

**参数：**

| 参数 | 类型 | 说明 |
|------|------|------|
| `surf` | `ipgui_surf_t *` | 目标 surface |
| `clip` | `ipgui_aabb_t *` | 裁剪区域，NULL=不裁剪 |
| `x`, `y` | `ipgui_coord_t` | 像素坐标 |
| `color` | `ipgui_color_t` | 源颜色 |
| `mask` | `u8_t` | 像素 mask 0~255 |
| `opacity` | `u8_t` | 不透明度 0~255 |
| `blend_mode` | `ipgui_blend_mode_t` | 混合模式 |

---

### 3.4 圆弧 / 圆环 / 圆 / 扇形 `ipgui_draw_arc`

通过 `er`（外圆半径）和 `ir`（内圆半径）组合可实现多种几何（约束：`er > ir ≥ 0`）：

| 配置 | 效果 |
|------|------|
| `er > ir > 0`, `angle < 360` | 圆弧 |
| `er > ir > 0`, `angle >= 360` | 圆环 |
| `er > 0, ir == 0`, `angle < 360` | 扇形 |
| `er > 0, ir == 0`, `angle >= 360` | 实心圆 |

```c
#include "ipgui_draw_arc.h"

ipgui_arc_t arc;
arc.cx    = 100;                       // 圆心 x
arc.cy    = 50;                        // 圆心 y
arc.er    = 25;                        // 外圆半径
arc.ir    = 10;                        // 内圆半径（0 = 实心扇区）
arc.start = 0;                         // 起始角度（度）
arc.dir   = IPGUI_ARC_DRAW_DIR_CW;     // 方向：CW=顺时针, CCW=逆时针
arc.angle = 270;                       // 绘制角度（度）

ipgui_arc_style_t style;
style.blend_mode = IPGUI_BLEND_NORMAL;
style.opacity    = 255;
style.sep_type   = IPGUI_ARC_ENDPOINT_TYPE_ROUND;   // 起始端点
style.eep_type   = IPGUI_ARC_ENDPOINT_TYPE_ROUND;   // 结束端点
style.paint.type = IPGUI_PAINT_COLOR;
IPGUI_COLOR_SET(style.paint.src.color, 255, 0x00FF00);

ipgui_draw_arc(&surf, NULL, &arc, &style);
```

**`ipgui_arc_t` 结构体：**

| 字段 | 类型 | 说明 |
|------|------|------|
| `cx`, `cy` | `ipgui_coord_t` | 圆心坐标 |
| `er` | `ipgui_coord_t` | 外圆半径 |
| `ir` | `ipgui_coord_t` | 内圆半径（0=实心） |
| `start` | `ipgui_arc_angle_t` | 起始角度（度） |
| `dir` | `ipgui_arc_draw_dir_t` | 绘制方向 |
| `angle` | `u16_t` | 绘制角度（度） |

**`ipgui_arc_style_t` 结构体：**

| 字段 | 类型 | 说明 |
|------|------|------|
| `paint` | `ipgui_paint_t` | 填充源 |
| `opacity` | `u8_t` | 不透明度 0~255 |
| `sep_type` | `ipgui_arc_endpoint_type_t` | 起始端点样式 |
| `eep_type` | `ipgui_arc_endpoint_type_t` | 结束端点样式 |
| `blend_mode` | `ipgui_blend_mode_t` | 混合模式 |

---

### 3.5 实心圆 `ipgui_draw_filled_circle`

```c
#include "ipgui_draw_filled_circle.h"

ipgui_filled_circle_style_t style;
style.blend_mode = IPGUI_BLEND_NORMAL;
style.opacity    = 255;
style.paint.type = IPGUI_PAINT_COLOR;
IPGUI_COLOR_SET(style.paint.src.color, 255, 0xFFA500);

ipgui_draw_filled_circle(&surf, NULL,
                         100,   // 圆心 x
                         100,   // 圆心 y
                         50,    // 半径
                         &style);
```

---

### 3.6 圆角矩形 `ipgui_draw_box_background` + `ipgui_draw_box_border`

```c
#include "ipgui_box_style.h"
#include "ipgui_draw_box_background.h"
#include "ipgui_draw_box_border.h"

// 矩形区域
ipgui_aabb_t box;
box.start.x = 10;   box.start.y = 80;
box.end.x   = 190;  box.end.y   = 180;

// 圆角配置
ipgui_box_style_t box_style;
box_style.left_padding   = 0;
box_style.right_padding  = 0;
box_style.top_padding    = 0;
box_style.bottom_padding = 0;
box_style.left_top_radius     = 20;
box_style.right_top_radius    = 20;
box_style.left_bottom_radius  = 20;
box_style.right_bottom_radius = 20;

// —— 背景 ——
ipgui_box_bg_style_t bg;
bg.blend_mode = IPGUI_BLEND_NORMAL;
bg.opacity    = 200;                  // 半透明
bg.paint.type = IPGUI_PAINT_COLOR;
IPGUI_COLOR_SET(bg.paint.src.color, 255, 0xFFA500);

ipgui_draw_box_background(&surf, NULL, &box, &box_style, &bg);

// —— 边框 ——
ipgui_box_border_style_t border;
border.blend_mode = IPGUI_BLEND_NORMAL;
border.width      = 3;                // 边框宽度
border.opacity    = 255;
border.paint.type = IPGUI_PAINT_COLOR;
IPGUI_COLOR_SET(border.paint.src.color, 255, 0xFF0000);

ipgui_draw_box_border(&surf, NULL, &box, &box_style, &border);
```

**`ipgui_box_style_t` 结构体：**

| 字段 | 类型 | 说明 |
|------|------|------|
| `left_padding` ~ `bottom_padding` | `ipgui_coord_t` | 内边距 |
| `left_top_radius` | `ipgui_coord_t` | 左上角圆角半径 |
| `right_top_radius` | `ipgui_coord_t` | 右上角圆角半径 |
| `left_bottom_radius` | `ipgui_coord_t` | 左下角圆角半径 |
| `right_bottom_radius` | `ipgui_coord_t` | 右下角圆角半径 |

**`ipgui_box_bg_style_t` 结构体：**

| 字段 | 类型 | 说明 |
|------|------|------|
| `paint` | `ipgui_paint_t` | 填充源 |
| `opacity` | `u8_t` | 不透明度 0~255 |
| `blend_mode` | `ipgui_blend_mode_t` | 混合模式 |

**`ipgui_box_border_style_t` 结构体：**

| 字段 | 类型 | 说明 |
|------|------|------|
| `paint` | `ipgui_paint_t` | 填充源 |
| `opacity` | `u8_t` | 不透明度 0~255 |
| `width` | `ipgui_coord_t` | 边框宽度 |
| `blend_mode` | `ipgui_blend_mode_t` | 混合模式 |

---

### 3.7 多边形 `ipgui_draw_polygon`

```c
#include "ipgui_draw_polygon.h"

ipgui_polygon_ras_t ras;
ipgui_polygon_ras_init(&ras);

ipgui_point_t pts[] = {
    {100, 10},
    {150, 60},
    {130, 110},
    {70,  110},
    {50,  60}
};

ipgui_polygon_style_t attr;
attr.blend_mode = IPGUI_BLEND_NORMAL;
attr.alpha      = 255;
IPGUI_COLOR_SET(attr.color, 255, 0x800080);

ipgui_draw_polygon(&surf, NULL, pts, 5, &ras, &attr);
```

> 顶点必须按多边形轮廓顺序排列，否则光栅化结果虽不出错但是与预期不符。

---

### 3.8 图像 `ipgui_draw_image` 系列

图像绘制分为底层 API 和高级 API。高级 API 是对底层 API 的封装，使用更方便。

#### 3.8.1 底层 API — `ipgui_draw_image`

```c
#include "ipgui_draw_image.h"

ipgui_image_data_t * img = ...;      // 图片数据（需预先加载）

ipgui_point_t pivot  = {0, 0};       // 变换参考点（相对于图片）
ipgui_point_t anchor = {50, 50};     // 目标锚点（在 surface 上的位置）

ipgui_trans_mat_t trans = {          // 变换矩阵
    64, 0,                           // a, b  — 缩放+旋转
    0,  64                           // c, d  — 64=1.0（26.6 定点数）
};

ipgui_image_draw_style_t style;
style.blend_mode = IPGUI_BLEND_NORMAL;
style.opacity    = 255;

ipgui_draw_image(&surf, NULL, img, &pivot, &anchor, &trans, &style);
```

**`ipgui_trans_mat_t` 结构体：**

| 字段 | 类型 | 说明 |
|------|------|------|
| `a`, `b` | `ipgui_scoord_t` | 变换矩阵第一行（26.6 定点数，64=1.0） |
| `c`, `d` | `ipgui_scoord_t` | 变换矩阵第二行 |

**`ipgui_image_draw_style_t` 结构体：**

| 字段 | 类型 | 说明 |
|------|------|------|
| `opacity` | `u8_t` | 不透明度 0~255 |
| `blend_mode` | `ipgui_blend_mode_t` | 混合模式 |

#### 3.8.2 高级 API — `ipgui_draw_image_api.h`

```c
#include "ipgui_draw_image_api.h"

ipgui_image_data_t * img = ...;
ipgui_image_draw_style_t style;
style.blend_mode = IPGUI_BLEND_NORMAL;
style.opacity    = 255;

// — 在指定坐标绘制 —
ipgui_draw_image_at(&surf, img, 10, 20, &style);

// — 以某点为中心绘制 —
ipgui_draw_image_centered(&surf, img, 100, 100, &style);

// — 在目标矩形内按对齐+缩放模式绘制 —
ipgui_aabb_t target;
target.start.x = 10;  target.start.y = 10;
target.end.x   = 190; target.end.y   = 190;

ipgui_draw_image_in_rect(
    &surf, img, &target,
    IPGUI_IMG_ALIGN_CENTER,      // 对齐方式（九宫格定位）
    IPGUI_IMG_FIT_FIT,           // 缩放模式
    &style
);
```

**`ipgui_image_align_t` 枚举：**

| 值 | 说明 |
|----|------|
| `IPGUI_IMG_ALIGN_TOP_LEFT` | 左上角 |
| `IPGUI_IMG_ALIGN_TOP` | 顶部居中 |
| `IPGUI_IMG_ALIGN_TOP_RIGHT` | 右上角 |
| `IPGUI_IMG_ALIGN_LEFT` | 左侧居中 |
| `IPGUI_IMG_ALIGN_CENTER` | 居中 |
| `IPGUI_IMG_ALIGN_RIGHT` | 右侧居中 |
| `IPGUI_IMG_ALIGN_BOTTOM_LEFT` | 左下角 |
| `IPGUI_IMG_ALIGN_BOTTOM` | 底部居中 |
| `IPGUI_IMG_ALIGN_BOTTOM_RIGHT` | 右下角 |

**`ipgui_image_fit_t` 枚举：**

| 值 | 说明 |
|----|------|
| `IPGUI_IMG_FIT_NONE` | 原图大小，不缩放 |
| `IPGUI_IMG_FIT_FIT` | 等比缩放至完全容纳（可能留空） |
| `IPGUI_IMG_FIT_FILL` | 等比缩放至完全覆盖（超出裁剪） |
| `IPGUI_IMG_FIT_STRETCH` | 拉伸填满，不保持宽高比 |

#### 3.8.3 带裁剪的绘制 `ipgui_draw_image_ex`（暂时不支持）

```c
#include "ipgui_draw_image_ex.h"

// 支持蒙版裁剪和完整的变换参数
ipgui_draw_image_ex(&surf, NULL, img_mask, img_data,
                    &pivot, &anchor, &trans, &style);
```

---

## 4. 像素格式

`ipgui_surf_t.pix_fmt` 支持的格式：

| 枚举 | 说明 | pix_size |
|------|------|----------|
| `PIX_FMT_RGBA8888` | 32位 RGBA | 4 |
| `PIX_FMT_BGRA8888` | 32位 BGRA | 4 |
| `PIX_FMT_ARGB8888` | 32位 ARGB | 4 |
| `PIX_FMT_ABGR8888` | 32位 ABGR | 4 |
| `PIX_FMT_RGB888` | 24位 RGB | 3 |
| `PIX_FMT_BGR888` | 24位 BGR | 3 |
| `PIX_FMT_RGB565` | 16位 RGB565 | 2 |
| `PIX_FMT_BGR565` | 16位 BGR565 | 2 |

---

## 5. Blend 模式

当前仅支持 `IPGUI_BLEND_NORMAL`（Porter-Duff OVER，预乘 alpha）。

```c
IPGUI_BLEND_NORMAL
```

---

## 6. 附录：完整示例（PC机版本）

以下示例在 200x200 白色背景上绘制红色三角形、蓝色直线、绿色圆弧、橙色圆角矩形（含红色边框），并导出为 BMP。

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ipgui_core.h"
#include "ipgui_color.h"
#include "ipgui_draw_triangle.h"
#include "ipgui_draw_line.h"
#include "ipgui_draw_arc.h"
#include "ipgui_box_style.h"
#include "ipgui_draw_box_background.h"
#include "ipgui_draw_box_border.h"

int main(void)
{
    ipgui_init();

    int w = 200, h = 200;
    int stride = w * 4;

    unsigned char * buf = (unsigned char *)malloc((size_t)stride * h);
    if (!buf) { printf("malloc fail\n"); return 1; }

    ipgui_color_t white;
    IPGUI_COLOR_SET(white, 255, 0xFFFFFF);
    for (int i = 0; i < w * h; i++)
        ((u32_t *)buf)[i] = white.v;

    ipgui_surf_t surf;
    surf.surf.start.x = 0;
    surf.surf.start.y = 0;
    surf.surf.end.x   = w - 1;
    surf.surf.end.y   = h - 1;
    surf.color  = buf;
    surf.stride = stride;
    surf.pix_fmt = PIX_FMT_RGBA8888;
    surf.pix_size = 4;

    /* ---- 红色三角形 ---- */
    ipgui_point_t p1, p2, p3;
    p1.x = 100; p1.y = 20;
    p2.x = 20;  p2.y = 170;
    p3.x = 180; p3.y = 170;

    ipgui_triangle_style_t tri_style;
    tri_style.blend_mode = IPGUI_BLEND_NORMAL;
    tri_style.opacity = 255;
    tri_style.paint.type = IPGUI_PAINT_COLOR;
    IPGUI_COLOR_SET(tri_style.paint.src.color, 255, 0xFF0000);

    ipgui_draw_triangle(&surf, NULL, &p1, &p2, &p3, &tri_style);

    /* ---- 蓝色直线 ---- */
    ipgui_line_t line;
    line.start.x = 10;
    line.start.y = 10;
    line.end.x   = 190;
    line.end.y   = 190;

    ipgui_line_style_t line_style;
    line_style.width      = 3;
    line_style.cap        = IPGUI_LINE_CAP_BUTT;
    line_style.blend_mode = IPGUI_BLEND_NORMAL;
    line_style.opacity    = 255;
    line_style.paint.type = IPGUI_PAINT_COLOR;
    IPGUI_COLOR_SET(line_style.paint.src.color, 255, 0x0000FF);

    ipgui_draw_line_generic(&surf, NULL, &line, &line_style);

    /* ---- 绿色圆弧 ---- */
    ipgui_arc_t arc;
    arc.cx    = 160;
    arc.cy    = 40;
    arc.er    = 25;
    arc.ir    = 10;
    arc.start = 0;
    arc.dir   = IPGUI_ARC_DRAW_DIR_CW;
    arc.angle = 270;

    ipgui_arc_style_t arc_style;
    arc_style.blend_mode = IPGUI_BLEND_NORMAL;
    arc_style.opacity    = 255;
    arc_style.sep_type   = IPGUI_ARC_ENDPOINT_TYPE_ROUND;
    arc_style.eep_type   = IPGUI_ARC_ENDPOINT_TYPE_ROUND;
    arc_style.paint.type = IPGUI_PAINT_COLOR;
    IPGUI_COLOR_SET(arc_style.paint.src.color, 255, 0x00FF00);

    ipgui_draw_arc(&surf, NULL, &arc, &arc_style);

    /* ---- 橙色圆角矩形 + 红色边框 ---- */
    ipgui_aabb_t box;
    box.start.x = 10;
    box.start.y = 80;
    box.end.x   = 190;
    box.end.y   = 180;

    ipgui_box_style_t box_style;
    box_style.left_padding   = 0;
    box_style.right_padding  = 0;
    box_style.top_padding    = 0;
    box_style.bottom_padding = 0;
    box_style.left_top_radius     = 20;
    box_style.right_top_radius    = 20;
    box_style.left_bottom_radius  = 20;
    box_style.right_bottom_radius = 20;

    ipgui_box_bg_style_t box_bg_style;
    box_bg_style.blend_mode = IPGUI_BLEND_NORMAL;
    box_bg_style.opacity    = 200;
    box_bg_style.paint.type = IPGUI_PAINT_COLOR;
    IPGUI_COLOR_SET(box_bg_style.paint.src.color, 255, 0xFFA500);

    ipgui_draw_box_background(&surf, NULL, &box, &box_style, &box_bg_style);

    ipgui_box_border_style_t box_border_style;
    box_border_style.blend_mode = IPGUI_BLEND_NORMAL;
    box_border_style.width      = 3;
    box_border_style.opacity    = 255;
    box_border_style.paint.type = IPGUI_PAINT_COLOR;
    IPGUI_COLOR_SET(box_border_style.paint.src.color, 255, 0xFF0000);

    ipgui_draw_box_border(&surf, NULL, &box, &box_style, &box_border_style);

    /* ---- 写 BMP ---- */
    int row_size = ((w * 3 + 3) / 4) * 4;
    int data_size = row_size * h;

    unsigned char bmp_header[14] = {
        'B','M',
        (14 + 40 + data_size) & 0xFF,
        ((14 + 40 + data_size) >> 8) & 0xFF,
        ((14 + 40 + data_size) >> 16) & 0xFF,
        ((14 + 40 + data_size) >> 24) & 0xFF,
        0,0, 0,0,
        54,0,0,0
    };

    unsigned char dib_header[40] = {0};
    dib_header[0]  = 40;
    dib_header[4]  = (w) & 0xFF;       dib_header[5]  = (w >> 8) & 0xFF;
    dib_header[8]  = (h) & 0xFF;       dib_header[9]  = (h >> 8) & 0xFF;
    dib_header[12] = 1;
    dib_header[14] = 24;
    dib_header[20] = (data_size) & 0xFF;
    dib_header[21] = (data_size >> 8) & 0xFF;
    dib_header[22] = (data_size >> 16) & 0xFF;
    dib_header[23] = (data_size >> 24) & 0xFF;

    FILE * f = fopen("gfx_test.bmp", "wb");
    if (!f) { printf("fopen fail\n"); free(buf); return 1; }

    fwrite(bmp_header, 1, 14, f);
    fwrite(dib_header, 1, 40, f);

    for (int y = h - 1; y >= 0; y--) {
        unsigned char * row = buf + (size_t)y * stride;
        int pad = 0;
        for (int x = 0; x < w; x++) {
            unsigned char * p = row + x * 4;
            fputc(p[2], f);  /* B */
            fputc(p[1], f);  /* G */
            fputc(p[0], f);  /* R */
            pad += 3;
        }
        while (pad < row_size) { fputc(0, f); pad++; }
    }

    fclose(f);
    free(buf);
    printf("BMP written to gfx_test.bmp\n");
    return 0;
}
```

> BMP 行序从底部到顶部；RGBA8888 → BMP 时 R 和 B 字节交换。
