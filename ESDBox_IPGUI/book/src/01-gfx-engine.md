# 第一章 嵌入式绘图引擎核心架构与实现

> 模块位置：`core/gfx/`  
> 核心职责：几何遮罩生成、渐变场计算、图元光栅化  
> 对外接口：遮罩数组 + Paint 描述 → 委托 composite 模块完成像素混合

---

## 1.1 架构概览

### 1.1.1 模块定位与职责边界

在 ESDBox_IPGUI 渲染管线的分层架构中，`core/gfx` 模块承担"形状感知"职责。它处理的是几何问题——直线的斜率、圆角的半径、三角形内点的判定、点到矩形边缘的有符号距离——而不涉及像素颜色的生成。颜色的合成工作完全委托给 `core/composite` 模块。

这种分工的意义在于解耦。gfx 模块输出两样东西：

1. **遮罩数组（mask）**：一个 `u8_t` 类型的数组，每个字节 0-255，指示对应像素的"颜色透过权"。255 表示完全不透明（颜色完全覆盖目标表面），0 表示完全透明（颜色不作用于该像素），中间值对应半透明过渡。
2. **涂料描述（paint）**：一个 `ipgui_paint_t` 类型的结构体，描述用于填充的颜色来源——纯色、渐变或图像。

gfx 模块的新增图元类型不影响 composite，composite 的混合算法优化不需要理解图元的几何特性。两个模块通过 `ipgui_blend()` 这一接口完成所有交互。

### 1.1.2 数据流全景

以绘制一个蓝色圆角按钮为例，数据在 gfx 和 composite 之间的流动如下：

```
用户代码: ipgui_draw_box_background(surf, clip, &abs, &shape, &bg_style)
    │
    ├── 步骤 1: 计算 padding_box（考虑 padding 的扩展矩形）
    ├── 步骤 2: 规整圆角半径 (get_max_radius)
    ├── 步骤 3: 将矩形分解为 5 个不含圆角的矩形条 + 4 个圆角区域
    ├── 步骤 4: 对每个圆角区域，调用 ipgui_fetch_ring_mask 获取环形遮罩
    │
    └── 步骤 5: 对每个子区域调用 ipgui_blend(surf, clip, dest, paint, opacity, mask, ...)
              → composite 模块接管：遮罩 × paint 颜色 × 目标像素 = 最终结果
```

关键在于：步骤 1-4 是纯粹的几何运算，完全不涉及颜色。步骤 5 是颜色运算，完全不涉及几何。这种清晰的边界使得两个模块可以独立测试、独立优化。

### 1.1.3 核心数据类型体系

gfx 模块定义了数个核心类型，它们构成了整个模块的类型骨架。理解这些类型是理解所有后续算法的前提。

```c
// ── 源文件: core/gfx/ipgui_color.h ──
typedef union {
    struct { u8_t b, g, r, a; };  // 小端序：内存布局为 [B][G][R][A]
    u32_t v;                       // 整体 32 位值
} ipgui_color_t;
```

`union` 设计同时支持结构化通道访问（`color.r`、`color.g`、`color.b`、`color.a`）和批量 32 位操作（`color.v`）。单条指令可同时检查四个通道是否全为零（`if (color.v == 0)`），或进行全通道的等值比较和赋值。

```c
// ── 源文件: core/gfx/ipgui_core.h ──
typedef struct {
    ipgui_aabb_t    surf;      // 表面在屏幕上的全局坐标范围
    u8_t          * color;     // 指向 surf 区域首像素的指针
    u32_t           stride;    // 每行字节跨度（可大于 w × pix_size）
    ipgui_pix_fmt_t pix_fmt;   // 像素格式枚举
    u8_t            pix_size;  // 每像素字节数
} ipgui_surf_t;

// ── 源文件: core/gfx/ipgui_prim.h ──
typedef struct {
    ipgui_coord_t start_x; ipgui_coord_t start_y;
    ipgui_coord_t end_x;   ipgui_coord_t end_y;
} ipgui_aabb_t;

typedef struct {
    ipgui_coord_t x; ipgui_coord_t y;
} ipgui_point_t;

typedef struct {
    ipgui_coord_t x; ipgui_coord_t y;
} ipgui_vector_t;
```

`ipgui_surf_t` 是绘制目标表面的描述符。注意 `stride` 可大于 `w × pix_size`——当 PFB（Partial Frame Buffer）的实际分配宽度大于脏矩形宽度时，stride 保证内存寻址的正确性。

`ipgui_aabb_t` 使用轴对齐包围盒（Axis-Aligned Bounding Box）描述矩形区域。这是整个系统中使用最频繁的数据结构——它同时用于描述绘制目标、裁剪区域、遮罩范围、控件边界。

### 1.1.4 gfx 模块的完整文件地图

```
core/gfx/
├── ipgui_edge_halfplane_mask.c/h  # 半平面边缘遮罩——反走样的基础单元
├── ipgui_edge_wdf_mask.c/h        # WDF 宽线厚度遮罩——宽线段的覆盖度渐变
├── ipgui_ring_mask.c/h            # 环形遮罩——圆角矩形 SDF + LRU 缓存
├── ipgui_mask_gradient.c/h        # 遮罩渐变——遮罩值沿线性/径向渐变分布
├── ipgui_mask_buf.c/h             # 遮罩缓冲区——临时遮罩内存的获取与释放
├── ipgui_image_buf.c/h            # 图像缓冲区——图像像素内存的获取与释放
├── ipgui_image_mask.c/h           # 图像遮罩——从图像数据生成遮罩值
├── ipgui_draw_line.c/h            # 线段绘制——Wu 经典细线 + 宽线 + 线帽
├── ipgui_draw_arc.c/h             # 弧/圆/扇形/圆环——统一接口覆盖四种形状
├── ipgui_draw_triangle.c/h        # 三角形光栅化——半边遮罩 + 三点排序法
├── ipgui_draw_polygon.c/h         # 多边形光栅化——扫描线 + AVL 活动边表
├── ipgui_draw_pixel.c/h           # 像素绘制——单像素混合
├── ipgui_draw_box_background.c/h  # Box 背景——圆角矩形分解策略
├── ipgui_draw_box_border.c/h      # Box 边框——四条边 + 四个圆角的组合
└── ipgui_draw_box_shadow.c/h      # Box 阴影——SDF + 1D 多项式模糊
```

---

## 1.2 子像素坐标系统

### 1.2.1 离散像素网格的根本局限

计算机屏幕由离散的像素网格构成。当一条数学上的理想线段穿过屏幕时，只有极少数像素的中心恰好落在线段上——绝大多数像素的中心与线段之间有一个不等于零的垂直距离。如果仅用整数坐标的描述来绘制，结果就是在视觉上产生锯齿（aliasing）。

反走样（anti-aliasing）解决这个问题的基本思路是：不把像素当作"要么画、要么不画"的二进制开关，而是根据像素中心到线段的距离，赋予每个像素一个 0 到 255 之间的透明度值。距离为零时透明度为 255（完全不透明），距离为一个像素以上时透明度为 0（完全透明），中间线性过渡。

实现这一思路需要比整数像素更细粒度的坐标系统。

### 1.2.2 Q26.6 定点数

系统使用 32 位有符号整数作为子像素坐标系的基础类型，定义为 Q26.6 定点数格式：

```c
// ── 源文件: core/gfx/ipgui_edge_halfplane_mask.h:17 ──
typedef s32_t ipgui_edge_coord_t; // Q26.6: 高26位为整数部，低6位为小数部
```

Q26.6 格式将每个整数像素等分为 64 个子像素单元。这意味着：

- 一个典型的 320×240 屏幕在子像素空间中的分辨率是 20480×15360
- 一条从 (2.3, 1.1) 到 (7.8, 5.6) 的线段在子像素空间中表示为从 (147, 70) 到 (499, 358)
- 可表示的坐标范围为 ±33,554,431 像素，远超任何实际屏幕

