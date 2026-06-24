# 第一章 嵌入式绘图引擎核心架构与实现

> **模块位置**：`core/gfx/`
> **核心职责**：几何遮罩生成、子像素反走样、图元光栅化
> **对外接口**：遮罩数组（mask）+ 涂料描述（paint）→ 委托 composite 模块完成像素混合

---

## 1.1 架构概览

### 1.1.1 模块定位与职责边界

ESDBox_IPGUI 的渲染管线采用严格的分层架构。`core/gfx` 模块位于管线的中间层：它向上接收控件的绘制请求，向下将计算结果传递给 `core/composite` 模块。两层之间的接口由两样东西定义——**遮罩数组**（mask）与**涂料描述**（paint）。

gfx 模块不关心颜色。它的全部职责是处理几何问题：一条直线的斜率如何影响像素覆盖度、一个圆角矩形在任意象限的 SDF 如何计算、三角形三个顶点按 y 坐标排序后如何拆分为上下两个子三角形的光栅化范围、多边形的边表如何按扫描线递增维护。

composite 模块不关心形状。它接收一个 `u8_t` 数组（mask）和一个 `ipgui_paint_t` 结构体，然后逐像素地将颜色混入表面。它不知道这个 mask 来自直线还是圆角——它只知道"在这个像素上，遮罩值是 128"。

这种分工带来了关键的架构收益：

1. **新增图元不影响 composite**。今天给 gfx 增加五角星的绘制逻辑，composite 的所有混合代码无需修改。
2. **新增颜料类型不影响 gfx**。明天给 composite 增加锥形渐变，gfx 的所有图元绘制代码无需修改。
3. **两个模块可以独立测试与优化**。gfx 的优化方向是减少几何计算量；composite 的优化方向是减少颜色混合的算术运算次数。

以绘制一个蓝色圆角按钮为例，数据在两模块之间的流动如下：

```
用户代码: ipgui_draw_box_background(surf, clip, &abs, &shape, &bg_style)
    │
    ├── 步骤 1: 计算 padding_box（考虑 padding 的内缩矩形）
    ├── 步骤 2: 规整圆角半径 (get_max_radius)
    ├── 步骤 3: 将矩形分解为 5 个不含圆角的矩形条 + 4 个圆角区域
    ├── 步骤 4: 对每个圆角区域，调用环形遮罩模块 (ipgui_ring_mask)
    │
    └── 步骤 5: 对每个子区域调用 ipgui_blend(surf, clip, dest, paint, opacity, mask, ...)
              → composite 模块接管：遮罩 × paint 颜色 × 目标像素 = 最终结果
```

步骤 1-4 是纯粹的几何运算，完全不涉及颜色。步骤 5 是颜色运算，完全不涉及几何。这种清晰的边界是 ESDBox_IPGUI 管线设计的核心原则。

### 1.1.2 核心数据类型体系

gfx 模块定义了数种贯穿所有子模块的基础类型。理解这些类型是理解后续算法的前提。

```c
/* ── 源文件: core/gfx/ipgui_core.h ── */

/* 像素表面描述符 */
typedef struct {
    ipgui_aabb_t    surf;       /* 表面在屏幕上的全局坐标范围 */
    u8_t          * color;     /* 指向 surf 区域首像素的内存指针 */
    u32_t           stride;    /* 每行字节跨度（可大于 w×pix_size） */
    ipgui_pix_fmt_t pix_fmt;   /* 像素格式枚举 */
    u8_t            pix_size;  /* 每像素字节数 */
} ipgui_surf_t;

/* 轴对齐包围盒 */
typedef struct {
    ipgui_coord_t start_x;
    ipgui_coord_t start_y;
    ipgui_coord_t end_x;
    ipgui_coord_t end_y;
} ipgui_aabb_t;

/* 二维点 */
typedef struct {
    ipgui_coord_t x;
    ipgui_coord_t y;
} ipgui_point_t;
```

`ipgui_surf_t` 是整个绘制操作的"画布"描述符。关键设计点在于 `stride` 字段——它允许行字节跨度大于 `宽度 × 每像素字节数`。当屏幕驱动使用部分帧缓冲（PFB）时，分配的缓冲区宽度可能大于脏矩形宽度（以满足对齐要求），此时 stride 保证逐行寻址的准确性。

另一个重要细节是 `surf` 字段的类型为 `ipgui_aabb_t`，其中存储的是**屏幕全局坐标**而非局部相对坐标。这意味着对像素数组 `color` 的任何偏移寻址都必须先用 `x - surf.start.x` 将屏幕坐标转换为表面局部坐标——这是全模块反复出现的模式。

```c
/* ── 源文件: core/gfx/ipgui_color.h ── */

typedef union {
    struct { u8_t b, g, r, a; };  /* 小端序内存布局: [B][G][R][A] */
    u32_t v;                        /* 整体 32 位值 */
} ipgui_color_t;
```

`union` 设计是嵌入式图形编程中的一个经典技巧。它允许代码根据场景需求选择最优的访问方式：

- **结构化通道访问**：`color.r`、`color.g`、`color.b`、`color.a`——适合算法描述和单通道操作
- **批量 32 位操作**：`color.v`——适合全通道等值比较（`if (color.v == 0)` 检查是否全透明）和颜色拷贝赋值

这种设计在无需依赖结构体布局假设的前提下，既保留了代码可读性，又获得了位级优化的能力。注意注释中明确注明了 `struct { b, g, r, a }` 的成员顺序依赖于小端序——这是嵌入式平台上的默认字节序假设，移植到大端序平台时需调整成员顺序。

### 1.1.3 gfx 模块的完整文件清单

```
core/gfx/
├── ipgui_edge_halfplane_mask.c/h  ── 半平面边缘遮罩：反走样的基础单元
├── ipgui_edge_wdf_mask.c/h        ── WDF 宽线厚度遮罩：宽线段的覆盖度渐变
├── ipgui_ring_mask.c/h            ── 环形遮罩：圆角矩形 SDF + LRU 缓存
├── ipgui_mask_gradient.c/h        ── 遮罩渐变：遮罩值沿线性/径向渐变分布
├── ipgui_mask_buf.c/h             ── 遮罩缓冲区：临时遮罩内存的获取与释放
├── ipgui_image_buf.c/h            ── 图像缓冲区：图像像素内存的获取与释放
├── ipgui_image_mask.c/h           ── 图像遮罩：从图像数据生成遮罩值
├── ipgui_draw_line.c/h            ── 线段绘制：Wu 经典细线 + 宽线 + 线帽
├── ipgui_draw_arc.c/h             ── 弧/圆/扇形/圆环统一接口
├── ipgui_draw_triangle.c/h        ── 三角形光栅化：半平面遮罩 + 三点排序法
├── ipgui_draw_polygon.c/h         ── 多边形光栅化：扫描线 + AVL 活动边表
├── ipgui_draw_pixel.c/h           ── 像素绘制：单像素混合
├── ipgui_draw_box_background.c/h  ── Box 背景：圆角矩形分解策略
├── ipgui_draw_box_border.c/h      ── Box 边框：四条边 + 四个圆角
└── ipgui_draw_box_shadow.c/h      ── Box 阴影：SDF + 1D 多项式模糊
```

这十五个模块中，最底层的是半平面边缘遮罩（edge_halfplane_mask）——它是三角形、弧、多边形等所有"形状边界"遮罩的计算单元。环形遮罩（ring_mask）是圆角矩形场景的专业优化。WDF 宽线遮罩是线段加粗的专用处理模块。其余的 draw_* 模块将遮罩和 composite 混合组合成可调用的公开 API。

