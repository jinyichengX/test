# ESDBox_IPGUI 图形引擎技术文档

> **模块聚焦**: core/gfx（图形渲染）与 core/composite（颜色合成）  
> **文档版本**: v1.0  
> **适用范围**: 嵌入式 GUI 渲染管线核心层  

---

## 目录

1. [工程架构概述](#1-工程架构概述)
2. [core/gfx 图形绘制模块](#2-coregfx-图形绘制模块)
   - [2.1 模块定位与职责](#21-模块定位与职责)
   - [2.2 核心数据结构](#22-核心数据结构)
   - [2.3 遮罩（Mask）子系统](#23-遮罩mask子系统)
   - [2.4 渐变色彩子系统](#24-渐变色彩子系统)
   - [2.5 图元绘制子系统](#25-图元绘制子系统)
   - [2.6 Box 绘制子系统](#26-box-绘制子系统)
   - [2.7 缓冲区管理](#27-缓冲区管理)
3. [core/composite 颜色合成模块](#3-corecomposite-颜色合成模块)
   - [3.1 模块定位与职责](#31-模块定位与职责)
   - [3.2 Paint 绘制系统](#32-paint-绘制系统)
   - [3.3 颜色混合引擎](#33-颜色混合引擎)
   - [3.4 渐变填充引擎](#34-渐变填充引擎)
   - [3.5 图像合成引擎](#35-图像合成引擎)
4. [模块间交互逻辑与数据流](#4-模块间交互逻辑与数据流)
5. [关键算法实现细节](#5-关键算法实现细节)
6. [设计模式与最佳实践](#6-设计模式与最佳实践)
7. [使用示例与 API 快速参考](#7-使用示例与-api-快速参考)

---

## 1. 工程架构概述

ESDBox_IPGUI 是一个纯 C 语言实现的轻量级嵌入式 GUI 渲染引擎，采用分层架构设计，从底层基础库到上层控件系统逐级构建：

```
┌─────────────────────────────────────────────────┐
│  core/ui/widget_manager/   控件树 · 脏矩形 · 布局│  ← UI 层
├─────────────────────────────────────────────────┤
│  core/gfx  ·  core/composite  ·  core/image     │  ← 渲染核心层
│  core/vector_render  ·  core/misc               │
├─────────────────────────────────────────────────┤
│  al/hal/ (屏幕 · VFS · 输入)  al/vfs/           │  ← 抽象层
├─────────────────────────────────────────────────┤
│  base/ (AVL · 内存池 · 队列 · 定时器 · 向量)     │  ← 基础库
├─────────────────────────────────────────────────┤
│  port/sdl/ (SDL2) · font/ (字体) · charset/     │  ← 移植/资源
└─────────────────────────────────────────────────┘
```

**核心渲染数据流**:

```
ipgui_draw_*()
    ↓ 生成 mask（遮罩）+ paint（颜色源）
    ↓ 裁剪到 clip（裁剪区）
    ↓ 逐像素调用 ipgui_blend()
        ↓ blend 分发器: 根据 paint.type 派发
        ↓ blend_color / blend_gradient / blend_image
        ↓ 写入 ipgui_surf_t（目标表面）
    ↑
ipgui_screen_render() — 驱动脏矩形渲染循环
```

---

## 2. core/gfx 图形绘制模块

### 2.1 模块定位与职责

`core/gfx` 负责一切"形状绘制"：从最基本的像素打点到复杂的圆角矩形、多边形、圆弧和线段。它不直接操作像素颜色——颜色的合成工作委托给 `core/composite` 模块。gfx 的职责是：

- 生成**几何遮罩**（mask），描述"哪些像素需要被绘制"
- 生成**渐变遮罩**（gradient mask），描述"绘制透明度如何变化"
- 管理绘制缓冲区（image buffer / mask buffer）
- 组合 mask 与 paint，调用 blend 完成像素写入

**目录结构**:

| 文件 | 职责 |
|------|------|
| `ipgui_draw_pixel.c/h` | 单像素绘制 |
| `ipgui_draw_line.c/h` | 线段绘制 |
| `ipgui_draw_polygon.c/h` | 多边形光栅化 |
| `ipgui_draw_arc.c/h` | 弧/圆/扇形/圆环绘制 |
| `ipgui_draw_triangle.c/h` | 三角形绘制 |
| `ipgui_draw_box_background.c/h` | 圆角矩形背景 |
| `ipgui_draw_box_border.c/h` | 圆角矩形边框 |
| `ipgui_draw_box_shadow.c/h` | Box 阴影 |
| `ipgui_draw_icon.c/h` | 图标绘制 |
| `ipgui_draw_image.c/h` | 图像绘制 |
| `ipgui_draw_builtin_font.c/h` | 内建字体绘制 |
| `ipgui_edge_halfplane_mask.c/h` | 半平面边沿遮罩 |
| `ipgui_edge_wdf_mask.c/h` | WDF 宽边沿遮罩 |
| `ipgui_ring_mask.c/h` | 环形圆角遮罩(带 LRU 缓存) |
| `ipgui_gradient_color.c/h` | 线性/径向/锥形渐变色彩计算 |
| `ipgui_mask_gradient.c/h` | 遮罩渐变（线性/径向透明度渐变） |
| `ipgui_image_buf.c/h` | 图像缓冲区分配 |
| `ipgui_mask_buf.c/h` | 遮罩缓冲区分配 |
| `ipgui_image_mask.c/h` | 图像遮罩处理 |
| `ipgui_box_style.h` | Box 样式结构体定义 |

---

### 2.2 核心数据结构

#### 绘图面 `ipgui_surf_t`

```c
typedef struct {
    ipgui_aabb_t    surf;      // 表面在屏幕上的区域
    u8_t          * color;     // 指向 surf 区域第一个像素
    u32_t           stride;    // 每行字节跨度
    ipgui_pix_fmt_t pix_fmt;   // 像素格式
    u8_t            pix_size;  // 每像素字节数（可能含对齐填充）
} ipgui_surf_t;
```

坐标使用相对坐标：`ipgui_surf_color_get(surf, x, y)` 返回 `color + y*stride + x*pix_size`。

#### 像素颜色 `ipgui_color_t`

```c
typedef union {
    struct { u8_t b, g, r, a; };  // 小端: BGRA 内存布局
    u32_t v;                       // 整体 32 位值
} ipgui_color_t;
```

**关键设计**: 使用 union 实现结构化通道访问与批量 32 位操作的无缝切换。

---

### 2.3 遮罩（Mask）子系统

遮罩系统是 gfx 模块的核心基础——几乎所有图元绘制都依赖遮罩来定义"哪些像素需要上色"以及"透明度权重"。

#### 2.3.1 半平面边沿遮罩 (`ipgui_edge_halfplane_mask`)

**用途**: 生成一条直线把平面分为左右两个半平面的遮罩值，每个像素到直线的距离映射为 0-255 的透明度。

**核心数据结构**:
```c
typedef s32_t ipgui_edge_coord_t;  // 26.6 定点数（64 子像素精度）

typedef struct {
    ipgui_edge_coord_t x1, y1;      // 线段端点（y 较小的那个）
    ipgui_edge_coord_t dx, dy;      // Δx, Δy
    u8_t correction_frac_index : 7; // 修正因子索引（轴向→垂直距离修正）
    u8_t flatten : 1;               // 1=平边(steep=0)，0=斜边
} ipgui_edge_param_t;
```

**关键算法** — 距离到透明度映射:

```
cover = 64 - d0        （d0 为像素覆盖值，在[0,63]范围内）
mask  = cover * 255 / 64
```

- 使用 26.6 定点数（64 子像素精度）保证亚像素精度
- `correction_frac_index` 修正因子将轴向距离转换为垂直距离（分母 256）
- 区分平边（|dx| >= dy）和斜边（|dx| < dy）两种模式，优化扫描效率

#### 2.3.2 WDF 宽边沿遮罩 (`ipgui_edge_wdf_mask`)

**用途**: 计算线段两侧各 `half_width` 像素范围的渐变遮罩。用于绘制反锯齿线段时定义线宽方向的透明度过渡。

```c
typedef struct {
    ipgui_coord_t a, b, c;      // 直线方程参数 ax + by + c = 0
    ipgui_coord_t x1, y1;       // 线段起点（y 较小的端点）
    ipgui_coord_t dx, dy;       // 增量
    u8_t correction_frac_index;  // 修正因子
    u8_t flatten;               // 平边标志
} ipgui_edge_wdf_param_t;
```

**与 halfplane 遮罩的关系**:
- halfplane 遮罩用于消除边的一侧（用于多边形/边框裁剪）
- WDF 遮罩用于定义边两侧的渐变带（用于线段抗锯齿）

#### 2.3.3 环形圆角遮罩 (`ipgui_ring_mask`)

**用途**: 为 Box 的圆角生成圆环遮罩。用一个 1/4 圆环的 mask 数据，通过**水平翻转和垂直翻转**覆盖四个角的渲染需求。

```c
typedef struct {
    struct list_head   node;           // LRU 链表节点
    ipgui_coord_t      r;             // 圆角半径
    u16_t            * mask_index_at_y; // 每行的 mask 起始索引
    ipgui_coord_t    * mask_start_x_at_y; // 每行的起始 x 偏移
    u16_t              dig_mask_index;
    ipgui_coord_t      dig_mask_start_xy;
    u8_t             * mask;          // 预计算的 mask 数据
    u32_t              last_used_tick;
    u16_t              refcnt;        // 引用计数（0 时可淘汰）
} corner_mask_cache_item_t;
```

**设计亮点** — LRU 缓存:
- 圆角 mask 计算开销大，因此缓存已计算的半径
- 使用引用计数 + LRU 时间戳管理缓存生命周期
- 同一半径的多次绘制直接复用缓存数据

#### 2.3.4 渐变遮罩 (`ipgui_mask_gradient`)

**用途**: 生成线性或径向的透明度渐变遮罩，每个像素处的透明度由 stop 点插值决定。

```c
typedef struct {
    unsigned char pos;      // 停止点位置 (0-255)
    unsigned char value;    // 遮罩值 (0=全透明, 255=完全不透明)
} ipgui_mask_gradient_stop_t;
```

提供两种渐变模式:
- **线性遮罩渐变**: 从起点到终点，每个像素投影到渐变向量上获取位置
- **径向遮罩渐变**: 以圆心向外辐射，距离映射为位置

**API 模式**: 支持比例模式（0.0-1.0 归一化 + apply_to_aabb）和直接坐标模式。

---

### 2.4 渐变色彩子系统

`ipgui_gradient_color` 是图形引擎中功能最完整的子系统之一。它实现了三种渐变类型，每种都支持 stop 点颜色插值和逐像素位置查询。

#### 2.4.1 线性渐变

```c
typedef struct {
    ipgui_coord_t x_start_abs, y_start_abs;  // 渐变起点（绝对坐标）
    ipgui_coord_t x_end_abs, y_end_abs;      // 渐变终点（绝对坐标）
    ipgui_vector_t gradient_vector;          // 渐变向量（终点 - 起点）
    s32_t gradient_vec_mod_pow;              // 向量模的平方
    ipgui_gradient_color_stop_t stops[IPGUI_GRADIENT_STOP_MAX];
    u32_t stop_nr : 16;
} ipgui_liner_gradient_color_t;
```

**位置计算** — 点到渐变向量投影法:
```
pos_at_xy = dot_product((P - start), gradient_vector) / |gradient_vector|² * 255
```

- 如果 `gradient_vec_mod_pow == 0`（起点=终点），强制设为 1 防止除零
- 起始位置之前返回第一个 stop 的颜色，终点之后返回最后 stop 的颜色

**双模式初始化**:
- `ipgui_liner_gradient_init()`: 比例模式（0.0-1.0 归一化坐标），通过 `apply_to_aabb()` 转换为绝对坐标
- `ipgui_liner_gradient_init_direct()`: 直接坐标模式，跳过 AABB 转换

#### 2.4.2 径向渐变

```c
typedef struct {
    ipgui_coord_t center_x, center_y;  // 中心点
    ipgui_coord_t radius;             // 半径
    ipgui_coord_t radius_pow;         // 半径的平方（加速距离比较）
    ipgui_gradient_color_stop_t stops[IPGUI_GRADIENT_STOP_MAX];
    u32_t stop_nr : 16;
} ipgui_radial_gradient_color_t;
```

**位置计算**:
```
dist_pow = (x - center_x)² + (y - center_y)²
pos = sqrt(dist_pow) / radius * 255
     ≈ dist_pow * 255 / radius_pow   (近似优化)
```

#### 2.4.3 锥形（角度）渐变

```c
typedef struct {
    ipgui_coord_t center_x, center_y;
    u32_t angle_start : 16;    // 起始角度（度，归一化到0-360）
    u32_t stop_nr : 16;
    ipgui_gradient_color_stop_t stops[IPGUI_GRADIENT_STOP_MAX];
} ipgui_conic_gradient_color_t;
```

**位置计算** — 笛卡尔坐标系中逆时针（屏幕坐标系顺时针）:
```
angle = atan2(y - center_y, x - center_x)
normalized_angle = (angle + 360 - angle_start) % 360
pos = normalized_angle * 255 / 360
```

#### 2.4.4 Stop 点颜色插值

```c
static u32_t interpolate_pixel(u32_t x, u8_t a, u32_t y, u8_t b) {
    return ((x * a + y * b + 127) / 255);
}
```

**实现细节**: stop 点按 `pos` 升序排列，目标位置落入两个 stop 之间时，按线性比例插值 RGBA 四通道。支持 LUT 加速表（`grad_lerp_lut[256][256]`），预计算所有可能的两点插值结果。

---

### 2.5 图元绘制子系统

#### 2.5.1 像素绘制 (`ipgui_draw_pixel`)

最基础的绘制单元，通过 `ipgui_blend_color` 将单个像素合成到目标表面。

```
ipgui_draw_pixel(surf, clip, x, y, color, mask, opacity, blend_mode)
  → 坐标裁剪
  → 颜色预乘 + 透明度组合
  → premult_blend_table[pix_fmt](color, pix_ptr, blend_mode)
```

#### 2.5.2 线段绘制 (`ipgui_draw_line`)

`ipgui_draw_line.c` 提供两套公开 API 和三套底层渲染路径，覆盖细线（1px Wu 反走样）到任意宽度的线段绘制，支持线帽（平头/圆头）和完整的渐变着色（纯色 / 线性 / 径向 / 锥形）。

##### 2.5.2.1 公开 API

| API | 用途 | 适用场景 |
|-----|------|---------|
| `ipgui_draw_line_classic()` | Wu 反走样细线算法，仅绘制 1px 宽线条 | 细线、网格线、图表轴线 |
| `ipgui_draw_line_generic()` | 通用宽线绘制，支持任意线宽和线帽 | UI 线段、进度条、分隔线、描边 |

##### 2.5.2.2 数据结构

**线段描述 `ipgui_line_t`**：

```c
typedef struct {
    ipgui_point_t start;  // 线段起点
    ipgui_point_t end;    // 线段终点
} ipgui_line_t;
```

**线段样式 `ipgui_line_style_t`**：

```c
typedef struct {
    ipgui_coord_t        width;       // 线宽（>= 1）
    ipgui_line_cap_t     cap;         // 线帽类型：BUTT 平头 / ROUND 圆头
    ipgui_paint_t        paint;       // 着色来源（纯色或渐变）
    u8_t                 opacity;     // 整体不透明度
    ipgui_blend_mode_t   blend_mode;  // 混合模式
} ipgui_line_style_t;
```

**线帽类型**：

| 类型 | 值 | 视觉效果 |
|------|---|---------|
| `IPGUI_LINE_CAP_BUTT` | 0 | 平头端点，线条在端点处截断 |
| `IPGUI_LINE_CAP_ROUND` | 1 | 圆头端点，端点处绘制半圆帽 |

**渐变方向枚举**（在头文件中定义，供未来扩展）：

```c
typedef enum {
    IPGUI_LINE_GRADIENT_FOLLIOW = 0,  // 沿线条方向渐变（起点→终点）
    IPGUI_LINE_GRADIENT_HOR,          // 线内水平渐变
    IPGUI_LINE_GRADIENT_VER,          // 线内垂直渐变
} ipgui_line_gradient_dir_t;
```

##### 2.5.2.3 渲染路径详解

`ipgui_draw_line_generic()` 作为总调度器，根据线段方向将绘制请求分派到三条内部路径之一：

```
ipgui_draw_line_generic(surf, clip, line, style)
    |
    ├── line->start.x == line->end.x ?
    |       → ipgui_draw_ver_line()     [垂直宽线]
    |
    ├── line->start.y == line->end.y ?
    |       → ipgui_draw_hor_line()     [水平宽线]
    |
    └── 否则:
            → ipgui_draw_skew_line()    [斜线·宽线]
```

**（1）Wu 反走样细线 —— `ipgui_draw_line_classic()`**

经典的 Wu 反走样算法，仅绘制 1px 宽度的线条。核心思想是在主轴上每个像素位置同时绘制两个像素（主像素 + 相邻像素），用透明度来模拟亚像素覆盖。

```
算法核心:
  flatten 判定: |dx| > |dy| → 以 x 为主轴遍历
  non-flatten:  以 y 为主轴遍历

  对主轴上的每个整数坐标:
    cover  = 255 - |err| * 255 / 主轴长度   (0~255)
    alpha  = cover * opacity / 256
    premult = color × alpha
    
    主像素(x, y):          以 opacity 混合 premult
    相邻像素(x, y+step):   以 complement opacity 混合 premult
    err += 副轴增量
```

特点：
- 默认覆盖 Wu 反走样的标准行为 —— 两像素共享 opacity 的双线绘制
- 支持所有 paint 类型（纯色、线性/径向/锥形渐变）
- 不支持 IMAGE paint 类型
- 无 clip 时用 surf 自身围盒做裁剪

**（2）水平 / 垂直线 —— `ipgui_draw_hor_line()` / `ipgui_draw_ver_line()`**

水平线和垂直线共享相同的绘制策略：
1. 用 `ipgui_blend()` 批量填充线条矩形主体
2. 若 `cap == IPGUI_LINE_CAP_ROUND`，在两端点各绘制一个圆形（`ipgui_draw_arc` 实心半圆），圆半径为 `width/2`

```
水平线主体计算:
  self.start.x = min(start.x, end.x)
  self.end.x   = max(start.x, end.x)
  self.start.y = start.y - width/2
  self.end.y   = self.start.y + width - 1

垂直线主体计算:
  self.start.y = min(start.y, end.y)
  self.end.y   = max(start.y, end.y)
  self.start.x = start.x - width/2
  self.end.x   = self.start.x + width - 1
```

然后通过 `ipgui_aabb_overlap` 进行两层裁剪（clip → self），最后调用 `ipgui_blend()` 完成矩形填充。整个主体仅需一次 blend 调用，效率极高。

圆头线帽实现：
```c
ipgui_arc_t circle;
circle.cx = line->start.x;  // 起点圆心
circle.cy = line->start.y;
circle.er = style->width >> 1;  // 半径 = 线宽 / 2
circle.ir = 0;
circle.start = 0;
circle.angle = 360;
circle.dir   = IPGUI_ARC_DRAW_DIR_CCW;
// 同样的 circle_style 用于两端点
ipgui_draw_arc(surf, NULL, &circle, &circle_style);
```

**（3）斜线·宽线 —— `ipgui_draw_skew_line()`**

宽斜线（非水平非垂直的线段）采用 **edge WDF mask + 端点裁剪** 的方式。这是最复杂的渲染路径。

流程：
1. **计算线段围盒** — 由两个端点生成 AABB，按 `width/2` 向外扩展
2. **分配 mask 缓冲区** — 调用 `ipgui_mask_buf_acquire()` 获取逐行 buffer
3. **生成 WDF 宽边遮罩** — 用线段端点初始化 `ipgui_edge_wdf_param_t`，调用 `ipgui_gen_edge_wdf_mask_dsc()` 生成遮罩描述符，然后逐行调用 `ipgui_edge_wdf_mask()` 填充 mask
4. **端点裁剪** — 调用 `init_cross_edge()` 生成两端点的横截线参数（垂直于线段方向），再用 `edge_clip_mask_with_aa()` 对 mask 做带反走样的半平面裁剪，切出干净的端点
5. **blend 渲染** — 将裁剪后的 mask 传给 `ipgui_blend()` 按行渲染
6. **圆头线帽** — 若 `cap == IPGUI_LINE_CAP_ROUND`，复用同水平/垂直线的方式在两端绘制圆形
7. **释放 mask buffer** — `ipgui_mask_buf_free()`

`init_cross_edge()` 端点横截线生成原理：

横截线经过端点，方向垂直于线段。

```
已知线段方向向量为 (dx, dy)，则横截线方向为 (-dy, dx)
取 deltax = 256（一个合适的整数距离）
计算 deltay = -dx * deltax / dy
得到横截线上的两点 (x0+deltax, y0+deltay) 和 (x0-deltax, y0-deltay)
```

这样生成的横截线用于 `edge_clip_mask_with_aa()` 裁剪 WDF mask，形成规整的平头端点。关键在于 dx 的符号决定了 "保留左侧 / 保留右侧" 的裁剪方向。

##### 2.5.2.4 参数有效性检查

所有公开 API 在入口处执行统一的参数校验：

```
ipgui_draw_line_generic():
  拒绝条件: surf == NULL || line == NULL || style == NULL
  拒绝条件: opacity < 3（透明度太低不可见）
  拒绝条件: width < 1

ipgui_draw_line_classic():
  拒绝条件: surf == NULL || line == NULL || style == NULL
  拒绝条件: opacity < 3
  拒绝条件: paint.type == IPGUI_PAINT_IMAGE（不支持图像纹理）
  clip 有效时: 线段与 clip 无交集则直接返回
```

##### 2.5.2.5 性能特点

| 路径 | 优缺点 |
|------|--------|
| Wu 反走样（classic） | 仅 1px 宽，逐像素计算，适合细线；每个主轴坐标画 2 个带透明度像素 |
| 水平/垂直线（hor/ver） | 极快 — 整个主体仅一次 `ipgui_blend()` 调用；圆头线帽需额外汇制两个圆 |
| 斜线·宽线（skew） | 需要 mask 缓冲区、逐行 WDF 计算 + 端点裁剪；性能开销较高但能绘制任意宽度的反走样斜线 |

##### 2.5.2.6 功能对比总结

| 特性 | `ipgui_draw_line_classic()` | `ipgui_draw_line_generic()` |
|------|----------------------------|----------------------------|
| 线宽 | 固定 1px | 任意 `width >= 1` |
| 线帽 BUTT | — | 支持 |
| 线帽 ROUND | — | 支持（绘制半圆端点） |
| 纯色着色 | 支持 | 支持 |
| 线性渐变 | 支持 | 支持 |
| 径向渐变 | 支持 | 支持 |
| 锥形渐变 | 支持 | 支持 |
| 图像着色 | 不支持 | 不支持 |
| 渲染方式 | 逐像素 Wu 算法 | hor:blend / ver:blend / skew:WDF mask |
| 裁剪 | AABB clip + surf 裁剪 | AABB clip + surf 裁剪 |
| 圆角端点 | 不支持 | 复用 `ipgui_draw_arc()` 画圆

#### 2.5.3 多边形光栅化 (`ipgui_draw_polygon`)

基于 **扫描线 + 活动边表（Active Edge Table）** 的经典多边形光栅化。

**数据结构**:
```c
typedef struct {
    avl_tree_t edge_buckets;   // 按 y_start 排序的边桶（AVL 树）
    avl_tree_t scanline_cells; // 按 x 排序的扫描线 cell（AVL 树）
    ipgui_edge_t * active;     // 活动边链表
    ipgui_coord_t y_max, y_min;
    ipgui_fill_rule_t fill_rule; // 填充规则（非零环绕/奇偶）
} ipgui_polygon_ras_t;
```

**完整流程**:
```
1. ipgui_polygon_ras_init(): 初始化光栅化器 + 边缘内存池
2. ipgui_polygon_ras_add_path(): 添加多边形路径
   → 取每条边，按 y_start 插入 edge_buckets AVL 树
   → 使用 membox 分配边对象
3. ipgui_polygon_ras_rasterize(): 逐条扫描线
   → 从 edge_buckets 取 y = 当前扫描线的所有新边加入 active 链表
   → 从 active 链表中移除 y_end < 当前扫描线的过期边
   → 对 active 边按 x 排序，计算填充区间
   → 填充规则：非零环绕或奇偶规则
   → 每完成一条扫描线，active 边的 x 向前步进
4. ipgui_polygon_ras_runtime_clear(): 清理全部资源
```

**性能优化**:
- 使用 `membox`（固定大小内存盒）分配边和 cell，避免频繁 `malloc/free`
- 嵌入 10 条边 + 50 个 cell 的预分配空间
- 使用 AVL 树对边桶和扫描线 cell 排序，O(log n) 插入
- x_full_step 预计算完整步长，x_step_ycor 用于子像素精度步进

#### 2.5.4 弧 / 圆 / 扇形 / 圆环绘制 (`ipgui_draw_arc`)

`ipgui_draw_arc` 是一个统一的圆形几何图元绘制接口，通过调节 `ir`（内圆半径）和 `angle`（绘制角度）两个核心参数，可以覆盖四种不同的几何形状：

| ir | angle | 绘制结果 |
|----|-------|---------|
| =0 | < 360 | **扇形**（pie / sector）：从圆心出发的扇形区域 |
| =0 | =360 | **实心圆**（filled circle） |
| >0 | < 360 | **圆弧**（arc）：内外径之间的弧形条带 |
| >0 | =360 | **圆环**（ring / doughnut） |

基于环形遮罩 (`ipgui_ring_mask`) + 边沿遮罩 (`ipgui_edge_halfplane_mask`) 的组合。

**核心思路**: 
1. 先从 ring mask 缓存获取圆环遮罩数据（外径 `er`、内径 `ir`）
2. 用两条半平面边（起始角边、终止角边）对圆环遮罩做裁剪
3. 如果绘制扇形（`ir=0`），再加上圆心方向的剪裁（半平面过圆心补全扇形边界）
4. 将最终 mask 数据传给 `ipgui_blend()` 完成渲染

**四种形状示例**：

```c
/* 1. 实心圆: 圆心 (50,50), 半径 30 */
ipgui_arc_t circle = {
    .cx = 50, .cy = 50, .er = 30, .ir = 0,
    .start = 0, .angle = 360, .dir = IPGUI_ARC_DRAW_DIR_CCW
};

/* 2. 圆环: 外径 30, 内径 25, 实心圆边框效果 */
ipgui_arc_t ring = {
    .cx = 100, .cy = 100, .er = 30, .ir = 25,
    .start = 0, .angle = 360, .dir = IPGUI_ARC_DRAW_DIR_CCW
};

/* 3. 扇形（饼形）: 圆心 (150,60), 半径 40, 从 30° 逆时针画 120° */
ipgui_arc_t sector = {
    .cx = 150, .cy = 60, .er = 40, .ir = 0,
    .start = 30, .angle = 120, .dir = IPGUI_ARC_DRAW_DIR_CCW
};

/* 4. 圆弧: 圆心 (200,60), 外径 25, 内径 15, 从 30° 顺时针画 270° */
ipgui_arc_t arc_strip = {
    .cx = 200, .cy = 60, .er = 25, .ir = 15,
    .start = 30, .angle = 270, .dir = IPGUI_ARC_DRAW_DIR_CW
};

/* 共用样式 */
ipgui_arc_style_t style = {
    .paint       = { .type = IPGUI_PAINT_COLOR, .color = {255,0,0,255} },
    .opacity     = 255,
    .sep_type    = IPGUI_ARC_ENDPOINT_TYPE_BUTT,
    .eep_type    = IPGUI_ARC_ENDPOINT_TYPE_BUTT,
    .blend_mode  = IPGUI_BLEND_NORMAL
};
```

**关键参数说明**：

| 参数 | 作用 | 各形状设置值 |
|------|------|-------------|
| `arc->ir` | 内圆半径 | 0 = 实心圆/扇形; >0 = 圆弧/圆环 |
| `arc->er` | 外圆半径 | 圆的半径 / 外径 |
| `arc->angle` | 绘制角度 (°) | 360 = 全圆/全环; <360 = 弧/扇形 |
| `arc->start` | 起始角度 (°) | 仅对弧/扇形有意义 |
| `arc->dir` | 绘制方向 | CW / CCW（全圆/全环时任意） |
| `style->sep_type` | 起始端点类型 | ROUND = 端部半圆帽, BUTT = 平头 |
| `style->eep_type` | 结束端点类型 | 同上 |

**角度归一化机制**：函数内部将 `angle > 359` 统一归为 `360`，并通过 `while (start_angle < 0) start_angle += 360; while (start_angle >= 360) start_angle -= 360` 将起始角归一化到 `[0, 360)`。随后通过四象限分块（每 90° 一块）逐象限调用 `draw_quarter()` 绘制，自动覆盖全圆。

---

### 2.6 Box 绘制子系统

Box 绘制实现了 CSS 风格的盒模型渲染，支持背景、边框、阴影，且全部带圆角支持。

#### 2.6.1 盒模型结构

```
┌────────────────────────────────────┐
│  border_box (padding_box ± bw)    │  ← border
│  ┌──────────────────────────────┐  │
│  │  padding_box                 │  │  ← 圆角在此计算
│  │  ┌────────────────────────┐  │  │
│  │  │  content_box           │  │  │
│  │  └────────────────────────┘  │  │
│  └──────────────────────────────┘  │
└────────────────────────────────────┘
```

```c
typedef struct {
    ipgui_coord_t left_padding, right_padding;
    ipgui_coord_t top_padding, bottom_padding;
    ipgui_coord_t left_top_radius, right_top_radius;
    ipgui_coord_t left_bottom_radius, right_bottom_radius;
} ipgui_box_style_t;
```

**圆角半径约束**: `max_radius = min(padding_w/2, padding_h/2)`，防止圆角重叠。

#### 2.6.2 Box 背景绘制 (`ipgui_draw_box_background`)

**分块策略** — 将带圆角的矩形分解为 5 个矩形块 + 4 个圆角块:

```
┌────┬──────────┬────┐
│ TL │   top    │ TR │  5 个矩形块:
├────┼──────────┼────┤    ① 中间竖条（全高，避开圆角）
│    │          │    │    ② 左侧竖条（高度避开两角）
│ L  │  middle  │ R  │    ③ 右侧竖条（同上）
│    │          │    │    ④⑤ 左右半径差补齐块
├────┼──────────┼────┤
│ BL │  bottom  │ BR │  4 个圆角块: 各调用 draw_one_corner()
└────┴──────────┴────┘
```

`draw_one_corner()` 函数通过 `x_step` 和 `y_flip` 参数，用同一套代码覆盖四个角的不同几何变换。

#### 2.6.3 Box 边框绘制 (`ipgui_draw_box_border`)

类似背景的分块策略，但矩形块分布在边框四边的带状区域和四个圆角的弧形区域。边框圆角半径 = padding 圆角 + border_width。

#### 2.6.4 Box 阴影绘制 (`ipgui_draw_box_shadow`)

提供与 CSS `box-shadow` 视觉效果高度一致的矩形阴影渲染能力，同时支持外阴影（outset）、内阴影（inset）两种模式。

**一、算法原理概览**

传统 2D 高斯卷积方案需要为每个像素执行 `blur²` 次采样，计算量 O(W·H·b²)，在嵌入式环境下不可接受。本模块采用全新的 **SDF（有符号距离场）+ 1D 多项式映射** 算法，将 2D 卷积降维为 1D 查表：

```
像素坐标 (x,y)
    │
    ▼
sdf_rounded_box_q8()  ← 解析 O(1) 计算圆角矩形 SDF 距离 d
    │
    ▼
blur_lut[d + blur]    ← 1D LUT 映射距离 → 透明度
    │
    ▼
ipgui_draw_pixel()    ← 输出最终像素
```

**二、核心组件**

**(A) 圆角矩形 SDF（Signed Distance Field）**

采用纯整数 Q8 定点实现。给定像素到圆角矩形的有符号距离，公式为：

```
d = sqrt(max(qx,0)² + max(qy,0)²) + min(max(qx,qy), 0) - r

其中: qx = |cx| - w/2 + r
      qy = |cy| - h/2 + r
      r  = 圆角半径
```

该公式在 O(1) 时间内解析计算出距离，无需任何迭代或采样。

**(B) 1D 模糊剖面多项式映射**

利用数学事实：1D SDF 距离 → 透明度 的多项式 smoothstep 映射等价于高斯卷积的 CDF（累积分布函数）采样。提供三种可选多项式：

| 多项式 | 公式 | 与 erf(t) 最大误差 | 适用场景 |
|--------|------|-------------------|----------|
| **smoothstep3** | t²·(3-2t) | ≈0.017 | 默认推荐，C¹ 连续 |
| **smoothstep5** | t³·(10-15t+6t²) | ≈0.007 | 大 blur（>16px），边缘更柔和 |
| **quadratic** | t·(2-t) | ≈10% | 超低功耗 MCU，一个乘法 |

误差均在人眼 8bit 深度下不可分辨（smoothstep3/5）或可接受（quadratic）。

**(C) 1D 模糊剖面 LRU 缓存**

LUT 长度 = `2 × blur + 1` 字节。按 `(blur, algo)` 键缓存，LRU 淘汰，全局上限 8 项。典型 16px 阴影仅占 33 字节。同 blur 半径的多次绘制完全共享缓存，避免重复计算。

**(D) 纯整数平方根 (`ipgui_sqrt32`)**

二分位法（Binary Digit-by-Digit）实现，16 次迭代，ARM Cortex-M 上约 40~60 周期，无需硬件除法器，完全无分支。

**三、渲染流程**

以**外阴影**为例：

```
1. 参数规整化
   - blur < 0 → 0
   - corner_radius clamp 到 min(w/2, h/2)
   - opacity < 2 → 跳过

2. 计算阴影实体框 (entity_box)
   entity_box = content_box 向外扩展 spread，偏移 (offset_x, offset_y)
   entity_r   = corner_radius + spread

3. 裁剪遍历范围
   x ∈ [entity_x - blur, entity_x + entity_w + blur]
   y ∈ [entity_y - blur, entity_y + entity_h + blur]

4. 逐像素渲染
   for each (x,y) in range:
       d = sdf_rounded_box(lx, ly, entity_w, entity_h, entity_r)
       alpha = (blur == 0) ? (d ≤ 0 ? 255 : 0) : lut[clamp(d/256 + blur, 0, 2×blur)]
       
       // 挖除 content_box：阴影在内容后方被遮挡
       cd = sdf_rounded_box(clx, cly, content_w, content_h, content_r)
       if cd ≤ 0: continue
       if cd < 1: alpha *= cd/256  // 边缘 1px 抗锯齿
       
       ipgui_draw_pixel(x, y, color, alpha, opacity, blend_mode)
```

**内阴影**流程对称，区别在于：遍历范围限定在 content_box 内；像素超过 content_box 边界则跳过；content_box 边缘做反向抗锯齿。

**四、性能特征**

| 指标 | 传统 2D 高斯 | 本实现 SDF+1D |
|------|-------------|---------------|
| 时间复杂度 (每像素) | O(blur²) | O(1) |
| 空间复杂度 | O(blur²) 模板 | O(blur) LUT |
| blur=8 典型遍历像素 | 200×100 全屏 20k | ~3,400 |
| 浮点依赖 | 高斯卷积需要 | 零浮点，纯定点 Q8 |
| 硬件要求 | FPU + 大模板 | Cortex-M0 即可 |

**五、API 接口**

```c
// 外阴影（阴影在内容框后方）
void ipgui_draw_box_shadow_outset(surf, clip, content_box, style);

// 内阴影（阴影在内容框内部）
void ipgui_draw_box_shadow_inset(surf, clip, content_box, style);

// 自动根据 style->inset 选择内/外
void ipgui_draw_box_shadow(surf, clip, content_box, style);
```

**六、与旧算法对比**

| 对比维度 | 旧算法 (`ipgui_mask_gradient`) | 新算法 (SDF+1D) |
|----------|-------------------------------|-----------------|
| 基本原理 | 生成整幅遮罩图 → 渐变填充 | 逐像素 SDF 距离 → 1D 查表 |
| 内存占用 | O(W×H) 全尺寸 mask 缓冲区 | O(blur) + O(1) 临时变量 |
| 计算量 | mask 生成 + gradient 计算 + blend | SDF (O(1)) + LUT 查表 + blend |
| 浮点依赖 | 依赖于渐变引擎实现 | 纯定点 Q8，零浮点 |
| 圆角支持 | 需外部额外处理 | SDF 公式原生支持圆角 |
| blur 质量 | 取决于渐变定义 | 三次/五次 smoothstep 近似高斯 CDF |
| 内阴影 | 需外挂实现 | 同一渲染循环，SDF 反向 |
| 硬件适配 | 需大 mask 缓冲区 | Cortex-M0 级 MCU 可运行 |
| 像素级抗锯齿 | 取决于 mask 分辨率 | SDF 子像素级，1px 边缘平滑过渡 |

---

### 2.7 缓冲区管理

#### 图像缓冲区 (`ipgui_image_buf`)

```c
u8_t * ipgui_image_buf_acquire(u32_t w_stride, ipgui_coord_t h, ipgui_coord_t * res_h);
void   ipgui_image_buf_free(void * data);
```

- `res_h` 为实际分配到的高度（可能因内存不足少于请求高度）
- 调用者应逐行检查 `res_h`

#### 遮罩缓冲区 (`ipgui_mask_buf`)

```c
u8_t * ipgui_mask_buf_acquire(ipgui_coord_t w, ipgui_coord_t h, ipgui_coord_t * res_h);
void   ipgui_mask_buf_free(void * mask);
```

用于临时遮罩数据的生成与传递。用完立即释放。

---

## 3. core/composite 颜色合成模块

### 3.1 模块定位与职责

`core/composite` 负责将"颜色源"（纯色/渐变/图像）混合到目标表面上。它使用**预乘 Alpha**（premultiplied alpha）作为内部混合模型，支持多种像素格式和端序。

**目录结构**:

| 路径 | 职责 |
|------|------|
| `ipgui_blend.c/h` | 混合分发器（Facade） |
| `ipgui_blend_mode.h` | 混合模式枚举（Normal/Add/Multiply） |
| `ipgui_color.h` | 颜色联合体定义与操作宏 |
| `blend_color/` | 纯色混合（含像素格式转换和预乘混合） |
| `blend_gradient/` | 渐变颜色混合 |
| `blend_image/` | 图像像素混合 |
| `blend_icon/` | 图标混合 |

---

### 3.2 Paint 绘制系统

```c
typedef enum {
    IPGUI_PAINT_COLOR,     // 纯色
    IPGUI_PAINT_GRADIENT,  // 渐变
    IPGUI_PAINT_IMAGE,     // 图像
} ipgui_paint_type_t;

typedef struct {
    ipgui_paint_type_t type;
    union {
        ipgui_color_t     color;
        ipgui_grad_src_t  grad_src;
        ipgui_image_src_t image_src;
    } src;
} ipgui_paint_t;
```

**设计意图**: `ipgui_paint_t` 统一了纯色、渐变、图像的接口。上层绘制代码只需传入一个 `paint` 对象，混合分发器自动路由到对应的实现。

**颜色表示**: 所有内部颜色操作基于 **预乘 Alpha**（premultiplied alpha）。即 R/G/B 分量已乘以 A/255。这保证了混合公式 `result = fg + (1-alpha) * bg` 可以直接用整数加法。

---

### 3.3 颜色混合引擎

#### 3.3.1 像素格式转换 (`ipgui_blend_color`)

**核心函数指针表**:
```c
// 纯色 → packed 格式转换
typedef u32_t (* solid_convert_func_t)(ipgui_color_t color, u8_t * pix);

// 预乘颜色 → 像素混合
typedef void (* premult_blend_func_t)(ipgui_color_t color, u8_t * pix,
                                       ipgui_blend_mode_t blend_mode);
```

#### 3.3.2 RGB565 混合优化

最核心的优化——在 **packed 空间直接操作半字**，不做通道拆解:

```c
// 掩码定义
#define MASK_RB     0xf81f   // R[4:0]G[5:3] | G[2:0]B[4:0] → 提取 R+B
#define MASK_G      0x07e0   // 提取 G
#define MASK_MUL_RB 0x3e07c0 // MASK_RB << 6 = α_max 下的溢出范围
#define MASK_MUL_G  0x1f800  // MASK_G  << 6

// 混合公式: pack(fg565_scaled) + pack(bg565 * (1-α))
u32_t bg_rb = ((ialpha6 * (bg & MASK_RB)) & MASK_MUL_RB) >> 6;
u32_t bg_g  = ((ialpha6 * (bg & MASK_G )) & MASK_MUL_G ) >> 6;
result = ((fg565 & MASK_RB) + bg_rb) | ((fg565 & MASK_G) + bg_g);
```

**为什么安全**: 预乘后 fg 的 R/G/B 分量不会超过对应的 α，而 BG 乘以 (64-α)/64 后也不会溢出到相邻通道——因为 MASK 掩码阻止了进位传播。

#### 3.3.3 大小端自适应

所有 RGB888/ARGB8888 混合函数都通过 `#if IPGUI_ENDIAN_LITTLE` 分支处理端序差异:
- 小端: 内存 `[B][G][R][A]`
- 大端: 内存 `[A][R][G][B]`

#### 3.3.4 inv_table 查找表优化

除法替换为乘法查表: `g_inv_tbl[256]` 预计算 `255*255 / x`，用于 ARGB8888 之间的 Porter-Duff 合成中的除法操作。

---

### 3.4 渐变填充引擎

#### 3.4.1 通用渐变填充 (`ipgui_fill_gradient_color`)

**水平/垂直渐变快速路径**: 
- 水平线性渐变：对每个 x 列只计算一次颜色，复用填充整列
- 垂直线性渐变：对每个 y 行只计算一次颜色，复用填充整行

**通用渐变路径**: 逐像素计算渐变位置 → 查找 stop 颜色 → 混合。

#### 3.4.2 带遮罩渐变混合 (`ipgui_blend_gradient_color`)

```c
for (y = dest_start_y; y <= dest_end_y; y++) {
    for (x = dest_start_x; x <= dest_end_x; x++) {
        pos = ipgui_gradient_pos_at_xy(src, x, y);
        cr  = ipgui_gradient_color_get(src, pos);
        mask_alpha = mask ? mask_buffer[y][x] : 255;
        final_alpha = mask_alpha * opacity / 255;
        premult_blend(cr, pix, final_alpha, blend_mode);
    }
}
```

---

### 3.5 图像合成引擎

#### 3.5.1 图像数据模型

```c
// 完整图像（含像素数据，不含位置）
typedef struct {
    ipgui_coord_t w, h;
    u32_t stride;
    u8_t * pixmap;
    ipgui_image_fomat_t fmt;
    u8_t px_size;
} ipgui_image_data_t;

// 图像子区域（含定位信息）
typedef struct {
    ipgui_aabb_t * img_aabb;   // 图像包围盒（定位+裁剪）
    u32_t stride;
    u8_t * buf;
    ipgui_image_fomat_t img_pxfmt;
    u8_t px_size;
} ipgui_image_src_t;
```

**strided 设计**: `stride` 可以大于 `w * px_size`，支持大图中的子图采样或对齐优化。

#### 3.5.2 像素格式混合表

为每种源格式 × 目标格式组合提供专用混合函数:

| 源格式 | 目标 RGB565 | 目标 RGB888 | 目标 ARGB8888 | ... |
|--------|-------------|-------------|---------------|-----|
| L8     | blend_l8_2_rgb565 | blend_l8_2_rgb888 | blend_l8_2_argb8888 | |
| RGB565 | blend_rgb565_2_rgb565 | ... | | |
| RGB888 | ... | | | |
| ARGB8888 | ... | | Porter-Duff 合成 | |

每个函数都是专门针对特定格式对写的手动优化代码，不做运行时格式转换开销。

#### 3.5.3 两种图像混合 API

- `ipgui_blend_image_v1()`: img_src 的位置由 `img_aabb` 决定，自动裁剪
- `ipgui_blend_image_v2()`: 显式指定 `dest` 区域，img_src 用于提供像素源

---

## 4. 模块间交互逻辑与数据流

### 4.1 完整绘制流程

```
ipgui_screen_render()
  → 遍历脏矩形池
  → ipgui_screen_render_dirty_rect()
      → ipgui_rect_slice_ctx_init + ipgui_get_rect_slice (切片)
      → [对每个切片] ipgui_draw_dirty()
          → ipgui_widget_draw(widget)
              → ipgui_draw_box_background()
                  → 分块 (5 个矩形 + 4 个圆角)
                  → ipgui_blend(surf, clip, fill_aabb, paint, ...)
                      → 分发到 ipgui_blend_color / gradient / image
              → ipgui_draw_box_border()
              → ipgui_draw_text()
```

### 4.2 Mask → Paint → Blend 三角关系

```
        gfx (生成几何形状)
         │
         ├─→ mask buffer (0-255 透明度)
         │
         ↓
    composite (颜色合成)
         │
         ├─→ ipgui_blend(surf, clip, dest, paint, opacity, mask, mask_aabb, mode)
         │
         ↓
    写入 surf->color
```

---

## 5. 关键算法实现细节

### 5.1 Packed RGB565 混合

**原理**: RGB565 在 16 位中存储 R(5bit)|G(6bit)|B(5bit)。通过精心设计的掩码，可以在不拆解通道的情况下进行混合计算，单像素混合仅需 ~10 条指令。

**通道隔离**: 关键掩码 `0xF81F` 覆盖 R 和 B 位域但不覆盖 G，确保通道间无进位串扰。

### 5.2 多边形扫描线光栅化

**活动边表算法**（经典实现）:
1. 每条边按 `y_start` 插入桶（AVL 树）
2. 逐扫描线 y: 加入新边 → 排序 → 成对填充 → 步进 x → 删除到期边
3. 使用 26.6 定点数保证亚像素精度

**非零环绕规则实现**: 累加每条边对 cell 的 winding 贡献，最终 winding != 0 的 cell 填充。

### 5.3 圆角背景分块算法

将任意四个独立圆角的背景分解为最多 9 个矩形+圆角块的组合，完全避免了逐像素计算圆角的开销。只有在四个圆角块的绘制中才使用预计算的圆环 mask。

### 5.4 Stop 颜色插值

采用线性插值 `lerp(c1, c2, t)`，每个 RGBA 通道独立: `c1 * (255-t) + c2 * t + 127) / 255`。支持 LUT 加速。

### 5.5 修正因子 `correction_frac_index`

在 halfplane edge mask 中，点到直线的"轴向距离"d0 需要转换为"垂直距离"才是真实的覆盖值。修正因子 = `cos(θ)` 的近似值（θ 为直线与水平方向夹角），预计算了 127 个值。

---

## 6. 设计模式与最佳实践

| 模式 | 应用位置 | 说明 |
|------|---------|------|
| **策略模式** | `premult_blend_func_t` / `img_px_blend_func_t` 函数指针表 | 根据像素格式动态选择混合函数 |
| **外观模式** | `ipgui_blend()` | 统一 paint 类型分发，隐藏内部复杂度 |
| **组合模式** | `ipgui_paint_t` 的 union 设计 | 纯色/渐变/图像共享同一接口 |
| **享元模式** | `corner_mask_cache_item_t` 圆角遮罩缓存 | 同半径圆角复用预计算数据 |
| **模板方法** | `draw_one_corner()` 的 `x_step`/`y_flip` 参数 | 同一段代码处理四个角的几何变换 |
| **内存池** | `ipgui_membox_t` 用于多边形边缘/cell 分配 | 消除频繁 malloc/free |
| **查找表** | `g_inv_tbl[256]` / `grad_lerp_lut[256][256]` | 用空间换时间，消除除法 |

---

## 7. 使用示例与 API 快速参考

### 7.1 初始化与创建绘制面

```c
ipgui_surf_t surf;
surf.surf     = (ipgui_aabb_t){{0,0},{99,99}};
surf.color    = framebuffer;
surf.stride   = 100 * 2;  // RGB565
surf.pix_fmt  = PIX_FMT_RGB565;
surf.pix_size = 2;
```

### 7.2 纯色填充

```c
ipgui_paint_t paint;
paint.type = IPGUI_PAINT_COLOR;
IPGUI_COLOR_SET(paint.src.color, 255, 0xFF4080); // 粉色

ipgui_blend(&surf, NULL, &dest_aabb, &paint, 200, NULL, NULL, IPGUI_BLEND_NORMAL);
```

### 7.3 线性渐变填充

```c
ipgui_liner_gradient_color_t grad;
ipgui_liner_gradient_init(&grad, 0.0f, 0.0f, 0.0f, 1.0f); // 垂直渐变
ipgui_liner_gradient_apply_to_aabb(&grad, &box_aabb);

ipgui_gradient_color_stop_t stop1 = {{255,0,0,255}, 0};   // 红色在0%
ipgui_gradient_color_stop_t stop2 = {{0,0,255,255}, 255}; // 蓝色在100%
ipgui_liner_gradient_add_stop(&grad, &stop1);
ipgui_liner_gradient_add_stop(&grad, &stop2);

ipgui_paint_t paint;
paint.type = IPGUI_PAINT_GRADIENT;
paint.src.grad_src.grad_type = IPGUI_GRADIENT_TYPE_LINEAR;
paint.src.grad_src.grad.liner_grad = grad;

ipgui_blend(&surf, NULL, &dest_aabb, &paint, 255, NULL, NULL, IPGUI_BLEND_NORMAL);
```

### 7.4 圆角 Box 背景

```c
ipgui_box_style_t box_style = {0};
box_style.left_top_radius  = 8;
box_style.right_top_radius = 8;
box_style.left_bottom_radius = 8;
box_style.right_bottom_radius = 8;

ipgui_box_bg_style_t bg_style;
bg_style.paint   = paint;
bg_style.opacity = 255;
bg_style.blend_mode = IPGUI_BLEND_NORMAL;

ipgui_draw_box_background(&surf, NULL, &box_aabb, &box_style, &bg_style);
```

### 7.5 线段绘制

**Wu 反走样细线**：

```c
ipgui_line_style_t line_style;
line_style.paint   = paint;
line_style.opacity = 200;

ipgui_line_t line = {{10, 10}, {100, 80}};
ipgui_draw_line_classic(&surf, NULL, &line, &line_style);
```

**通用宽线（带圆头线帽）**：

```c
ipgui_line_style_t line_style;
line_style.paint      = paint;
line_style.opacity    = 200;
line_style.width      = 4;
line_style.cap        = IPGUI_LINE_CAP_ROUND;   // 圆头端点
line_style.blend_mode = IPGUI_BLEND_NORMAL;

ipgui_line_t line = {{10, 10}, {100, 80}};
ipgui_draw_line_generic(&surf, NULL, &line, &line_style);
```

**渐变着色宽线**：

```c
ipgui_gradient_color_t grad;
ipgui_liner_gradient_init(&grad, 10, 10, 100, 80);
ipgui_liner_gradient_add_stop(&grad, 0.0f, COLOR_RED);
ipgui_liner_gradient_add_stop(&grad, 1.0f, COLOR_BLUE);

ipgui_line_style_t line_style;
line_style.paint.type                    = IPGUI_PAINT_GRADIENT;
line_style.paint.src.grad_src.grad_type  = IPGUI_GRADIENT_TYPE_LINEAR;
line_style.paint.src.grad_src.grad.liner_grad = grad;
line_style.opacity = 255;
line_style.width   = 3;
line_style.cap     = IPGUI_LINE_CAP_BUTT;

ipgui_line_t line = {{10, 10}, {100, 80}};
ipgui_draw_line_generic(&surf, NULL, &line, &line_style);
```

### 7.6 多边形绘制

```c
// 全局光栅化器 g_ras 在 ipgui_init() 中初始化
ipgui_polygon_ras_reset(&g_ras);

ipgui_polygon_style_t poly_style;
poly_style.paint   = paint;
poly_style.opacity = 255;

ipgui_point_t points[] = {{10,10},{100,10},{100,100},{10,100}};
ipgui_draw_polygon(&surf, NULL, points, 4, &g_ras, &poly_style);
```

### 7.7 快速 API 对照表

| 功能 | API | 所在文件 |
|------|-----|---------|
| 纯色填充 | `ipgui_fill_color()` | `blend_color/ipgui_blend_color.c` |
| 纯色遮罩混合 | `ipgui_blend_color()` | `blend_color/ipgui_blend_color.c` |
| 渐变填充 | `ipgui_fill_gradient_color()` | `blend_gradient/ipgui_blend_gradient_color.c` |
| 渐变遮罩混合 | `ipgui_blend_gradient_color()` | `blend_gradient/ipgui_blend_gradient_color.c` |
| 图像混合 | `ipgui_blend_image_v2()` | `blend_image/ipgui_blend_image.c` |
| 通用混合 | `ipgui_blend()` | `composite/ipgui_blend.c` |
| 画点 | `ipgui_draw_pixel()` | `gfx/ipgui_draw_pixel.c` |
| 画线(经典Wu) | `ipgui_draw_line_classic()` | `gfx/ipgui_draw_line.c` |
| 画线(通用宽线) | `ipgui_draw_line_generic()` | `gfx/ipgui_draw_line.c` |
| 画多边形 | `ipgui_draw_polygon()` | `gfx/ipgui_draw_polygon.c` |
| 画 Box 背景 | `ipgui_draw_box_background()` | `gfx/ipgui_draw_box_background.c` |
| 画 Box 边框 | `ipgui_draw_box_border()` | `gfx/ipgui_draw_box_border.c` |
| 画圆弧 | `ipgui_draw_arc()` | `gfx/ipgui_draw_arc.c` |
| 渐变初始化(线性) | `ipgui_liner_gradient_init()` | `gfx/ipgui_gradient_color.c` |
| 渐变初始化(径向) | `ipgui_radial_gradient_init()` | `gfx/ipgui_gradient_color.c` |
| 渐变初始化(锥形) | `ipgui_conic_gradient_init()` | `gfx/ipgui_gradient_color.c` |
| 渐变停止点 | `*_gradient_add_stop()` | `gfx/ipgui_gradient_color.c` |
| 获取环形遮罩 | `ipgui_fetch_ring_mask()` | `gfx/ipgui_ring_mask.c` |
| 获取遮罩缓冲区 | `ipgui_mask_buf_acquire()` | `gfx/ipgui_mask_buf.c` |

---

> **文档编写依据**: 基于对 `core/gfx` 和 `core/composite` 目录下全部 `.c` 和 `.h` 源文件的逐行阅读与分析。