选择 64 而非其他值（如 256）作为子像素缩放因子，是基于两个工程考量：64 的乘法可以用 `<< 6` 实现，不依赖硬件乘法器；64 的精度对于 8 位颜色深度的 alpha 混合已经足够（64 级离散度的误差在 8 位空间中小于 1）。

### 1.2.3 整数坐标与子像素坐标的转换

以下四个工具函数构成了两种坐标系的转换桥梁：

```c
// ── 源文件: core/gfx/ipgui_edge_halfplane_mask.c:107-119 ──
ipgui_edge_coord_t dist_to_lower_64(ipgui_edge_coord_t c)
{
    return c & 63;                           // 取低 6 位 → 0~63
}

ipgui_edge_coord_t dist_to_upper_64(ipgui_edge_coord_t c)
{
    return (64 - (c & 63)) & 63;             // 到下一个 64 对齐点的距离
}

ipgui_edge_coord_t align_down_64(ipgui_edge_coord_t c)
{
    return c & ~(ipgui_edge_coord_t)63;      // 向下对齐到 64 的倍数
}

ipgui_edge_coord_t align_up_64(ipgui_edge_coord_t c)
{
    return (c + 63) & ~(ipgui_edge_coord_t)63; // 向上对齐
}
```

`dist_to_lower_64(c)` 返回 c 到其下方最近整数像素中心的子像素距离（c 本身的低 6 位）。`dist_to_upper_64(c)` 返回到上方最近整数像素中心的距离（64 - 该距离）。

`align_down_64(c)` 等价于 `(c / 64) * 64`，将子像素坐标向下对齐到整数像素边界。`align_up_64(c)` 则向上对齐。两者的实现都利用了位运算：64 是 2 的幂，对齐就是清零低 6 位。

注意 `dist_to_upper_64` 的最后一步 `& 63`——当 `c & 63 == 0`（c 恰好对齐）时，`64 - 0 = 64`，但该像素到"上方中心"的距离应该是 0 而非 64，`64 & 63 = 0` 修正了这一边界情况。

---

## 1.3 半平面边缘遮罩

### 1.3.1 基本原理

半平面（halfplane）是二维空间中由一条直线分割出的两个区域之一。给定一条有向线段（从 P1 到 P2），可以定义一个"左半平面"（线段的左侧）和一个"右半平面"（线段的右侧）。

半平面边缘遮罩的任务是：对于扫描线上的每个像素，判断它在半平面的哪一侧，求其到边缘线的子像素距离，并将该距离映射为 0-255 的透明度值。

这个基础单元被用于三角形的三条边遮罩、弧的角度裁剪、多边形的填充规则判定等几乎所有涉及"形状边界"的场景。

### 1.3.2 边缘参数结构体

每条边缘被参数化为一个紧凑的结构体：

```c
// ── 源文件: core/gfx/ipgui_edge_halfplane_mask.h:24-32 ──
typedef struct {
    ipgui_edge_coord_t   x1;       // y 较小的端点（按 y 排序后的结果）
    ipgui_edge_coord_t   y1;
    ipgui_edge_coord_t   dx;       // Δx（符号保留）
    ipgui_edge_coord_t   dy;       // Δy（恒为非负）
    s32_t                delta_x;
    s32_t                delta_y;  // 平坦边的垂直步进因子
    u8_t correction_frac_index : 7; // 距离修正因子索引（0~126）
    u8_t flatten : 1;               // 0=陡峭边, 1=平坦边（|dx| >= dy）
} ipgui_edge_param_t;
```

结构体中包含了两个关键设计：

1. **`flatten` 位域**：将边缘分为"平坦"（`|dx| >= dy`）和"陡峭"（`|dx| < dy`）两类。分类的意义在于距离计算方式不同——平坦边的扫描线沿水平方向前进，相邻像素的水平增量需要乘以斜率才能转换为垂直距离；陡峭边的扫描线沿垂直方向前进，相邻像素的垂直增量直接就是垂直距离。

2. **`correction_frac_index`**：存储修正因子在 `correction_frac` 表中的索引，范围 0-126。当边缘是精确的 45° 时，索引约为 126；当边缘接近水平/垂直时，索引接近 0。

### 1.3.3 边缘初始化

初始化函数完成三项工作：端点排序、平坦/陡峭分类、步进因子和修正因子索引的预计算。

```c
// ── 源文件: core/gfx/ipgui_edge_halfplane_mask.c:36-87 ──
ipgui_edge_param_t ipgui_edge_param_init(
    ipgui_edge_coord_t x1, ipgui_edge_coord_t y1,
    ipgui_edge_coord_t x2, ipgui_edge_coord_t y2)
{
    ipgui_edge_param_t param;
    ipgui_memset(&param, 0, sizeof(ipgui_edge_param_t));

    // 步骤 1：按 y 排序，确保 dy ≥ 0
    if (y2 > y1) {
        param.dy = y2 - y1; param.dx = x2 - x1;
        param.x1 = x1;       param.y1 = y1;
    } else {
        param.dy = y1 - y2; param.dx = x1 - x2;
        param.x1 = x2;       param.y1 = y2;
    }

    // 步骤 2：分类与预计算
    if (IPGUI_ABS(param.dx) >= param.dy) {
        param.flatten = 1;
        s64_t scaled_dy = (s64_t)param.dy << 16;   // Q16.16 格式
        s32_t half_dx   = param.dx / 2;
        if (param.dx > 0)
            param.delta_y = (scaled_dy + half_dx) / IPGUI_ABS(param.dx);
        else if (param.dx < 0)
            param.delta_y = (scaled_dy - half_dx) / IPGUI_ABS(param.dx);
        param.correction_frac_index =
            (u8_t)((param.dy * 126 + (IPGUI_ABS(param.dx) >> 1)) / IPGUI_ABS(param.dx));
    } else {
        param.flatten = 0;
        param.correction_frac_index =
            (u8_t)((IPGUI_ABS(param.dx) * 126 + (param.dy >> 1)) / param.dy);
    }
    return param;
}
```

**`delta_y` 的计算解析**：`delta_y` 是平坦边专用的垂直步进因子。在平坦边上，当扫描线水平方向每前进一个像素时，边缘线的 y 坐标的实际变化量是 `dy/|dx|`。`delta_y` 存储的是这个比值的 Q16.16 定点表示：`delta_y = (dy << 16) / |dx|`。乘以 `delta_y / 65536` 即可将水平距离转换为垂直距离。

`half_dx` 的加减用于四舍五入。`(scaled_dy + half_dx) / |dx|` 等价于数学上的 `round(dy * 65536 / |dx|)`。在除法前加上 `|dx|/2`，实现了整数的四舍五入而无需使用浮点。

**`correction_frac_index` 的计算解析**：该索引表示 `dy / |dx|` 在 [0, 1] 区间的位置（按比例映射到 0-126）。对于完全水平的边（dy=0），索引为 0；对于 45° 的边（dy=|dx|），索引为 126（最大值）。注意当 dy 接近 0 时，索引接近 0，修正因子接近 255——这意味着对于接近水平的边，轴向距离几乎等于垂直距离（cos≈1），几乎不需要修正。

### 1.3.4 修正因子表

```c
// ── 源文件: core/gfx/ipgui_edge_halfplane_mask.c:22-30 ──
const u8_t correction_frac[127] = {
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 254, 254,
    254, 254, 254, 253, 253, 253, 253, 252, 252, 252, 251, 251, 251, 250, 250,
    250, 249, 249, 248, 248, 248, 247, 247, 246, 246, 245, 245, 244, 244, 243,
    243, 242, 242, 241, 241, 240, 240, 239, 239, 238, 237, 237, 236, 235, 235,
    234, 233, 233, 232, 231, 231, 230, 229, 228, 228, 227, 226, 225, 224, 224,
    223, 222, 221, 220, 220, 219, 218, 217, 216, 215, 214, 213, 212, 211, 210,
    209, 208, 208, 207, 206, 205, 204, 203, 202, 201, 200, 199, 198, 197, 196,
    195, 194, 193, 192, 191, 190, 189, 188, 187, 186, 185, 184, 184, 183, 183
};
```