---

## 1.2 子像素坐标系统

### 1.2.1 离散像素网格的固有局限

计算机屏幕由离散的像素网格构成。当一条数学上的理想线段穿过屏幕时，只有极少数像素的中心恰好落在线段上——大多数像素的中心与线段之间有一个不等的垂直距离。如果仅用整数坐标（第几个像素）描述图形，每个像素只有"画"和"不画"两种状态，结果必然是阶梯状的锯齿。

反走样（anti-aliasing）的核心思路是将每个像素的"不透明度"与其到图形边界的距离关联：距离为零的像素完全不透明（强度 255），距离逐渐增加时强度逐渐降低（0~255），超过一个像素宽度的像素完全透明（强度 0）。

实现这一思路需要比整数像素更细粒度的坐标系统。

### 1.2.2 Q26.6 定点数格式

ESDBox_IPGUI 使用 32 位有符号整数作为子像素坐标系的基础类型，采用 Q26.6 定点数格式：

```c
/* ── 源文件: core/gfx/ipgui_edge_halfplane_mask.h:17 ── */
typedef s32_t ipgui_edge_coord_t; /* Q26.6: 高26位为整数部，低6位为小数部 */
```

Q26.6 格式将每个整数像素等分为 2^6 = 64 个子像素单元。选择 64 而非 256（Q24.8）或 16（Q28.4）的工程理由是：

1. **乘法效率**：乘以 64 等价于 `<< 6`，除以 64 等价于 `>> 6`，全部用位移操作实现，不依赖硬件乘法器或除法器。
2. **精度匹配**：64 级离散度在 8 位颜色深度（256 级透明度）下误差不超过 ±1 级——人眼无法分辨此差异。
3. **整数范围**：Q26.6 可表示的坐标范围为 ±33,554,431 像素，远超任何实际嵌入式屏幕的尺寸（通常 ≤ 1024×768）。

一个典型的 320×240 屏幕在子像素空间中的分辨率等价于 20480×15360。一条从 (2.3, 1.1) 到 (7.8, 5.6) 的线段在子像素空间中表示为从 `2×64+19=147` 到 `7×64+51=499` 的整数坐标。

### 1.2.3 坐标对齐工具函数

四个基础工具函数构成整数坐标与子像素坐标的转换桥梁：

```c
/* ── 源文件: core/gfx/ipgui_edge_halfplane_mask.c:107-119 ── */

/* 返回 c 的低 6 位——即 c 到其下方最近整数像素中心的子像素距离 */
ipgui_edge_coord_t dist_to_lower_64(ipgui_edge_coord_t c)
{
    return c & 63;  /* 取低 6 位，等价于 c % 64 */
}

/* 返回 c 到其上方最近整数像素中心的子像素距离 */
ipgui_edge_coord_t dist_to_upper_64(ipgui_edge_coord_t c)
{
    return (64 - (c & 63)) & 63;
    /*            ^^^^^^^^     ^^^^
     *       到上边界的距离   边界修正：c恰好对齐时 64 & 63 = 0 */
}

/* 向下对齐到 64 的倍数（等价于 (c/64)*64） */
ipgui_edge_coord_t align_down_64(ipgui_edge_coord_t c)
{
    return c & ~(ipgui_edge_coord_t)63;  /* 清零低 6 位 */
}

/* 向上对齐到 64 的倍数（等价于 ceil(c/64)*64） */
ipgui_edge_coord_t align_up_64(ipgui_edge_coord_t c)
{
    return (c + 63) & ~(ipgui_edge_coord_t)63;
}
```

`dist_to_upper_64` 中的 `& 63` 修正是一个值得专门讨论的边界处理细节。当 `c & 63 == 0`（即 c 恰好是 64 的整数倍）时，`64 - 0 = 64`。但一个恰好落在整数像素中心的点，它到"下一个像素中心"的距离应该是 0 而非 64。`64 & 63 = 0` 修正了这一边界。这一行代码用零分支消除了一个显式的 if-else，而分支预测失败在 Cortex-M 处理器上通常消耗 2-3 个周期——在被数百条扫描线重复调用的上下文中，这些周期会快速累积。

---

## 1.3 半平面边缘遮罩

### 1.3.1 边参化数据结构

半平面边缘遮罩模块是 gfx 层最底层的计算单元。三角形光栅化、弧的扇形裁剪、多边形的边表生成、宽线的覆盖度计算——所有这些高层功能最终都委托给此模块执行"已知一条有向线段，求某像素到线段的有符号距离"这一原子操作。

每条边缘被参数化为一个紧凑的结构体：

```c
/* ── 源文件: core/gfx/ipgui_edge_halfplane_mask.h:24-32 ── */
typedef struct {
    ipgui_edge_coord_t   x1;       /* 线段端点中 y 较小的端点 */
    ipgui_edge_coord_t   y1;
    ipgui_edge_coord_t   dx;       /* Δx（保留符号） */
    ipgui_edge_coord_t   dy;       /* Δy（恒为非负，因为 y1 已经选了较小的端点） */
    s32_t                delta_x;  /* 保留字段，实际未使用 */
    s32_t                delta_y;  /* 平坦边垂直步进因子（dy/dx 的 Q16.16 表示） */
    u8_t                 correction_frac_index : 7;  /* 修正因子表索引（0~126） */
    u8_t                 flatten : 1;               /* 0=陡峭边, 1=平坦边（|dx| >= dy） */
} ipgui_edge_param_t;
```

两个关键设计细节：

1. **`flatten` 位域**（1 比特）：将边分为"平坦边"（`|dx| >= dy`）和"陡峭边"（`|dx| < dy`）。分类的意义在于距离计算方式不同——平坦边沿水平方向扫描时，相邻像素的水平增量需要乘以斜率（dy/dx）才能转换为垂直距离；陡峭边沿垂直方向扫描时，水平增量直接就是垂直距离。1 比特的存储选择反映了嵌入式开发中"抠门到极致"的内存哲学。

2. **`correction_frac_index` 位域**（7 比特）：存储修正因子在全局查找表中的索引。当边不是精确 45° 时，按扫描方向算出的"轴向距离"与"真正的垂直距离"之间存在一个固定的比例偏差。修正因子表预先计算了 127 个离散斜率对应这个偏差，避免了运行时的浮点三角函数计算。

### 1.3.2 边缘参数初始化

初始化函数的核心任务是：保证 `y1` 是 y 较小的端点（使 `dy` 非负）、根据斜率分类、预计算 delta_y（平坦边的垂直步进因子）和修正因子索引。

```c
/* ── 源文件: core/gfx/ipgui_edge_halfplane_mask.c:41-85 ── */
ipgui_edge_param_t ipgui_edge_param_init(
    ipgui_edge_coord_t x1, ipgui_edge_coord_t y1,
    ipgui_edge_coord_t x2, ipgui_edge_coord_t y2)
{
    ipgui_edge_param_t param;
    ipgui_memset(&param, 0, sizeof(ipgui_edge_param_t));

    /* 步骤 1: 确保 y1 ≤ y2，保证 dy 非负 */
    if (y2 > y1) {
        param.dy = y2 - y1;
        param.dx = x2 - x1;
        param.x1 = x1;
        param.y1 = y1;
    } else {
        param.dy = y1 - y2;
        param.dx = x1 - x2;
        param.x1 = x2;
        param.y1 = y2;
    }

    /* 步骤 2: 斜率分类 */
    if (IPGUI_ABS(param.dx) >= param.dy) {
        param.flatten = 1;  /* 平坦边 */

        /* 预计算 delta_y = dy/dx（Q16.16 格式） */
        s64_t scaled_dy = (s64_t)param.dy << 16;
        s32_t half_dx   = param.dx / 2;

        if (param.dx > 0) {
            param.delta_y = (scaled_dy + half_dx) / IPGUI_ABS(param.dx);
        } else if (param.dx < 0) {
            param.delta_y = (scaled_dy - half_dx) / IPGUI_ABS(param.dx);
        }

        /* 计算修正因子索引：dy/dx 的归一化映射 */
        s32_t abs_dx = IPGUI_ABS(param.dx);
        s32_t abs_dy = param.dy;
        param.correction_frac_index = (u8_t)(
            (abs_dy * 126 + (abs_dx >> 1)) / abs_dx);
    } else {
        param.flatten = 0;  /* 陡峭边 */

        s32_t abs_dx = IPGUI_ABS(param.dx);
        s32_t abs_dy = param.dy;
        param.correction_frac_index = (u8_t)(
            (abs_dx * 126 + (abs_dy >> 1)) / abs_dy);
    }

    return param;
}
```

关于 `delta_y` 的计算需要特别说明。`scaled_dy = (s64_t)param.dy << 16` 将 dy 放缩到 Q16.16 格式（低 16 位为小数部）。`+ half_dx` 实现向最近整数的舍入（而非向下取整），这是 `(scaled_dy + half_dx) / abs_dx` 的等效形式。注意根据 `dx` 的符号分别处理 `+ half_dx` 和 `- half_dx`——这是含符号分母整数除法的舍入方向修正。

关于 `(abs_dy * 126 + (abs_dx >> 1)) / abs_dx`：126 是修正因子表的容量上限（127 项，索引 0~126）。当斜率为 1（精确 45°）时，`abs_dy / abs_dx = 1`，计算得 `(126 + 0) = 126`，即取索引 126。当斜率接近 0（近乎水平）时，`abs_dy / abs_dx ≈ 0`，索引接近 0。这种线性映射基于一个工程事实：修正因子在不同斜率下近似线性变化。

### 1.3.3 扫描线描述符

对于给定的扫描线 y，边缘在该行上的遮罩生成信息被打包为描述符：

```c
/* ── 源文件: core/gfx/ipgui_edge_halfplane_mask.h:35-40 ── */
typedef struct {
    edge_halfplane_dir_t dir;       /* LEFT 或 RIGHT，决定遮罩在半平面的哪一侧生效 */
    ipgui_coord_t        y;         /* 当前扫描线的 y 坐标 */
    ipgui_coord_t        x_start;   /* 第一个需要生成遮罩的像素的 x 坐标（整数像素） */
    ipgui_edge_coord_t   frac_x;    /* x_start 像素到边界的子像素距离 */
    ipgui_edge_param_t * p;         /* 指向边缘参数结构体的指针 */
} ipgui_edge_halfplane_mask_dsc_t;
```

生成描述符的函数 `ipgui_gen_edge_halfplane_mask_dsc` 针对给定的扫描线 y 计算出边界穿过的精确 x 坐标（子像素精度），然后确定"从哪个整数像素开始需要遮罩"：

```c
/* ── 源文件: core/gfx/ipgui_edge_halfplane_mask.c:123-136 ── */
void ipgui_gen_edge_halfplane_mask_dsc(
    ipgui_edge_halfplane_mask_dsc_t * res,
    edge_halfplane_dir_t    dir,
    ipgui_edge_param_t    * p,
    ipgui_coord_t           y)
{
    ipgui_edge_coord_t x_sub = edge_x_at_y(p, y);  /* 已知 y，求边界 x 坐标（子像素精度） */

    res->dir = dir;
    res->p   = p;
    res->y   = y;

    if (dir == EDGE_HALFPLANE_DIR_LEFT) {
        /* 左半平面：从 x_sub 向上对齐到下一个整数像素开始遮罩 */
        res->x_start = align_up_64(x_sub) >> 6;  /* 除以 64 转换为整数像素坐标 */
        res->frac_x  = dist_to_upper_64(x_sub);
    } else {
        /* 右半平面：从 x_sub 向下对齐到当前整数像素开始遮罩 */
        res->x_start = align_down_64(x_sub) >> 6;
        res->frac_x  = dist_to_lower_64(x_sub);
    }
}
```

`edge_x_at_y` 函数通过斜率关系计算给定 y 处的边界 x 坐标：

```c
/* ── 源文件: core/gfx/ipgui_edge_halfplane_mask.c:7-14 ── */
ipgui_edge_coord_t edge_x_at_y(ipgui_edge_param_t * p, ipgui_coord_t y)
{
    if (p->dy == 0) return 0;  /* 水平线的边界处理（实际由上层保证不使用此函数） */

    s64_t temp;
    ipgui_edge_coord_t dy = (y * 64) - p->y1;  /* 子像素 y 偏移 */
    temp = (s64_t)dy * p->dx;
    return temp / p->dy + p->x1;
}
```

关键点：`(y * 64)` 将整数像素 y 转换为子像素 y 坐标，保证与 `p->y1`（子像素坐标）的单位一致。`(s64_t)dy * p->dx` 使用 64 位中间量防止溢出（dy 和 dx 都是子像素坐标，乘积可能超出 32 位范围）。`temp / p->dy` 是带舍入方向的整数除法（向零舍入）。

### 1.3.4 逐像素遮罩查询

`ipgui_edge_halfplane_mask` 是最终的遮罩查询入口：给定描述符和像素的 x 坐标，返回 0-255 的遮罩值：

```c
/* ── 源文件: core/gfx/ipgui_edge_halfplane_mask.c:143-162 ── */
u8_t ipgui_edge_halfplane_mask(
    ipgui_edge_halfplane_mask_dsc_t * dsc,
    ipgui_edge_coord_t      x)
{
    ipgui_edge_coord_t d;
    ipgui_edge_coord_t delta_x;

    /* 确定方向并计算 delta_x */
    if (dsc->dir == EDGE_HALFPLANE_DIR_LEFT) {
        if (x < dsc->x_start) return 255;  /* 完全在左半平面内，完全不透明 */
        delta_x = ((x - dsc->x_start) << 6) + dsc->frac_x;
    } else {
        if (x > dsc->x_start) return 255;  /* 完全在右半平面内，完全不透明 */
        delta_x = ((dsc->x_start - x) << 6) + dsc->frac_x;
    }

    /* 平坦边：将水平距离转换为垂直距离 */
    if (dsc->p->flatten) {
        d = ((s64_t)delta_x * dsc->p->delta_y + 32768) >> 16;
    } else {
        d = delta_x;  /* 陡峭边：水平距离直接就是垂直距离 */
    }

    /* 应用修正因子并映射到 0~255 */
    correct_d_and_return_mask(d);
}
```

这里有两个核心数学操作需要详细解释。

**delta_x 的计算**：`((x - dsc->x_start) << 6) + dsc->frac_x`。`x` 和 `x_start` 是整数像素坐标，它们的差乘以 64 转换为子像素距离。加上 `frac_x`（x_start 到边界本身的子像素偏移），得到当前像素到边界的完整子像素距离。

**平坦边的距离转换**：`((s64_t)delta_x * dsc->p->delta_y + 32768) >> 16`。`delta_y` 是 dy/dx 的 Q16.16 表示。当扫描线沿水平方向前进一个像素宽度（64 个子像素）时，边界线在垂直方向上的位移是 `64 × dy/dx` 个子像素。`delta_x × delta_y` 就是当前像素的水平距离对应的垂直距离（仍为 Q16 精度）。`+ 32768`（即 0.5×65536）实现向最近整数舍入。`>> 16` 将结果还原为整数子像素距离。