此表只有 127 个条目而非 256。原因是：当斜率小于 `1/127 ≈ 0.008` 时（即 `dy / |dx| < 1/127`），`dy` 与 `|dx|` 的比值已经小到 `cos(atan(dy/dx)) ≈ 1.0`，轴向距离与垂直距离的差异小于 1/256，此时的修正因子为 255（即不修正）。

表中的每个值代表 `cos(atan(slope_index / 126)) × 256`。表中的前半段（接近 255）对应几乎水平的边，末尾段（183-184）对应接近 45° 的边。

修正因子的作用是：`vertical_distance = horizontal_distance × correction_frac[index] / 256`。对于水平边，不需要修正（`×255/256 ≈ 1`）；对于 45° 边，需要修正（`×183/256 ≈ 0.715`），因为此时水平方向走一个像素，垂直方向离边缘才算走了 `cos(45°) ≈ 0.707` 个像素的距离。

### 1.3.5 扫描线描述器

对于每条扫描线 y，引擎生成一个描述器，记录"从该扫描线看，边缘交点的子像素位置"：

```c
// ── 源文件: core/gfx/ipgui_edge_halfplane_mask.h:34-40 ──
typedef struct {
    edge_halfplane_dir_t dir;    // 保留左半平面还是右半平面
    ipgui_coord_t        y;      // 当前扫描线 y 坐标
    ipgui_coord_t        x_start;// 第一个部分覆盖的像素 x 坐标
    ipgui_edge_coord_t   frac_x; // x_start 像素中心到边缘的子像素距离 (0~63)
    ipgui_edge_param_t * p;      // 边缘参数指针
} ipgui_edge_halfplane_mask_dsc_t;
```

`dir` 取值为 `EDGE_HALFPLANE_DIR_LEFT` 或 `EDGE_HALFPLANE_DIR_RIGHT`：

- **左半平面**：保留边缘的"左侧"区域。`x_start` 是边缘交点上取整，即从交点所在像素的下一个像素开始（因为交点所在像素的左侧实际上是在保留区域内）。
- **右半平面**：保留边缘的"右侧"区域。`x_start` 是边缘交点下取整，即交点所在的整数像素——该像素右侧是保留区域。

`frac_x` 是 `x_start` 像素中心到边缘的子像素距离（0-63）。它用于计算 `x_start` 本身的覆盖率：`mask = (64 - frac_x) * 4`。

### 1.3.6 逐像素遮罩值计算

```c
// ── 源文件: core/gfx/ipgui_edge_halfplane_mask.c:127-153 ──
u8_t ipgui_edge_halfplane_mask(
    ipgui_edge_halfplane_mask_dsc_t * dsc,
    ipgui_edge_coord_t x)
{
    ipgui_edge_coord_t d;       // 到边缘的垂直距离（子像素单位）
    ipgui_edge_coord_t delta_x; // 像素 x 到 x_start 的子像素距离

    if (dsc->dir == EDGE_HALFPLANE_DIR_LEFT) {
        if (x < dsc->x_start) return 255;            // 完全在保留侧 → 全透
        delta_x = ((x - dsc->x_start) << 6) + dsc->frac_x;
    } else if (dsc->dir == EDGE_HALFPLANE_DIR_RIGHT) {
        if (x > dsc->x_start) return 255;            // 完全在保留侧 → 全透
        delta_x = ((dsc->x_start - x) << 6) + dsc->frac_x;
    }

    // 对于平坦边，将轴向距离转换为垂直距离（乘以斜率）
    if (dsc->p->flatten)
        d = ((s64_t)delta_x * dsc->p->delta_y + 32768) >> 16;
    else
        d = delta_x;  // 陡峭边：水平增量本身即为垂直距离

    // 应用修正因子
    d = d * correction_frac[dsc->p->correction_frac_index] >> 8;

    // 距离到透明度的映射
    if (!d)       return 255;        // 像素中心恰好在边上
    else if (d > 64) return 0;       // 距离超过一个像素 → 完全不透
    else          return (64 - d) << 2;  // 部分覆盖：0→252, 64→0
}
```

**算法结构分析**：

该函数的核心是三个判定层次逐级深入：

1. **最外层（方向判定与快速返回）**：如果像素完全在保留侧（左半平面下 x < x_start，右半平面下 x > x_start），直接返回 255，无需计算距离。这对于大面积填充场景（例如圆角矩形中间的矩形条区域）极大地减少了计算量。

2. **中层（距离计算）**：计算 `delta_x`——当前像素到 x_start 的子像素距离。x_start 是该扫描线上第一个部分覆盖的像素，"当前像素到 x_start 的距离"等同于"当前像素到第一个部分覆盖像素的距离"，再在 x_start 的基础上加上 `frac_x` 得到该像素到边缘的总子像素距离。

   对于平坦边：`d = horizontal_offset × delta_y / 65536`，将水平像素偏移转换为垂直距离。`+32768`（即 `+0.5 << 16`）是四舍五入。

3. **最内层（距离到透明度映射）**：`(64 - d) * 4 = 256 - 4d`。当 d 从 0 到 64 线性增长时，透明度从 256 到 0 线性下降。取低 8 位后范围为 [255, 0]。

---

## 1.4 WDF 宽线遮罩

### 1.4.1 问题定义与设计动机

上一节讨论的半平面遮罩处理了"点到边缘的距离"这一个自由度。但宽线段需要处理两个自由度：沿线段方向的"进退"和垂直于线段方向的"厚度过渡"。

WDF（Width Dependent Falloff）遮罩专门处理宽线段的厚度方向遮罩。给定一条有向线段（起点 P1、终点 P2）和一个线宽参数 width，对于扫描线上的每个像素：

- 到线段中心线的垂直距离 ≤ half_width → mask = 255（像素在线的实体区域内）
- 距离 ≥ half_width + 1 → mask = 0（像素完全在线外）
- half_width < 距离 < half_width + 1 → mask 线性渐变（亚像素过渡）

这里的核心挑战是将"沿线段的前进量"转换为"扫描线上的子像素偏移量"——WDF 描述器中的每个扫描线对应一个"线段中心线在扫描线上的交点位置"。

### 1.4.2 数据结构

```c
// ── 源文件: core/gfx/ipgui_edge_wdf_mask.h ──
typedef struct {
    ipgui_edge_wdf_param_t * p;       // 线段边缘参数
    ipgui_xidx_t             x_idx;   // 线段中心线在扫描线上的交点（整数+小数）
    ipgui_xstep_t            x_step;  // x_idx 的每行步进量
    ipgui_coord_t            x_half_span; // 搜索半径：half_width + 1
    s32_t                    half_width64; // half_width << 6（子像素单位）
} ipgui_edge_wdf_mask_dsc_t;
```

与半平面遮罩的关键区别：WDF 描述器的 `x_half_span` 是 `half_width` 的 1~2 倍，限制了每行仅需在中心线左右有限范围内计算遮罩值。

### 1.4.3 逐行批量遮罩生成