### 1.3.5 修正因子宏

`correct_d_and_return_mask` 宏完成距离修正和遮罩值映射：

```c
#define correct_d_and_return_mask(distance)\
    distance = distance * correction_frac[dsc->p->correction_frac_index] >> 8;\
    if (!distance) return 255;\
    else if (distance > 64) return 0;\
    else return (64 - distance) << 2;
```

修正因子的分母为 256（通过 `>> 8` 实现）。取索引对应斜率下的真实垂直距离 = 轴向距离 × 修正因子 / 256。

距离映射规则：
- `distance == 0`：像素中心恰好在边界上 → 遮罩值 255（完全不透明）
- `distance > 64`：像素中心距离超过一个整像素宽度 → 遮罩值 0（完全透明）
- `0 < distance <= 64`：线性过渡 → 遮罩值 = `(64 - distance) × 4 = 256 - 4×distance`

`<< 2`（乘以 4）将 0~64 的范围线性映射到 0~255：当 distance=0 时 `64<<2=256`，distance=64 时 `0<<2=0`。注意当 distance=0 时已经在第一个分支返回了 255，因此 `(64 - distance) << 2` 不会输出 256（超出 u8 范围）。

### 1.3.6 修正因子表

```c
/* ── 源文件: core/gfx/ipgui_edge_halfplane_mask.c:20-27 ── */
const u8_t correction_frac[127] = {
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 254, 254,
    254, 254, 254, 253, 253, 253, 253, 252, 252, 252, 251, 251, 251, 250, 250,
    250, 249, 249, 248, 248, 247, 247, 247, 246, 246, 245, 245, 244, 244, 243,
    /* ... 共127个值 ... */
    194, 193, 192, 192, 191, 190, 189, 189, 188, 187, 186, 186, 185, 184, 183, 183
};
```

此表的数学含义：对于斜率为 `abs(dy)/abs(dx)` 的边，其"轴向扫描距离"与"真实垂直距离"的比例为 `sqrt(dx² + dy²) / |dx|`（勾股定理）。修正因子 = `256 × |dx| / sqrt(dx² + dy²)`。例如，45° 时 `|dx| = |dy|`，修正因子 = `256 / sqrt(2) ≈ 181`。表的前 14 个值全部为 255，表明当斜率 < 1/10 时，轴向距离与垂直距离的偏差小于 1/255——肉眼无法分辨，直接取 255 即可。

127 字节的查找表在 64KB RAM 的嵌入式系统中占 0.2% 的内存。这是一个"经过深思熟虑的奢侈"——它换来了运行时零浮点计算和零除法的反走样效果。

---

## 1.4 环形遮罩——圆角矩形的 SDF 引擎

### 1.4.1 设计动机

圆角矩形是 GUI 中最频繁出现的形状——按钮、输入框、卡片、面板，几乎全部基于圆角矩形。如果每个圆角矩形都在运行时用半平面遮罩或其他通用方法逐像素计算，性能将不可接受。

环形遮罩（ring mask）模块专门为圆角矩形场景做了极致的优化。它使用有符号距离场（SDF）预先计算每个像素到圆角边缘的距离，并将计算结果缓存在 LRU 链表中以复用。

### 1.4.2 圆角参数与初值估计

```c
/* ── 源文件: core/gfx/ipgui_ring_mask.c ── */
typedef struct {
    ipgui_coord_t r;        /* 圆角半径 */
    s32_t         r2;       /* r²（避免运行时乘法） */
    s32_t         rmax2;    /* (r+1)² —— 遮罩外边界 */
    s32_t         rmin2;    /* (r-1)² —— 遮罩内边界 */
    s32_t         inv_out;  /* 外过渡区反比系数 */
    s32_t         inv_in;   /* 内过渡区反比系数 */
} corner_param_t;

void out_corner_param_init(corner_param_t * p, ipgui_coord_t r)
{
    p->r       = r;
    p->r2      = r * r;
    p->rmax2   = (r + 1) * (r + 1);
    p->rmin2   = (r - 1) * (r - 1);
    p->inv_out = (0xffff) / (p->rmax2 - p->r2);
    p->inv_in  = (0xffff) / (p->r2 - p->rmin2);
}
```

遮罩的过渡区域限定在 `[r-1, r+1]` 范围内（±1 像素宽度）。在此范围内，距离通过 `inv_out` 和 `inv_in` 的倒数线性插值转换为 0~255 的遮罩值。此设计的本质是 **smoothstep 的线性近似**——在反走样所需的 1 像素过渡带内，线性插值在 8 位颜色深度下与 smoothstep 的视觉差异人眼无法分辨，而避免了运行时的三次多项式计算。

### 1.4.3 LRU 缓存机制

环形遮罩的计算结果被缓存为 `corner_mask_cache_item_t`：

```c
/* ── 源文件: core/gfx/ipgui_ring_mask.c ── */
typedef struct {
    list_head_t node;              /* LRU 链表节点（内核风格 list_head） */
    ipgui_coord_t r;               /* 圆角半径（缓存 key） */
    u32_t last_used_tick;          /* 最后访问时间戳 */
    u32_t refcnt;                  /* 引用计数 */
    ipgui_coord_t dig_mask_start_xy; /* 对角线起始坐标 */
    u16_t * mask_index_at_y;       /* 每行遮罩起始索引 */
    ipgui_coord_t * mask_start_x_at_y; /* 每行遮罩起始 x 坐标 */
    u8_t * mask;                   /* 遮罩数据 */
} corner_mask_cache_item_t;
```

缓存项在 LRU 链表上按 `last_used_tick` 排序。当需要淘汰时，选择 tick 最小的节点。典型命中率超过 95%，因为一个界面上通常只有 2-3 种不同的圆角半径。

`last_used_tick` 使用了一个全局递增的时间戳计数器 `g_corner_cache_tick`。当计数器接近溢出（`>= 0xffffff00`）时，调用 `normalize_corner_cache_tick()` 将所有节点的 tick 归一化，防止 32 位溢出导致 LRU 排序错乱。

### 1.4.4 八分之一圆生成算法

圆角遮罩的核心是生成第一象限八分之一圆（0° 到 45°）的遮罩数据。从笛卡尔坐标系的 (r-1, 1) 出发，沿圆弧逐像素计算距离：

```
从 x = r-1, y = 1 开始：
每一步决定：(x-1, y+1) 和 (x, y+1) 哪个更接近圆弧
选择距离平方更接近 r² 的候选点
同时计算当前点的距离值
```

该算法是 Bresenham 圆算法的变体，在计算几何路径的同时生成每像素的距离值并填入遮罩数组。每行的遮罩起始 x 坐标和索引被记录在 `mask_start_x_at_y` 和 `mask_index_at_y` 中，供运行时快速查找。

### 1.4.5 运行时查询

当绘制一个具体像素 (px, py) 时，环形遮罩通过查表快速获取遮罩值：

1. 根据像素所在的象限（左上/右上/左下/右下），将坐标映射到第一象限
2. 根据 y 坐标从 `mask_index_at_y` 获取该行的起始索引
3. 根据 x 坐标判断是"内区域"（遮罩=255）、"过渡带"（查 mask 数组）还是"外区域"（遮罩=0）

这种查表策略将 O(1) 的逐像素查询复杂度从 30+ 条指令（实时 SDF 计算）降到约 10 条指令（查表 + 边界判断）。

---

## 1.5 WDF 宽线遮罩——粗线的覆盖度计算

### 1.5.1 问题定义