```c
// ── 源文件: core/gfx/ipgui_edge_wdf_mask.c:193-295 ──
void ipgui_edge_wdf_mask(
    ipgui_edge_wdf_mask_dsc_t * dsc,
    ipgui_coord_t sx, u8_t * mask_buf, ipgui_coord_t len)
{
    // 步骤 1：计算本扫描线的遮罩起止位置
    ipgui_coord_t x_left, x_right;
    if (dsc->x_idx.frac) {
        // 有小数部分：交点位于两个整数像素之间
        x_left  = dsc->x_idx.inte - dsc->x_half_span;
        x_right = dsc->x_idx.inte + 1 + dsc->x_half_span;
        // 将分数部分转换为子像素坐标系下的"沿扫描线的偏移量"
        lfrac64 = ((s64_t)dsc->x_idx.frac << 6) / dsc->p->dy;
    } else {
        x_left  = dsc->x_idx.inte - dsc->x_half_span;
        x_right = dsc->x_idx.inte + dsc->x_half_span;
    }

    // 步骤 2：快速裁剪——与 mask_buf 区间无交集则返回全零
    ipgui_coord_t ex = sx + len - 1;
    if ((ex < x_left) || (sx > x_right)) {
        ipgui_memset(mask_buf, 0, len);
        return;
    }

    // 步骤 3：在有效区间外填充 0
    if (sx < x_left) {
        ipgui_coord_t left_zeros = x_left - sx;
        ipgui_memset(mask_buf, 0, left_zeros);
        mask_buf += left_zeros; len -= left_zeros; sx = x_left;
    }
    if (ex > x_right) {
        ipgui_coord_t right_zeros = ex - x_right;
        ipgui_memset(mask_buf + (len - right_zeros), 0, right_zeros);
        len -= right_zeros; ex = x_right;
    }

    // 步骤 4：在有效区间内逐像素计算遮罩（对应三种位置关系）
    // 分支 1: mask_buf 全部在 x_idx 左侧 → 从左边界向 x_idx 填渐增值
    // 分支 2: mask_buf 全部在 x_idx 右侧 → 从右边界向 x_idx 填渐增值
    // 分支 3: mask_buf 跨越 x_idx          → 两端向中间填，中间填 255
}
```

**三种位置关系的分支处理**：

当 mask_buf 跨越中心线（分支 3）时，从两端向中心线逐像素计算遮罩值。一旦遇到 mask=255 就停止——从该点到中心线的所有像素都在线的实体区域内，mask 值恒为 255。这种"遇到 255 即停止"的策略避免了不必要的 `correct_d_and_mask` 计算。

当 mask_buf 完全在中心线一侧（分支 1 或分支 2）时，只从远离中心线的一端向靠近中心线的一端计算。同样，遇到 mask=255 即停止。

**`lfrac64` 的数学意义**：

`lfrac64 = (frac << 6) / dy` 是将"沿线段方向的前进比例"转换为"沿扫描线方向的子像素偏移量"。frac 是当前扫描线交点在线段上的分数位置（Q26.6 格式），除以 dy 后得到交点相对于整数像素的偏移（0~63 范围）。这个值被用于计算每个像素到中心线的精确子像素距离——将"线段坐标系"中的位置映射到"屏幕坐标系"中的位置。

**性能分析**：

对于典型线宽（4-10 像素），`x_half_span` 通常是 `half_width + 1 = 3-6`，意味着每行仅需计算约 7-13 个像素的遮罩值。与该扫描线的总长度相比（通常为屏幕宽度），复杂度为 O(width) 而非 O(scanline_length)，减少了约 95%-98% 的计算量。

---

## 1.5 环形遮罩与 LRU 缓存

### 1.5.1 圆角矩形的 SDF 表示

环形遮罩（ring mask）描述的是"外径 er 和内径 ir 之间的环形区域"。对圆角矩形，环形遮罩是实现背景、边框、阴影绘制的基础几何元素。

圆角矩形的 SDF（Signed Distance Field）定义为：对于屏幕上的每个像素，计算其到圆角矩形边界的有符号距离。内部为负（ir 区域），边界为零，外部为正。

引擎不直接为每个圆角矩形计算 SDF，而是按行批量获取——`ipgui_fetch_ring_mask(circle_x_start, circle_y, ir, er, x_step, row_mask, w)` 一次性填充一行的遮罩值。核心参数包括：

- `circle_x_start`：该行在圆角坐标系中的起始逻辑 x（1 到 er）
- `circle_y`：该行在圆角坐标系中的逻辑 y（1 到 er）
- `er`：外径（圆角半径 + 边框宽度）
- `ir`：内径（圆角半径）
- `x_step`：水平方向（-1 为左侧圆角区，1 为右侧圆角区）

### 1.5.2 LRU 缓存策略

环形遮罩的 SDF 计算结果被缓存在一个 LRU（Least Recently Used，最近最少使用）缓存中。缓存的 key 为 `(er, ir)` 对，即外径和内径的组合。

缓存的淘汰策略是标准 LRU：

- 每次命中时将条目移动到链表头部（标记为最近使用）
- 当缓存满时淘汰链表尾部的条目（最久未被访问）
- 新条目插入链表头部

这种策略在 UI 渲染场景中特别有效。同一页面中的所有控件通常共享相同的圆角半径（例如所有卡片半径=8），在一帧的渲染过程中，同一 `(er, ir)` 组合会被多次命中——背景、边框、阴影的四个角都会需要相同的环形遮罩值。缓存命中率通常超过 90%。

---

## 1.6 线段绘制

### 1.6.1 三路径分发策略

线段绘制系统通过一个总调度函数将所有线段请求分发到三条渲染路径：

```
ipgui_draw_line_generic(surf, clip, line, style)
    │
    ├── dx == 0 ──→ ipgui_draw_ver_line()    # 垂直：矩形填充
    ├── dy == 0 ──→ ipgui_draw_hor_line()    # 水平：矩形填充
    │
    └── 其他 ────→ ipgui_draw_skew_line()    # 斜线：WDF 遮罩 + 端点裁剪
```

**垂直/水平快速通道**：

对于垂直和水平线段，引擎直接退化为矩形填充——线段的起点和终点构成矩形的一条边，线宽构成另一条边。整个矩形区域使用 `ipgui_blend()` 一次性填充，不使用遮罩。

这种退化的性能提升是巨大的：从 O(w×h) 的逐像素 WDF 遮罩生成降为 O(1) 的矩形填充。在嵌入式 MCU 上，二者的差距可能是一帧内的几十毫秒和几百微秒。

### 1.6.2 线帽实现

线帽（line cap）在线段端点处的处理复用已有模块：

- **IPGUI_LINE_CAP_BUTT**（平头）：在线段的端点处做横截线裁剪。横截线的方向垂直于线段方向。裁剪通过 `edge_clip_ring_mask_no_aa()` 或 `edge_clip_mask_with_aa()` 函数实现——将线段端点外的 mask 值清零。
- **IPGUI_LINE_CAP_ROUND**（圆头）：在线段端点处调用 `ipgui_draw_arc()` 绘制一个实心半圆（ir=0, angle=180°）。半圆的圆心位于线段端点，半径为 `width/2`。半圆的朝向与线段方向垂直。

线帽的复用设计避免了为线帽独立编写圆弧渲染逻辑。`ipgui_draw_arc()` 本身已经是调用 `ipgui_fetch_ring_mask()` + `edge_clip_mask_with_aa()`，线帽只需要传递合适的参数组合即可。

### 1.6.3 Wu 反走样细线

当线宽为 1（`line->width == 1`）时，引擎进入 Wu 经典反走样路径。该路径是经典 Wu 线段算法在 256 级 alpha 精度下的完整实现：

```
主轴为 x 轴（|dx| > |dy|）：
    对于每个整数 x 坐标：
        计算 err 值（当前扫描线相对于理想线的累积误差）
        计算 cover = 255 - |err| × 255 / |dx|
        主像素 (x, y)：        alpha = cover × opacity / 255
        相邻像素 (x, y ± 1)：  alpha = (255 - cover) × opacity / 255
        如果 cover >= 255 → 仅绘制主像素（误差为零，无需相邻补偿）
        如果 cover <= 0   → 仅绘制相邻像素（误差充满一整步）
        继续下一个 x：err += dy × 2
```

核心思想是：在每个主轴坐标（主像素）上，在垂直方向绘制两个相邻像素，其透明度之和等于前景色的总不透明度。人眼的亮度感知会将两个像素的亮度平均，在主观上看到的是一条平滑的亚像素宽度线段。