半平面边缘遮罩处理的是"一条零宽度的直线如何反走样"的问题。当线条有宽度（比如 5 像素宽的线段）时，问题变为"一条有一定厚度的带子如何反走样"。

简单的做法是将宽线视为两条平行的细线，分别使用半平面遮罩处理。但这会在两端（线帽）产生不自然的垂直切断——因为两端没有圆弧过渡。

ESDBox_IPGUI 使用 **WDF（Width Distribution Function，宽度分布函数）** 解决这一问题。核心思路是：对于扫描线上的每个像素，累加它到线段的精确垂直距离，然后将距离映射为 0-255 的遮罩值。

### 1.5.2 WDF 参数结构

```c
/* ── 源文件: core/gfx/ipgui_edge_wdf_mask.c ── */
typedef struct {
    ipgui_coord_t a, b, c;     /* 直线一般式参数: ax + by + c = 0 */
    ipgui_coord_t x1, y1;      /* y 较小端点 */
    ipgui_coord_t dx, dy;      /* 线段的变化量 */
    ipgui_coord_t delta_y;     /* 平坦边垂直步进因子 */
    u8_t          correction_frac_index : 7;  /* 修正因子索引 */
    u8_t          flatten : 1;              /* 平坦/陡峭分类 */
} ipgui_edge_wdf_param_t;

typedef struct {
    ipgui_coord_t  x_half_span;    /* x 方向的半跨度 */
    ipgui_xstep_t  x_step;         /* 每行 x 步进值 */
    ipgui_xidx_t   x_idx;          /* 当前 x 索引 */
    ipgui_coord_t  half_width64;   /* 半线宽（子像素单位） */
    ipgui_edge_wdf_param_t * p;    /* 指向参数 */
} ipgui_edge_wdf_mask_dsc_t;
```

### 1.5.3 横向跨度计算

`ipgui_edge_wdf_x_halfspan` 函数通过"单点步进试探"的方式计算线段中轴在水平方向上的半跨度。从一个端点向右逐步试探，直到点到直线的距离超过半线宽。

距离公式使用直线一般式 `ax + by + c = 0` 的形式：`d = |ax₀ + by₀ + c| / sqrt(a² + b²)`。为避免 sqrt，比较时使用平方形式：`(ax₀ + by₀ + c)² >= half_w² × (a² + b²)`。其中 `a = y1 - y2`，`b = x2 - x1`，`c = x1·y2 - x2·y1`。

`half_w2` 的计算有两个补偿：
- `if (width & 1) half_w2 += 1`：宽度的奇偶补偿
- `half_w2 += 1`：边界问题补偿，防止光栅化平坦线时产生锯齿

这两个"魔法+1"是在实际测试中发现并修正的经验参数，源注释明确标注了其用途。

### 1.5.4 批量遮罩生成

`ipgui_edge_wdf_mask` 是主要的遮罩生成入口。对于给定的扫描线，它在线段中轴两侧生成一排遮罩值：

- 在中轴的正上方/正下方一定范围内：遮罩 = 255（完全不透明——线段内部）
- 从中轴到半线宽之间：遮罩从 255 线性衰减到 0（过渡带）
- 超过半线宽：遮罩 = 0（完全透明）

遮罩的修正同样使用了 `correction_frac` 表（与半平面遮罩共享）。对于平坦边，水平距离需要乘以 `delta_y` 转换为垂直距离后再进行修正。

---

## 1.6 线段绘制——Wu 算法与宽线

### 1.6.1 Wu 经典细线算法

线段绘制函数的入口是 `ipgui_draw_line_classic`，它实现了吴小林（Xiaolin Wu）发明的反走样细线算法。该算法的核心洞察是：对于线段穿过的每一列，同列的两个像素（上方和下方）各自获得与"线段到各自中心的距离成反比"的透明度。

算法的实现分为平坦线和陡峭线两个分支：

**平坦线分支**（`|dx| > |dy|`）：
1. 从左到右逐列扫描
2. 使用 Bresenham 误差累积器跟踪线段在 y 方向上的偏移
3. 对于每列的上下两个像素，分别计算透明度：
   - `cover = 255 - (|err| × 255 / dx_abs)`：下方像素的覆盖率
   - `255 - cover`：上方像素的覆盖率
4. 覆盖率乘以样式的全局透明度后，调用 composite 的 blend 函数写入最终颜色

**陡峭线分支**（`|dx| <= |dy|`）：
1. 从上到下逐行扫描
2. 对称地使用 x 方向的误差累积器
3. 对于每行的左右两个像素进行相同的透明度分配

关键的性能优化：连续列/行之间的误差更新是 `err += dy` 或 `err += dx`，仅需一次加法。当误差跨越边界值时，`if (IPGUI_ABS(err) >= dx_abs)` 触发 y 坐标步进。零除法的保证来自函数入口处对 `dx_abs > dy_abs` 的分类。

值得注意的是，Wu 算法本身通过 Bresenham 误差累积器隐式地考虑了子像素位置——`err` 的初始值根据裁剪边界的偏移做了修正（`err = dy * (draw.start.x - xs)`）。这确保了被裁剪裁断的线段也能获得正确的子像素起始位置。

### 1.6.2 线段样式的渐变支持

线段模块不仅支持纯色绘制，还内联支持了渐变颜色——线性渐变、径向渐变和锥形渐变。在逐像素循环中，代码根据 `style->paint.type` 的取值分派到对应的渐变颜色采样函数：

```c
if (style->paint.type == IPGUI_PAINT_COLOR)
    paint_cr = style->paint.src.color;
else if (style->paint.type == IPGUI_PAINT_GRADIENT) {
    if (style->paint.src.grad_src.grad_type == IPGUI_GRADIENT_TYPE_LINEAR)
        ipgui_liner_gradient_color_get(&grad, pos, &paint_cr);
    else if (style->paint.src.grad_src.grad_type == IPGUI_GRADIENT_TYPE_RADIAL)
        ipgui_radial_gradient_color_get(&grad, pos, &paint_cr);
    else if (style->paint.src.grad_src.grad_type == IPGUI_GRADIENT_TYPE_CONIC)
        ipgui_conic_gradient_color_get(&grad, pos, &paint_cr);
}
```

这种内联分派避免了间接函数调用的开销——在逐像素循环中，函数指针的间接跳转在 ARM Cortex-M 上至少消耗 2 个额外周期，而 switch-if-else 在编译器优化后通常可以内联展开。

---

## 1.7 三角形光栅化——半平面遮罩的组合应用

### 1.7.1 三点排序法

三角形光栅化模块 `ipgui_draw_triangle` 将三角形拆分为上下两个子三角形分别处理。算法的第一步是按 y 坐标将三个顶点排序：

```c
/* ── 源文件: core/gfx/ipgui_draw_triangle.c ── */
static void ipgui_sort3p_by_y(
    ipgui_point_t  * p1, ipgui_point_t  * p2, ipgui_point_t  * p3,
    ipgui_point_t ** sort_arr)
{
    /* 通过 3 次比较确定最小 y 的顶点作为 sort_arr[0] */
    /* 再通过 1 次比较确定 sort_arr[1] 和 sort_arr[2] */
}
```

排序后，`sort_arr[0]` 是 y 最小的顶点（顶部），`sort_arr[1]` 是中间顶点，`sort_arr[2]` 是 y 最大的顶点（底部）。这两个子三角形分别是：
- **下三角形**：`sort_arr[0] → sort_arr[1] → sort_arr[2]`（只覆盖到 sort_arr[1].y）
- **上三角形**：`sort_arr[1] → sort_arr[2] + sort_arr[0] → sort_arr[2]` 的重新表述