Wu 路径支持渐变和纯色两种涂料类型。对于渐变涂料，每个像素都会独立查询一次 `ipgui_gradient_color_get()` 以获取正确的渐变坐标颜色。

---

## 1.7 弧与圆的统一绘制接口

### 1.7.1 参数矩阵

系统用一个函数 `ipgui_draw_arc()` 覆盖四种几何形状，通过三个参数的组合实现：

| er | ir | angle | 结果形状 |
|-----|-----|-------|---------|
| = radius | 0 | 360° | 实心圆 |
| = radius | 0 | < 360° | 扇形（pie） |
| = outer_r | > 0 | 360° | 圆环 |
| = outer_r | > 0 | < 360° | 弧条 |

### 1.7.2 三步骤实现

绘制流程由三个子步骤组成：

**步骤 1**：从 ring mask 缓存获取对应 `(er, ir)` 的环形遮罩。如果缓存命中，跳过 SDF 计算。

**步骤 2**：用两条半平面边缘遮罩裁剪环形区域。第一条边沿起始角度方向（从圆心指向起始角度的方向），保留逆时针侧；第二条边沿终止角度方向，保留顺时针侧。两条边的交集确定了最终的角度范围。

**步骤 3**：对于 ir=0 的扇形（实心扇形），额外做圆心角方向的裁剪——两条边缘都从圆心出发，保证圆心处的像素被正确保留。

### 1.7.3 角度边缘裁剪的两种模式

角度边缘裁剪提供了两种精度模式：

- **`edge_clip_ring_mask_no_aa()`**：不计算半平面遮罩的精确覆盖率，直接做硬裁剪——边缘线左侧全保留（255）、右侧全抹除（0）。用于对质量要求不高但需要速度的场景。
- **`edge_clip_mask_with_aa()`**：计算半平面遮罩的精确覆盖率，边缘线两侧像素按覆盖率做软过渡。用于对视觉质量有要求的场景。

`edge_clip_ring_mask_no_aa()` 的实现逻辑：

```c
// ── 源文件: core/gfx/ipgui_draw_arc.c ──
// 左半平面：保留边缘左侧，抹除右侧
// dsc.x_start 是边缘交点的整数坐标
// x_start + 1 及其右侧的所有像素清零
if (mask_aabb->end.x > dsc.x_start) {
    ipgui_coord_t clear_start = IPGUI_MAX(0, dsc.x_start + 1 - mask_aabb->start.x);
    if (clear_start < mask_stride) {
        ipgui_memset(row_mask + clear_start, 0, mask_stride - clear_start);
    }
}

// 右半平面：保留边缘右侧，抹除左侧
// x_start 及其左侧的所有像素清零
if (mask_aabb->end.x >= (dsc.x_start + 1)) {
    mask_len_tmp -= IPGUI_MIN(mask_stride, mask_aabb->end.x - dsc.x_start);
}
ipgui_memset(row_mask, 0, mask_len_tmp);
```

---

## 1.8 三角形光栅化

### 1.8.1 算法总览

三角形光栅化是图形管道中最基础的光栅化操作之一。引擎采用了经典的三段式扫描线光栅化算法：

1. 将三角形按 y 坐标最小的顶点切分为"下三角形"和"上三角形"
2. 分别对两段进行扫描线光栅化
3. 单独处理中间顶点所在扫描线

### 1.8.2 三点排序与投影点判定

首先将三个顶点按 y 坐标从大到小排序：

```c
// ── 源文件: core/gfx/ipgui_draw_triangle.c ──
__IPGUI_STATIC__ void ipgui_sort3p_by_y(
    ipgui_point_t  * p1,
    ipgui_point_t  * p2,
    ipgui_point_t  * p3,
    ipgui_point_t ** sort_arr) // sort_arr[0] 的 y 最小，sort_arr[2] 的 y 最大
{
    // 三路比较判断：将 y 最小的放在 sort_arr[0]
    // sort_arr[1] 和 sort_arr[2] 分别是 y 居中和最大的
}
```

排序后，`pa[0]` 是 y 最小的顶点（三角形的"顶部"），`pa[2]` 是 y 最大的顶点（三角形的"底部"），`pa[1]` 是中间顶点。

接下来用"长边"（`pa[0] → pa[2]`）作为参考，通过投影点判定法确定左右关系：

```c
ipgui_edge_coord_t x_at_m = edge_x_at_y(&e_long, pa[1]->y);
if ((pa[1]->x * 64) < x_at_m) {
    e_long_dir  = EDGE_HALFPLANE_DIR_LEFT;
    e_short_dir = EDGE_HALFPLANE_DIR_RIGHT;
} else {
    e_long_dir  = EDGE_HALFPLANE_DIR_RIGHT;
    e_short_dir = EDGE_HALFPLANE_DIR_LEFT;
}
```

这个判定的意义是：如果 `pa[1]` 在长边的左侧（即 `pa[1]->x < 长边在 y=pa[1]->y 处的交点 x`），则长边是三角形的右边界（保留左侧），短边是左边界（保留右侧）。反之亦然。

### 1.8.3 下三角形光栅化（扫描线区域 `pa[0].y → pa[1].y`）

下三角形由长边（`pa[0] → pa[2]`）和短边（`pa[0] → pa[1]`）围成。光栅化时：

1. 在每条扫描线 y 上生成两条边缘遮罩的描述器——一条对应短边，一条对应长边
2. 将两个半平面遮罩的值相乘（每个像素的最终遮罩值 = 短边遮罩 × 长边遮罩 / 255），得到该像素在三角形内的覆盖度
3. 使用 `fill_mask_two_edges()` 函数高效填充一行——跳过中间的全覆盖区域（255 区域）

### 1.8.4 上三角形光栅化（扫描线区域 `pa[1].y → pa[2].y`）

上三角形的处理与下三角形对称：短边从 `pa[1] → pa[2]`，长边仍然是 `pa[0] → pa[2]`。其他逻辑相同。

### 1.8.5 中间行独立处理

`pa[1].y` 所在的扫描线是一个特殊情况——该扫描线同时属于下三角形和上三角形的边界。这行需要三条边（两条短边 + 一条长边）同时参与：

```c
// 三条边的遮罩值相乘，取交集
m1 = ipgui_edge_halfplane_mask(&em1, x);  // 短边1
m2 = ipgui_edge_halfplane_mask(&em2, x);  // 长边
m3 = ipgui_edge_halfplane_mask(&em3, x);  // 短边2
mask_buf[j] = ((u32_t)m1 * m2 * m3 + 65535) >> 16;
```

三个 0-255 值的乘积除以 255² 得到 0-255 的结果。`+65535`（即 `+255² - 1`）实现四舍五入。

### 1.8.6 fill_mask_two_edges 优化

`fill_mask_two_edges()` 是对三角形光栅化中逐像素双遮罩计算的优化：

```c
// ── 源文件: core/gfx/ipgui_draw_triangle.c ──
// 将一行划分为三段：
// 1. 左侧抗锯齿区：从右边缘左侧渐变区边界向左到左边缘左侧渐变区边界
// 2. 中间实心区：两个边缘的 mask 都是 255，直接填充 255
// 3. 右侧抗锯齿区：从左边缘右侧渐变区边界向右
```

对于每个部分：只在需要渐变的地方逐像素计算半平面遮罩值，中间实心部分用 `ipgui_memset(mask, 255, len)` 一次性填充。这比全行逐像素计算的版本减少了约 60%-80% 的 `ipgui_edge_halfplane_mask()` 调用。

### 1.8.7 遮罩缓冲区管理

三角形光栅化需要一块临时遮罩缓冲区。缓冲区的申请使用 `ipgui_mask_buf_acquire(w, h, &res_h)`：

```c
// ── 源文件: core/gfx/ipgui_mask_buf.c ──
u8_t * ipgui_mask_buf_acquire(ipgui_coord_t w, ipgui_coord_t h,
                                ipgui_coord_t * res_h)
{
    while (h > 0) {
        p = (u8_t *)ipgui_mem_alloc_def(w * h);
        if (p) break;    // 分配成功
        h --;            // 分配失败：降低一行后重试
    }
    *res_h = h;
    return p;
}
```

降级分配策略是关键设计：当请求的 `w × h` 尺寸无法分配时（内存池剩余不足），不是直接返回失败，而是逐行降低 `h` 后再次尝试。每次降低一行，直到分配成功或 h 降为零。

这意味着大三角形的光栅化会自动拆分成多个批次——每次只处理 `res_h` 行的遮罩，光栅化一批后立即将其混合（`ipgui_blend()`）到目标表面，然后继续下一批。这种设计使得系统在内存极度紧张的条件下仍能正确渲染任意大小的三角形，不会因为单次内存申请失败而导致渲染中断。

`ipgui_image_buf_acquire()` 使用完全相同的降级策略，用于图像缓冲区的申请。

---

## 1.9 Box 背景：圆角矩形的分解策略

### 1.9.1 问题定义

Box 背景绘制的目标是：在给定的 `padding_box` 内，以四个圆角半径 `(r_lt, r_rt, r_lb, r_rb)` 裁剪出一个圆角矩形，并用指定的涂料（纯色/渐变/图像）填充。

引擎的解决策略是**分治法**：将圆角矩形分解为若干个不含圆角的矩形条和四个独立的圆角区域，分别绘制。

### 1.9.2 get_max_radius：圆角半径规整

```c
// ── 源文件: core/gfx/ipgui_draw_box_background.c ──
void get_max_radius(ipgui_aabb_t * padding_box, ipgui_box_style_t * style,
    ipgui_coord_t * r_lt, ipgui_coord_t * r_rt,
    ipgui_coord_t * r_lb, ipgui_coord_t * r_rb)
{
    ipgui_coord_t rmax_w = ipgui_aabb_width (padding_box) >> 1;
    ipgui_coord_t rmax_h = ipgui_aabb_height(padding_box) >> 1;
    ipgui_coord_t rmax   = IPGUI_MIN(rmax_w, rmax_h);

    *r_lt = IPGUI_MIN(rmax, style ? style->left_top_radius     : 0);
    *r_rt = IPGUI_MIN(rmax, style ? style->right_top_radius    : 0);
    *r_lb = IPGUI_MIN(rmax, style ? style->left_bottom_radius  : 0);
    *r_rb = IPGUI_MIN(rmax, style ? style->right_bottom_radius : 0);
}
```

`rmax` 被限制为 padding_box 的半宽和半高的较小值。这确保圆角半径不会超过矩形自身尺寸的一半——当用户设置的半径大于矩形最大允许值时，自动钳位到允许的最大值，不产生渲染错误或溢出。

### 1.9.3 五部分矩形条分解

引擎将不含圆角的矩形区域分解为五个矩形条（part 1-5），顺序为：

```
┌────────────────────────────────┐
│ part2 (左上方块)   part1 (中央贯穿竖条)   part4 (右上方块)  │
│          l_min                l_max~r_max                r_min   │
│                                  │                          │
│ part3 (左下方块, l_max > l_min)  │  part5 (右下方块, r_max > r_min)  │
└────────────────────────────────┘
```

**Part 1（中央贯穿竖条）**：起点 x = `padding_box.start.x + l_max`，终点 x = `padding_box.end.x - r_max`，y 从 padding_box 的顶部到底部。这个竖条占用了矩形最宽的部分——它避开了左右两侧所有圆角列。

**Part 2（左侧上竖条）**：仅在 `l_min > 0` 时存在。x 范围从 `padding_box.start.x` 到 `padding_box.start.x + l_min - 1`（即左侧两圆角中较小者的宽度），y 从 `padding_box.start.y + r_lt` 到 `padding_box.end.y - r_lb`（避开左上和左下圆角）。

**Part 3（左侧下竖条）**：仅在 `l_max > l_min`（左上和左下圆角半径不同）时存在。根据哪个圆角更大来判断哪些区域不需要避开。x 范围从 `padding_box.start.x + l_min` 到 `padding_box.start.x + l_max - 1`。

**Part 4 和 Part 5**：与左侧对称，处理右侧的上下方块。

这五个矩形条共同覆盖了圆角矩形中"不需要圆角处理"的区域——它们与圆角无交集，因此可以直接用 `ipgui_blend()` 做矩形填充，无需遮罩。

### 1.9.4 四个圆角区域

对于每个 `radius > 0` 的圆角，引擎生成一个 `corner` 区域并调用 `draw_one_corner()`：

```c
// 左上角（x_step=-1 表示这是圆的左半部分，y_flip=1 表示上半部分）
corner.start.x = padding_box.start.x;
corner.end.x   = padding_box.start.x + r_lt - 1;
corner.start.y = padding_box.start.y;
corner.end.y   = padding_box.start.y + r_lt - 1;
draw_one_corner(surf, clip, &cdraw, &corner, bg_style, r_lt, 0, -1, 1);
```

`draw_one_corner()` 的核心步骤：

1. 申请遮罩缓冲区（`ipgui_mask_buf_acquire`）
2. 逐行调用 `ipgui_fetch_ring_mask(circle_x_start, circle_y, 0, er, x_step, row_mask, w)`
3. 每 `res_h` 行做一次 `ipgui_blend()`
4. 释放遮罩缓冲区

`circle_x_start` 和 `circle_y` 的计算将屏幕坐标转换为以圆心为中心的局部坐标系：

```c
// x_step == -1（左半区）: cdraw->start.x 离圆心最远，逻辑 x 最大
circle_x_start = (corner->end.x + 1 - cdraw->start.x);
// y_flip == 1（上半区）: 越往下离圆心越近，逻辑 y 越小
circle_y = (corner->end.y + 1 - draw_y);
```

---

## 1.10 Box 边框

### 1.10.1 四条边的矩形条填充

边框绘制首先填充四条不含圆角区域的边：

- **上边**：从 `padding_box.start.x + r_lt` 到 `padding_box.end.x - r_rt`，y 在 border 厚度范围内
- **下边**：同上，y 在 border 底部范围内
- **左边**：从 `padding_box.start.y + r_lt` 到 `padding_box.end.y - r_lb`，x 在 border 左侧范围内
- **右边**：同上，x 在 border 右侧范围内

这四段都是纯粹的矩形——与圆角无交集，无需遮罩。

### 1.10.2 半径为零时的缺角补充

当某个圆角半径为零时，四条边的矩形不会自然延伸到角位置。例如，`r_lt == 0` 时，上边和左边在左上角的位置留下了一个空白缺口。引擎通过四个条件判断补充这些缺口：

```c
if (r_lt == 0) {
    fill.start.x = padding_box.start.x - bw;
    fill.end.x   = padding_box.start.x - 1;
    fill.start.y = padding_box.start.y - bw;
    fill.end.y   = padding_box.start.y - 1;
    draw_border_rect(fill);
}
```

### 1.10.3 圆角边框的环形遮罩

对于 `radius > 0` 的圆角，边框区域可以通过外径 `er = radius + border_width` 和内径 `ir = radius` 描述为一个环形。`draw_one_corner()` 使用与背景绘制相同的逻辑，但传递的是 `(ir, er)` 而非 `(0, er)`：

```c
ipgui_fetch_ring_mask(circle_x_start, circle_y, ir, er, x_step, row_mask, w);
```

当 `ir > 0` 时，环形遮罩返回的是"环形区域"（`ir < distance < er`）而非"实心圆区域"（`distance < er`）——相当于在圆的内部切掉了一个 `ir` 半径的圆形空洞。

---

## 1.11 Box 阴影：SDF + 1D 多项式模糊

### 1.11.1 问题建模

Box 阴影的视觉需求是：在按钮的周围产生一个从按钮边缘向外逐渐衰减的暗色半透明区域。从信号处理的角度，这等效于对按钮形状的二值指示函数做高斯模糊，然后将模糊结果偏移和着色。