### 1.7.2 长边与短边的判定

三角形的"长边"定义为 y 跨度最大的边——即 `sort_arr[0] → sort_arr[2]`。在光栅化过程中，长边作为"固定的右边界"（或左边界），两个短边（`sort_arr[0] → sort_arr[1]` 和 `sort_arr[1] → sort_arr[2]`）交替作为扫描线的左边界。

长边方向的判定使用"投影点判定法"：

```c
ipgui_edge_coord_t x_at_m = edge_x_at_y(&e_long, pa[1]->y);
if ((pa[1]->x * 64) < x_at_m) {
    e_long_dir  = EDGE_HALFPLANE_DIR_LEFT;  /* 长边在右侧 */
    e_short_dir = EDGE_HALFPLANE_DIR_RIGHT; /* 短边在左侧 */
} else {
    e_long_dir  = EDGE_HALFPLANE_DIR_RIGHT; /* 长边在左侧 */
    e_short_dir = EDGE_HALFPLANE_DIR_LEFT;  /* 短边在右侧 */
}
```

在中间顶点 y 处，计算长边对应的 x 坐标 `x_at_m`。如果中间顶点的实际 x 坐标小于该值，则长边在中间顶点的右侧（即三角形的右侧边界），短边在左侧。反之亦然。这个判断只需一次 `edge_x_at_y` 调用（约 20 条指令）即确定整个三角形光栅化过程中左右半平面的角色分配。

### 1.7.3 双半平面遮罩的逐行填充

三角形的每行扫描线由两条半平面遮罩共同决定：

```c
/* ── 源文件: core/gfx/ipgui_draw_triangle.c:fill_mask_two_edges ── */
static void fill_mask_two_edges(
    u8_t * mask, ipgui_coord_t sx, ipgui_coord_t len,
    ipgui_edge_halfplane_mask_dsc_t * right,  /* 右侧边界 */
    ipgui_edge_halfplane_mask_dsc_t * left)    /* 左侧边界 */
{
    /* 步骤 1: 左侧抗锯齿区（从 left.x_start 向左过渡到 255） */
    /* 步骤 2: 完全填充区（右侧边界的完全内部 + 左侧边界的完全内部之间的区域） */
    /* 步骤 3: 右侧抗锯齿区（从 right.x_start 向右过渡到 0） */
}
```

这三个区域对应三角形的内部（完全填充）、左边缘过渡带和右边缘过渡带。`ipgui_memset(mask, 255, mid_mask_len)` 对完全填充区进行批量设置——这比逐像素循环快约 10 倍。

处理完一个子三角形后，遮罩数组通过 `ipgui_blend` 一次性将颜色写入表面。两个子三角形共用同一个遮罩缓冲区，避免重复分配。

---

## 1.8 多边形光栅化——扫描线 + AVL 活动边表

### 1.8.1 算法架构

多边形光栅化模块 `ipgui_draw_polygon` 实现了经典的扫描线填充算法。其核心数据结构包括：

- **边表（edge buckets）**：按边的起始 y 坐标分类的 AVL 树。每条边存储其起始/结束 y 坐标、当前 x 坐标（含子像素小数部）、dx/dy 斜率信息。
- **活动边表（active edges）**：当前扫描线"正在穿越"的所有边的链表（按当前 x 坐标排序）。
- **扫描线单元表（scanline cells）**：当前扫描线上所有"从一条边到另一条边"的跨度区间。

整体流程：

```
1. 初始化：分配固定大小的边内存池和单元内存池
2. 将所有多边形的边按 y_start 插入边桶 AVL 树
3. 从 y_min 到 y_max 逐行扫描：
   a. 将边桶中 y_start == 当前 y 的边移入活动边表
   b. 对活动边表按 x_cur 排序
   c. 生成扫描线单元（交替配对：覆盖/不覆盖）
   d. 填充当前行的所有单元区间到 mask 数组
   e. 更新活动边表中每条边的 x_cur（按斜率步进）
   f. 移除 y_end == 当前 y 的边
4. 清理并释放内存
```

### 1.8.2 内存池设计

多边形光栅化的一个关键挑战是：边的数量不可预测（三角形=3 条边，复杂多边形可能=20 条边）。在 64KB RAM 的嵌入式系统中，动态分配每条边和每个扫描线单元是不可接受的。

ESDBox_IPGUI 的解决方案是使用**固定容量的 membox**（内存块分配器）：

```c
/* ── 源文件: core/gfx/ipgui_draw_polygon.c ── */
#define EDGE_EMBED_NUM  10   /* 最多 10 条边 */
#define EDGE_MAX_SLOPE   5   /* 每条边在单条扫描线上最多贡献 5 个 cell */
#define CELL_EMBED_NUM  (EDGE_EMBED_NUM * EDGE_MAX_SLOPE)  /* 最多 50 个 cell */
```

`ipgui_membox_init` 将预分配的内存块切分为固定大小的槽。分配和释放均为 O(1)。当边数超过 `EDGE_EMBED_NUM` 或扫描线单元数超过 `CELL_EMBED_NUM` 时，分配失败并返回错误——由上层代码负责"降级渲染"或"分片处理"。

### 1.8.3 AVL 树的排序比较器

边桶使用 AVL 树（一种自平衡二叉搜索树）维护，这保证了 O(log n) 的插入和删除。比较器按 y_start 排序：

```c
static s8_t y_start_cmp(void * a, void * b)
{
    ipgui_edge_t * ea = (ipgui_edge_t *)a;
    ipgui_edge_t * eb = (ipgui_edge_t *)b;

    if (eb->y_start < ea->y_start) return -1;  /* eb 应排在 ea 前面 */
    else if (eb->y_start > ea->y_start) return 1;

    /* y_start 相同 → 按 y_end 排序（保持稳定） */
    if (eb->y_end < ea->y_end) return -1;
    else if (eb->y_end > ea->y_end) return 1;

    /* y_end 相同 → 按 x_cur.inte 排序 */
    if (eb->x_cur.inte < ea->x_cur.inte) return -1;
    else if (eb->x_cur.inte > ea->x_cur.inte) return 1;

    /* x_cur.inte 相同 → 按 x_cur.frac 排序 */
    if (eb->x_cur.frac < ea->x_cur.frac) return -1;
    else if (eb->x_cur.frac > ea->x_cur.frac) return 1;

    /* 最后退化到地址比较，保证严格弱序（strick weak ordering） */
    { uintptr_t va = (uintptr_t)ea; uintptr_t vb = (uintptr_t)eb;
      if (vb < va) return -1; else if (vb > va) return 1; }
    return 0;
}
```

注意返回值的方向：`eb` 与 `ea` 比较而非直觉的 `ea` 与 `eb` 比较。这是 AVL 树实现中排序方向的选择，等价于把 y_start 较小的边放在 AVL 树的"较早遍历位置"。

最后一个地址比较是为了处理"两条边在所有排序字段上完全相同"的极端情况。AVL 树要求比较器必须满足严格弱序——如果两条边返回 0（相等），AVL 会拒绝插入其中一条。地址比较保证即使所有字段相同，两条边也不会被判为相等。

### 1.8.4 活动边表的 x 步进

活动边表中每条边都有一个 `x_cur`（当前 x 坐标，含子像素分数部）。当扫描线下移一行时，边的 x 坐标需要按 `dx/dy` 步进：