高斯模糊的计算量在嵌入式 MCU 上不可行——需要为每个像素做多个周边像素的加权求和。引擎采用的替代方案是分三步：

1. 计算每个像素到按钮边缘的有符号距离（SDF）
2. 用廉价的多项式函数将距离值映射为 alpha 值（CDF 近似）
3. 将结果 LRU 缓存，避免重复计算

### 1.11.2 三种模糊 Profile 多项式

系统提供了三种用于近似高斯累积分布函数（CDF）的多项式，均为 Q8 定点数：

| Profile | 多项式 | 特性 |
|---------|--------|------|
| smoothstep3 | `3t² - 2t³` | 默认选择。C¹ 连续（导数在端点处连续），质量与速度的最佳平衡 |
| smoothstep5 | `6t⁵ - 15t⁴ + 10t³` | C² 连续（二阶导数也在端点处为零），过渡最平滑 |
| quadratic | `1 - (1-t)²` | C⁰ 连续，计算量最小（仅有两次乘法），适合极低性能设备 |

三者的工程取舍：

- smoothstep5 最接近真实高斯 CDF，但需要在 48MHz MCU 上计算 5 次多项式乘法
- smoothstep3 视觉上几乎无法与 smoothstep5 区分，但乘法次数减半（2 次 vs 5 次）
- quadratic 只有两次乘法，但过渡的视觉平滑度低于前两者

默认推荐 smoothstep3。只有在极低性能设备（< 20MHz）或阴影面积占总屏幕超过 50% 时才考虑 quadratic。

### 1.11.3 LRU Profile 缓存

每个 blur profile 的计算结果（一个 `u8_t[256]` 的查表）被 LRU 缓存。缓存的 key 为 `(blur_radius, profile_type, half_blur)` 三元组。

因为同一界面中所有控件的阴影通常使用相同的模糊半径和 profile 类型，缓存命中率极高。每个控件的渲染只需要一次查表操作（LRU 缓存查找）而非一次多项式计算（包含 3-5 次乘法）。

---

## 1.12 遮罩渐变系统

### 1.12.1 问题定义

遮罩渐变是"将遮罩值也做成渐变"的系统。它与颜色渐变（见 1.13 节）的区别在于：颜色渐变控制的是"用什么颜色绘制"，遮罩渐变控制的是"在哪里绘制"。遮罩渐变的值本身是一个 0-255 的透明度参数，经过遮罩渐变后，原本的纯色颜色会被不同程度的透明度遮挡。

遮罩渐变支持两种类型：**线性遮罩渐变**和**径向遮罩渐变**。

### 1.12.2 数据结构

```c
// ── 源文件: core/gfx/ipgui_mask_gradient.h ──
typedef struct {
    unsigned char pos;      // 停止点在渐变中的位置 (0-255)
    unsigned char value;    // 遮罩值 (0-255)
} ipgui_mask_gradient_stop_t;

typedef struct {
    // 比例模式参数 (×255.0f)
    int x_start, y_start, x_end, y_end;
    unsigned int opacity : 16;
    unsigned int stop_nr : 16;
    ipgui_mask_gradient_stop_t stops[IPGUI_GRADIENT_STOP_MAX];

    // 应用 AABB 后计算的绝对坐标
    ipgui_aabb_t aabb;
    ipgui_coord_t x_start_abs, y_start_abs;
    ipgui_coord_t x_end_abs, y_end_abs;
    ipgui_vector_t gradient_vector;
    int gradient_vec_mod_pow;  // 渐变向量模的平方
} ipgui_liner_mask_gradient_t;
```

线性遮罩渐变的工作流：

1. 通过 `ipgui_liner_mask_gradient_init()` 或 `ipgui_liner_mask_gradient_init_direct()` 定义渐变的起止点
2. 通过 `ipgui_liner_mask_gradient_add_stop()` 添加停止点（保持按 pos 排序）
3. 通过 `ipgui_liner_mask_gradient_apply_to_aabb()` 将比例坐标转换为绝对坐标
4. 逐像素调用 `ipgui_liner_mask_gradient_value_at_xy()` 获取遮罩值

### 1.12.3 位置计算（投影法）

```c
// ── 源文件: core/gfx/ipgui_mask_gradient.c ──
unsigned char ipgui_get_liner_mask_gradient_pos_at_xy(
    ipgui_liner_mask_gradient_t * gradient,
    ipgui_coord_t x, ipgui_coord_t y)
{
    ipgui_vector_t start2p;
    start2p.x = x - gradient->x_start_abs;
    start2p.y = y - gradient->y_start_abs;

    int v_dot = start2p.x * gradient->gradient_vector.x +
                start2p.y * gradient->gradient_vector.y;

    int proj_int = v_dot / gradient->gradient_vec_mod_pow;
    int proj_frac = v_dot % gradient->gradient_vec_mod_pow;

    if (proj_int < 0 || proj_frac < 0) return 0;
    else if (proj_int >= 1) return 255;
    else return (unsigned char)(proj_frac * 255 / gradient->gradient_vec_mod_pow);
}
```

这个函数的数学本质是：将像素 `(x, y)` 投影到渐变方向向量上，计算投影位置在 [0, 1] 区间的比例（转换为 0-255 的整数）。

`v_dot` 是起点到像素的向量与渐变向量的点积。`v_dot / gradient_vec_mod_pow` 得到投影位置的比例（0 表示位于起点，1 表示位于终点）。`gradient_vec_mod_pow` 即 `|gradient_vector|²`，用平方而非模长避免了开方运算。

如果像素在渐变起点之前（`proj_int < 0`），返回位置 0；如果像素在渐变终点之后（`proj_int >= 1`），返回位置 255。

### 1.12.4 遮罩值插值

```c
unsigned char ipgui_liner_mask_gradient_value_get(
    ipgui_liner_mask_gradient_t * gradient, unsigned char pos)
{
    // 在 stops[] 中按 pos 做区间查找
    // 找到满足 stops[idx-1].pos <= pos <= stops[idx].pos 的相邻停止点对
    // 线性插值：result = lerp(val1, val2, (pos - pos1) * 255 / (pos2 - pos1))
}
```

### 1.12.5 遮罩插值查找表

引擎提供了一个可选的 256×256 LUT 来加速两个遮罩值的加权混合：

```c
#if IPGUI_MASK_GRADIENT_LUT_EN == 1
static unsigned char mask_blend_table[256][256] = {0};

void ipgui_mask_gradient_lut_init(void)
{
    for (int dist = 0; dist < 256; dist++)
        for (int value = 0; value < 256; value++)
            mask_blend_table[dist][value] = (dist * value + 127) / 255;
}
#endif
```

当 LUT 启用时，遮罩值插值退化为两次查表：`mask_blend_table[dist][val2] + mask_blend_table[idist][val1]`。这比运行时做整数乘法和除法快数倍，代价是 64KB 的 ROM 空间（256×256×1 字节）。

### 1.12.6 径向遮罩渐变

径向渐变的位置计算基于像素到渐变中心的距离平方：

```c
unsigned char ipgui_get_radial_mask_gradient_pos_at_xy(
    ipgui_radial_mask_gradient_t * gradient,
    ipgui_coord_t x, ipgui_coord_t y)
{
    int dx = x - gradient->center_x;
    int dy = y - gradient->center_y;
    int dp = dx * dx + dy * dy;

    if (dp >= gradient->radius_pow) return 255;

    // 通过平方根查找将距离映射到 0-255
    unsigned int pos_sq = (dp * 255U / gradient->radius)
                        * 255U / gradient->radius;
    unsigned char pos = 0;
    while (pos < 255) {
        if (((pos + 1) * (pos + 1)) > pos_sq) break;
        pos++;
    }
    return pos;
}
```

这里使用了一个朴素的逐位搜索来求平方根——`while (pos < 255)` 循环。由于 `pos` 的范围仅 0-255，这个循环最多执行 255 次，对每个像素来说仍然是常数时间。但 `radius_pow`（`radius × radius`）在初始化时预计算，避免了在线乘法。