```c
static void ipgui_x_step(ipgui_edge_t * edge, ipgui_egde_xstep_t * step)
{
    edge->x_cur.inte += step->inte;
    edge->x_cur.frac += step->frac;

    while (IPGUI_ABS(edge->x_cur.frac) >= edge->dy)
    {
        if (edge->dx > 0) {
            edge->x_cur.inte += IPGUI_PIXEL_PRECI;  /* +1 像素 */
            edge->x_cur.frac -= edge->dy;
        } else {
            edge->x_cur.inte -= IPGUI_PIXEL_PRECI;  /* -1 像素 */
            edge->x_cur.frac += edge->dy;
        }
    }
}
```

这里的 `step` 是预先计算好的"每行步进值"——`inte` 是 dx/dy 的整数部分，`frac` 是余数部分（以 dy 为分母）。逐行累加 `inte` 和 `frac`，当 `frac` 的绝对值超过 `dy` 时进位到 `inte`。这避免了每行都做整数除法的开销——除法只做一次（在初始化时），后续全是加法和比较。

---

## 1.9 弧与扇形的统一接口

### 1.9.1 设计理念

`ipgui_draw_arc` 模块将弧、圆、扇形、圆环四种形状统一为一个接口。四种形状共享相同的计算框架——环形遮罩（提供圆形的距离场）+ 半平面遮罩（裁剪弧的起始和终止角度）+ 线宽控制（决定是"填充"还是"描边"模式）。

### 1.9.2 角度裁剪的半平面方法

弧的"起始角度"和"终止角度"通过生成从圆心出发的两条径向边缘来实现裁剪。这两条径向边缘被转换为半平面遮罩：

```c
static void generate_angle_edge(
    ipgui_arc_angle_t angle,   /* 0~36000（100 倍放大） */
    ipgui_coord_t cx, ipgui_coord_t cy,
    ipgui_coord_t * x2, ipgui_coord_t * y2)
{
    s32_t sin_val = ipgui_sin(angle);  /* 三角函数查找表 */
    s32_t cos_val = ipgui_cos(angle);

    *x2 = cx + (ipgui_coord_t)cos_val;
    *y2 = cy + (ipgui_coord_t)sin_val;
}
```

角度值被放大 100 倍（例如 90° 表示为 9000），以整数形式参与三角函数查找。`ipgui_sin` 和 `ipgui_cos` 使用预计算的查找表，返回的值同样是放大的整数（16384 为 1.0）。

两条角度边缘和环形遮罩的叠加方式根据"扇形"还是"弧边"模式而不同。对于扇形，角度边缘之间是"填充区域"；对于弧边，角度边缘之间是"保留区域"。

### 1.9.3 边缘裁剪遮罩

`edge_clip_mask_with_aa` 和 `edge_clip_ring_mask_no_aa` 两个函数完成了角度边缘对环形遮罩的裁剪。前者在裁剪边界上保留了反走样过渡（使用半平面逐像素查询），后者直接做硬裁剪（无过渡带）。带反走样的裁剪在弧的边界处产生平滑效果，而硬裁剪适用于"已知后续操作会处理反走样"的场景。

---

## 1.10 Box 背景绘制——圆角矩形的区域分解

### 1.10.1 九区域分解策略

`ipgui_draw_box_background` 负责绘制一个圆角矩形的背景填充。其核心优化策略是将矩形区域分解为若干个不含圆角的矩形条和 4 个弧角区域：

```
        ┌───────┬───────────┬───────┐
        │ 左上角 │   上边条   │ 右上角 │
        │  r_lt  │           │  r_rt  │
        ├───────┼───────────┼───────┤
        │ 左边条 │   中心区域 │ 右边条 │
        │  r_lb  │           │  r_rb  │
        ├───────┼───────────┼───────┤
        │ 左下角 │   下边条   │ 右下角 │
        └───────┴───────────┴───────┘
```

中心区域和四条边条是不含圆角的纯色填充区域——它们直接调用 `ipgui_blend` 而不需要任何遮罩。只有四个角区域需要通过环形遮罩模块获取圆角遮罩。这种分解策略将需要遮罩计算的区域缩小到原来的约 20%（以圆角半径占总宽度 20% 估算）。

### 1.10.2 非对称圆角半径处理

ESDBox_IPGUI 支持四个角各自独立的圆角半径（`left_top_radius`、`right_top_radius`、`left_bottom_radius`、`right_bottom_radius`）。区域分解代码需要考虑左右两侧圆角半径的不对称性：

```c
ipgui_coord_t l_max = IPGUI_MAX(r_lt, r_lb);  /* 左侧最大半径 */
ipgui_coord_t r_max = IPGUI_MAX(r_rt, r_rb);  /* 右侧最大半径 */
ipgui_coord_t l_min = IPGUI_MIN(r_lt, r_lb);  /* 左侧最小半径 */
ipgui_coord_t r_min = IPGUI_MIN(r_rt, r_rb);  /* 右侧最小半径 */
```

当 `l_max > l_min` 时，左侧边条被切成两段：
- 高度避开**较大**圆角的一段（如果上角更大，则避开顶部；下角更大，则避开底部）
- 高度为剩余部分的一段（宽度为 `l_max - l_min`）

这个"避开"逻辑在源码中用 `if (r_lt >= r_lb)` 判定——直观理解是"哪个角更大就避开哪一侧"。这种分段处理保证了在不对称圆角场景下，每个矩形条都是无圆角的纯矩形。

### 1.10.3 get_max_radius——圆角半径的规整

`get_max_radius` 函数将用户指定的圆角半径限制为不超过矩形的半宽或半高：

```c
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

    *r_lt = IPGUI_MAX(0, *r_lt);
    *r_rt = IPGUI_MAX(0, *r_rt);
    *r_lb = IPGUI_MAX(0, *r_lb);
    *r_rb = IPGUI_MAX(0, *r_rb);
}
```

`IPGUI_MIN(rmax, ...)` 限制半径不超过半宽和半高的较小者——否则两个相邻的圆角会重叠。`IPGUI_MAX(0, ...)` 防止负数半径（来自 `style` 为 NULL 时的默认值 0）。

---

## 1.11 Box 阴影——SDF + 1D 多项式模糊

### 1.11.1 从 2D 卷积到 1D 映射

Box 阴影模块 `ipgui_draw_box_shadow` 是 ESDBox_IPGUI 中数学含量最高的模块之一。传统的 CSS box-shadow 效果基于 2D 高斯卷积：对每个像素，取样其周围 `blur × blur` 个像素并按高斯核加权平均。复杂度为 O(w × h × b²)。

ESDBox_IPGUI 的阴影模块通过以下两个等价变换将复杂度降为 O(w × h)：

1. **SDF 替代**：圆角矩形的有符号距离场（SDF）可以通过解析公式在 O(1) 时间内精确计算——无需 2D 卷积。
2. **一维映射替代**：将一维 SDF 距离值通过多项式函数映射为透明度值等价于高斯卷积的 CDF（累积分布函数）采样。

### 1.11.2 SDF 计算

对于圆角矩形，SDF 的计算公式是：

```
sdf_rounded_box(x, y) = distance_to_box(x, y) - corner_radius
```

其中 `distance_to_box` 是到矩形边界的有符号距离（内正外负），这与环形遮罩中使用的距离计算共享相同的数学基础。

源文件中使用了 Q8 定点数格式处理 SDF 计算——`#define Q8_ONE 256` 将 1.0 映射为 256。定点数转换、乘法和除法均使用宏 `TO_Q8`、`Q8_MUL`、`Q8_DIV` 实现，全部为整数运算。

### 1.11.3 多项式选择

源文件中提供了三种多项式，在精度和计算成本之间做权衡：