---

## 1.13 像素绘制

像素绘制是图形系统中粒度最小的操作——在指定坐标绘制一个单像素点。虽然简单，但它体现了从 mask/opacity 组合到 premultiplied alpha 混合的完整微缩链路。

```c
// ── 源文件: core/gfx/ipgui_draw_pixel.c ──
void ipgui_draw_pixel(ipgui_surf_t * surf, ipgui_aabb_t * clip,
    ipgui_coord_t x, ipgui_coord_t y, ipgui_color_t color,
    u8_t mask, u8_t opacity, ipgui_blend_mode_t blend_mode)
{
    // 1. 裁剪判定
    // 2. 合并 mask 和 opacity
    mask_opacity_combined = (u8_t)(((u32_t)opacity * mask + 127) >> 8);

    // 3. 预乘 alpha
    premult = ipgui_color_combine_opacity_and_premultiply(&color,
                                                          mask_opacity_combined);

    // 4. 定位目标像素地址
    dest_cr_buf = ipgui_surf_color_get(surf,
        x - surf->surf.start.x, y - surf->surf.start.y);

    // 5. 混合
    blend_fn(premult, dest_cr_buf, blend_mode);
}
```

这个短函数展示了从 gfx 层到 composite 层的完整数据变换：

- `mask`（来自 gfx 层，范围 0-255）和 `opacity`（来自用户代码，范围 0-255）合并为一个综合透明度
- `ipgui_color_combine_opacity_and_premultiply()` 一次性完成"透明度组合 + RGBA 预乘"
- `premult_blend_table[surf->pix_fmt]` 做 O(1) 表查找，获取目标像素格式对应的 blend 函数
- blend 函数执行实际的像素混合操作

---

## 1.14 渐变色彩系统

### 1.14.1 停止点插值模型

三种渐变（线性、径向、锥形）共享同一套停止点插值逻辑：

```
给定位置 pos (0-255):
1. 在 stops[] 中二分查找，找到相邻停止点对 (i, i+1)
   满足 stops[i].pos <= pos <= stops[i+1].pos
2. 计算插值比例 t = (pos - stops[i].pos) × 255 / (stops[i+1].pos - stops[i].pos)
3. 对 RGBA 四个通道分别线性插值：channel = stops[i].ch + t × (stops[i+1].ch - stops[i].ch) / 255
```

停止点数组始终保持按 `pos` 升序排列。`add_stop()` 插入时做有序插入（O(n) 移动），`remove_stop()` 同理。停止点最大数量由 `IPGUI_GRADIENT_STOP_MAX` 控制。

### 1.14.2 线性渐变的方向快速通道

```c
// ── 源文件: core/gfx/ipgui_gradient_color.h ──
static inline s32_t ipgui_if_liner_gradient_hor(
    ipgui_liner_gradient_color_t * gradient)
{
    return gradient->y_start_abs == gradient->y_end_abs;
}

static inline s32_t ipgui_if_liner_gradient_ver(
    ipgui_liner_gradient_color_t * gradient)
{
    return gradient->x_start_abs == gradient->x_end_abs;
}
```

当渐变方向为水平（y 起止点相同）时，每一列的像素颜色相同——每列只计算一次颜色，复用整列。当渐变方向为垂直（x 起止点相同）时，每行的像素颜色相同，每行只计算一次颜色。

### 1.14.3 径向渐变

径向渐变的位置计算基于像素到渐变中心的欧几里得距离。所有运算使用整数：

```
pos = sqrt((px - cx)² + (py - cy)²) / radius × 255
```

在实现中，`dx × dx + dy × dy` 与 `radius²` 比较，然后通过查表或线性搜索做平方根近似。

### 1.14.4 锥形渐变

锥形渐变需要求像素相对于渐变中心的极角（atan2），并将其映射到 [0, 255]。引擎在 `apply_to_aabb` 阶段预计算 `atan2` 的定点化查表结果，避免在线三角函数计算。

---

## 1.15 本章小结

本章从源码层面剖析了 ESDBox_IPGUI gfx 模块的十一个核心子系统和配套基础设施：

| 子系统 | 核心机制 | 关键优化 |
|--------|---------|---------|
| 子像素坐标系 | Q26.6 定点数，每像素 64 子像素单元 | 位运算替代乘除：`<<6`, `>>6`, `&63` |
| 半平面边缘遮罩 | 边缘参数化 + 平坦/陡峭分类 + 修正因子表 | 完全在保留侧的像素直接返回 255，无距离计算 |
| WDF 宽线遮罩 | 中心线交点 + 渐变区间 + 三种位置分支 | O(width) 而非 O(scanline_length) 的像素计算量 |
| 环形遮罩 | 圆角矩形 SDF + LRU 缓存 | 相同 (er, ir) 的请求命中缓存，跳过 SDF 计算 |
| 线段三路径 | 垂直/水平矩形填充 + 斜线 WDF + Wu 细线 | 垂直/水平退化到 O(1) 的矩形填充 |
| 弧/圆 | er/ir/angle 统一接口覆盖四种形状 | 复用 ring_mask + edge_clip，无重复代码 |
| 三角形光栅化 | 三点排序 + 投影点判定 + 三段式扫描线 | fill_mask_two_edges 跳过中间实心区域 |
| Box 背景 | 5 矩形条 + 4 圆角区域的分治分解 | 矩形条无需遮罩，圆角区域复用 ring mask 缓存 |
| Box 边框 | 4 边条 + 4 角环形遮罩 + 缺角补充 | 与背景共享 draw_one_corner 逻辑 |
| Box 阴影 | SDF + 1D 多项式 CDF 近似 | 三个 profile 选项，LRU 缓存避免重复多项式计算 |
| 遮罩渐变 | 投影法位置计算 + 停止点插值 + LUT 加速 | 64KB LUT 避免运行时乘除；gradient_vec_mod_pow 避免开方 |

---

### 思考问题

1. Q26.6 定点数采用 64 作为子像素缩放因子。如果改为 Q24.8（256 子像素/像素），修正因子表和 `dist_to_lower_64` 等函数需要如何修改？精度提升是否值得额外的代码修改量？

2. 修正因子表只有 127 个条目。如果斜率超过 45°（`|dx| < |dy|`），修正因子通过何种机制保证正确的垂直距离映射？请分析 `flatten` 分类在此过程中的作用。

3. WDF 宽线遮罩的三种位置关系分支中，"遇到 mask=255 即停止"的策略依赖什么前提条件？在什么边界情况下这个前提不成立？

4. 三角形光栅化中 `fill_mask_two_edges` 的中间实心区优化通过什么方式避免了逐像素的双遮罩乘法？为什么这种优化在三角形的长边接近垂直时效果最佳？

5. Box 背景的五部分矩形条分解中，Part 3 和 Part 5 仅在 `l_max > l_min` 的条件下存在。如果去掉这个条件判断会有什么后果？请分析一个具体场景（如 r_lt=8, r_lb=4, r_rt=6, r_rb=0）。

6. 遮罩梯度的 LUT（`mask_blend_table[256][256]`）占用 64KB ROM。如果在 ROM 只有 128KB 的设备上，你会选择保留 LUT 还是用运行时除法？请结合该设备上遮罩插值的调用频率做分析。

7. `ipgui_mask_buf_acquire` 的降级分配策略中，为什么采用 `h--`（每次降低一行）而非 `h = h >> 1`（减半）？两种策略在哪些场景下各自有优势？

---

**下一章预告**：第二章将深入 `core/composite` 模块，分析 Paint 分发器如何将 gfx 层输出的遮罩与涂料转化为最终像素颜色，重点讨论 premultiplied alpha 混合引擎的工程实现、RGB565 packed blend 的位级优化技巧、梯度填充的方向感知快速通道、图像合成的全像素格式交叉矩阵、脏矩形管理与屏幕渲染调度。