1. **三次 smoothstep**：`S₃(t) = t² × (3 - 2t)`，`t ∈ [0, 1]`。与高斯误差函数 erf(t) 的最大偏差 ≈ 0.017，在 8 位色深下人眼完全不可区分。等效于 σ ≈ blur/2.5 的高斯核。

2. **五次 smoothstep**：`S₅(t) = t³ × (10 - 15t + 6t²)`。与 erf(t) 的最大偏差 ≈ 0.007。边缘更柔和，适合较大的 blur 半径。

3. **二次抛物线**：`Q(t) = t × (2 - t)`。一阶可导，非 C² 连续（拐点处有不连续的二阶导数），有轻微的折线感。计算量最低，适合超低功耗 MCU。

### 1.11.4 模糊剖面缓存

模糊剖面 LUT（长度 = 2 × blur + 1 字节）存储了多项式函数在每个离散距离位置的输出值。这个 LUT 按 blur 半径缓存，并受 LRU 淘汰策略管理：

```c
/* ── 源文件: core/gfx/ipgui_draw_box_shadow.c ── */
typedef struct {
    u8_t  blur;              /* blur 半径（缓存 key） */
    u8_t  profile[1];        /* 柔性数组——实际长度为 2*blur+1 */
    /* ... LRU 链表节点在外部管理 */
} blur_profile_cache_t;
```

全局缓存上限为 8 项。对于典型的 16px 阴影，每条剖面仅占 33 字节。同一 blur 半径的多次绘制完全共享缓存。

### 1.11.5 纯整数平方根

SDF 计算中最昂贵的操作是平方根——距离公式需要 `sqrt(dx² + dy²)`。传统做法是用浮点库，但 ESDBox_IPGUI 实现了一个纯整数的 32 位平方根函数——二分位法（Binary Digit-by-Digit）：

```c
/* ── 源文件: core/gfx/ipgui_draw_box_shadow.c ── */
static u32_t ipgui_sqrt32(u32_t n)
{
    u32_t res  = 0;
    u32_t bit  = 1u << 30;  /* 从最高可能的位对开始 */

    while (bit > n) { bit >>= 2; }  /* 找到第一个 ≤ n 的位对 */

    while (bit != 0) {
        u32_t trial = res + bit;
        res >>= 1;
        if (n >= trial) {
            n   -= trial;
            res += bit;
        }
        bit >>= 2;
    }
    return res;
}
```

此算法基于"恢复余数开平方"原理。每次迭代测试一个位对（bit pair），如果当前结果的平方 ≤ n，则将位设置并从未知量中减去。16 次迭代完成，零分支，ARM Cortex-M 上约 40~60 周期，无需硬件除法器。

### 1.11.6 无分支绝对值/最值

阴影模块还包含一组无分支的数学工具函数：

```c
static inline s32_t ipgui_abs_s32(s32_t x)
{
    s32_t mask = x >> 31;         /* 正数: 0x00000000, 负数: 0xFFFFFFFF */
    return (x ^ mask) - mask;     /* 正数: x-0 = x, 负数: ~x+1 = -x */
}
```

`x >> 31` 在 C 语言中的行为是**实现定义**的（算术右移或逻辑右移）。在 ARM GCC 中默认为算术右移（保留符号位），使得此技巧成立。如果目标编译器不同，需要用 `(x < 0) ? -x : x` 代替。

这些无分支实现避免了分支预测失败导致的流水线冲刷——在 Cortex-M3/4 的 3 级流水线中，每次分支预测失败浪费 2-3 个周期。在数百万次调用的累积下，无分支版本的优势显著。

---

## 本章小结

本章从架构设计出发，逐层拆解了 ESDBox_IPGUI 图形绘制引擎的完整技术栈。

**子像素坐标系统**（1.2 节）是整个 gfx 模块的数学基础。Q26.6 定点数将离散的像素网格细化为 64 层精度，用位移替代乘除法，在 8 位颜色深度下精度足够。四个对齐工具函数（align_up_64 / align_down_64 / dist_to_lower_64 / dist_to_upper_64）提供了零分支的转换操作。

**半平面边缘遮罩**（1.3 节）是所有形状边界反走样的计算单元。边参化数据结构通过"平坦/陡峭"分类和修正因子查找表，在 O(1) 时间内完成任意角度线段的逐像素距离计算。127 字节的修正因子表用 0.2% 的内存预算换来了零浮点计算的精确反走样。

**环形遮罩**（1.4 节）针对 GUI 最高频的圆角矩形场景做了专门优化。SDF 预计算 + LRU 缓存的设计让圆角遮罩的逐像素查询降为约 10 条指令，典型命中率超过 95%。

**WDF 宽线遮罩**（1.5 节）将"零宽线反走样"推广到"任意宽线反走样"，通过宽度分布函数在线段中轴两侧生成连续过渡的遮罩值。

**图元绘制模块**（1.6-1.9 节）将遮罩与颜色合成组合为可调用的公开 API。线段绘制（Wu 算法）、三角形光栅化（三点排序+双半平面遮罩）、多边形光栅化（扫描线+AVL 活动边表）、弧绘制（环形遮罩+角度半平面裁剪）各自采用了该领域最经典的算法并经过嵌入式适配。

**Box 绘制模块**（1.10-1.11 节）展示了实用层面的工程优化：背景填充通过九区域分解将遮罩计算需求压缩到 20%；阴影模块通过 SDF+1D 多项式映射将 `O(w·h·b²)` 的 2D 卷积降为 `O(w·h)` 的逐像素查表。

### 重点知识回顾

1. **Q26.6 定点数**：32 位整数，高 26 位为整数部，低 6 位为小数部，精度 1/64，适合 8 位色深反走样
2. **半平面遮罩**：根据 `|dx|` 与 `dy` 的大小关系分类边（平坦/陡峭），用不同扫描方向和距离转换公式计算每个像素到边界的距离
3. **修正因子表**：127 元素 `u8_t` 查找表，存储斜率修正系数，分母 256
4. **SDF（有符号距离场）**：存储每个像素到最近形状边界的带符号距离，正数为内部、负数为外部
5. **LRU 缓存策略**：按最后使用时间戳排序的链表，全局淘汰上限 8~12 项，用于环形遮罩和模糊剖面复用
6. **Wu 线算法**：基于 Bresenham 误差累积器的反走样细线算法，每列两个像素各按距离比例分配透明度
7. **扫描线 + AVL 活动边表**：多边形光栅化的标准算法，AVL 树保证 O(log n) 边插入/删除
8. **SDF+1D 多项式映射**：将 2D 高斯卷积降维为 O(1) 解析 SDF + O(1) 多项式查表，复杂度从 `O(w·h·b²)` 降为 `O(w·h)`

### 思考问题

1. 如果子像素精度改为 256（Q24.8），`dist_to_lower_64` 和 `align_up_64` 中的位掩码应如何修改？这对精度和性能各有什么影响？
2. 修正因子表为什么只需要 127 个值（索引 0~126）而不是 256 个？当索引为 0 或 126 时，对应的边斜率分别是多少？
3. `edge_x_at_y` 中使用 `(s64_t)dy * p->dx / p->dy` 而非 `dy * p->dx / p->dy`。如果省略 64 位中间类型，在最大屏幕 1024×768 和 45° 斜线的情况下，乘积会溢出吗？请计算。
4. 多边形光栅化中的 `EDGE_EMBED_NUM` 设为 10。如果用户传入一个 20 边形，系统会如何处理？降级渲染的策略可以是怎样的？
5. Box 阴影使用三次 smoothstep (`t²×(3-2t)`) 而非五次 smoothstep 作为默认选项。这两者在数学上和视觉上各有什么差异？什么场景下值得换用五次？

---
