# 第二章 混合合成系统——从遮罩到像素

> **模块位置**：`core/composite/` + `al/hal/` + `core/ui/widget_manager/`
> **核心职责**：接收 gfx 输出的 Mask + Paint → 逐像素混合 → 写入 Surface
> **关键设计**：Paint 分发器、Premultiplied Alpha、像素格式函数表、RGB565 Packed Blend、脏矩形系统、矩形切片器、屏幕渲染管线

---

## 2.1 架构概览

### 2.1.1 模块定位与职责分界

composite 模块在整个渲染管线中承担"颜色落笔"的最终职责。它在管线中的位置是 gfx 模块的直接下游——接收来自 gfx 的两样东西，然后将颜色写入屏幕表面的像素缓冲区。

两样输入分别是：

1. **Mask（遮罩数组）**：一个 `u8_t` 类型的数组，长度等于目标绘制区域的像素数。每个元素取值 0~255，0 表示"此像素完全透明，不对其做任何颜色操作"，255 表示"此像素完全不透明，用 paint 的颜色完全覆盖"，中间值表示按比例的半透明混合。

2. **Paint（涂料描述）**：一个 `ipgui_paint_t` 结构体，描述用于填充的颜色来源。可能是纯色（一个 `ipgui_color_t` 值）、渐变（一个 `ipgui_grad_src_t` 结构体，含类型和参数）、或图像（一个 `ipgui_image_src_t` 结构体，含像素数据和格式）。

composite 模块不关心 mask 中的值是如何产生的——是直线穿过像素时的距离计算结果、是圆角边缘的 SDF 值、还是多边形的填充规则判定。它只关心"这个像素的遮罩值是 128，所以颜料作用一半"。这种"不知道、不关心"的设

记性使得两个模块完全解耦：

```
gfx 模块输出: mask[] ────────────┐
                                  ├──→ ipgui_blend() → 像素颜色写入 surf->color
gfx 模块输出: paint ─────────────┘
```

### 2.1.2 Paint 分发器——Tagged Union

`ipgui_paint_t` 是 gfx 和 composite 之间的核心桥梁结构体。它使用 C 语言的 tagged union 设计模式：

```c
/* ── 源文件: core/composite/ipgui_blend.h ── */
typedef enum {
    IPGUI_PAINT_COLOR,        /* 类型 A: 纯色 */
    IPGUI_PAINT_GRADIENT,     /* 类型 B: 渐变 */
    IPGUI_PAINT_IMAGE,        /* 类型 C: 图像 */
} ipgui_paint_type_t;

typedef struct {
    ipgui_paint_type_t type;  /* 标签字段 */
    union {
        ipgui_color_t     color;       /* 类型 A 的数据 */
        ipgui_grad_src_t  grad_src;    /* 类型 B 的数据 */
        ipgui_image_src_t image_src;   /* 类型 C 的数据 */
    } src;
} ipgui_paint_t;
```

分发函数 `ipgui_blend()` 的唯一职责是读取 `paint->type` 字段，然后跳转到对应的子函数：

```c
/* ── 源文件: core/composite/ipgui_blend.c ── */
void ipgui_blend(
    ipgui_surf_t * surf, ipgui_aabb_t * clip, ipgui_aabb_t * dest,
    ipgui_paint_t * paint, u8_t opacity, u8_t * mask,
    ipgui_aabb_t * mask_aabb, ipgui_blend_mode_t blend_mode)
{
    switch (paint->type) {
    case IPGUI_PAINT_COLOR:
        ipgui_blend_color(surf, clip, dest,
            paint->src.color, opacity, mask, mask_aabb, blend_mode);
        break;
    case IPGUI_PAINT_GRADIENT:
        ipgui_blend_gradient_color(surf, clip, dest,
            &(paint->src.grad_src), opacity, mask, mask_aabb, blend_mode);
        break;
    case IPGUI_PAINT_IMAGE:
        ipgui_blend_image_v2(surf, clip, dest,
            &(paint->src.image_src), opacity, mask, mask_aabb, blend_mode);
        break;
    default:
        break;
    }
}
```

这个"薄薄的分发层"只有约 30 行代码，但它是整个系统的核心抽象层。它的价值在于隔离性：如果需要新增一种涂料类型（比如"视频帧流"），需要修改的地方仅限于三处——枚举加一个值、union 加一个成员、switch 加一个 case——而且全部在 composite 模块内部。任何调用 `ipgui_blend()` 的 gfx 层代码一行不动。

`opacity` 参数提供了全局透明度控制——在 mask 的逐像素遮罩值之上再加一层整体透明度衰减。`blend_mode` 参数支持未来扩展不同的混合模式（如叠加、正片叠底），但在当前实现中大多数混合函数将其忽略。

### 2.1.3 composite 模块的文件结构

```
core/composite/
├── ipgui_blend.c/h                    ── Paint 分发器（核心入口）
├── ipgui_blend_mode.h                 ── 混合模式枚举定义
├── ippgui_color.h                     ── ipgui_color_t 颜色结构体
├── blend_color/
│   └── ipgui_blend_color.c/h          ── 纯色混合（函数表 + packed blend）
├── blend_gradient/
│   └── ipgui_blend_gradient_color.c/h ── 渐变混合（方向优化快速通道）
├── blend_image/
│   └── ipgui_blend_image.c/h          ── 图像混合（格式交叉矩阵）
└── blend_icon/
    └── ipgui_blend_icon.c/h           ── 图标混合（字体/字形专用）

core/ui/widget_manager/
├── ipgui_dirty_rect.c/h               ── 脏矩形管理器（DFS 收集 + 相邻合并）
└── ipgui_widget_tree.c/h              ── 控件树遍历（Z-order DFS）

al/hal/
├── ipgui_screen.c/h                   ── 屏幕渲染主入口（PFB 分配 + 逐控件递归）
└── ipgui_rect_slice.c/h               ── 矩形切片器（贪心列优先 + 除法消除）
```

注意脏矩形管理器、控件树和屏幕渲染入口虽然不属于 `core/composite` 目录，但从完整的渲染管线视角看，它们是 composite 渲染不可缺少的调度层基础设施。本章将从完整的渲染链路视角统一讨论这些模块。

---

## 2.2 Alpha 混合的数学基础

### 2.2.1 Porter-Duff Over 算子

Porter 和 Duff 于 1984 年在卢卡斯影业的计算机图形部门推导了一套基于 alpha 通道的图像合成公式，其中最重要的算子是 **Over**——将前景（源，Source）叠加在背景（目标，Destination）之上。

在连续域中，公式为：

```
α_result = α_s + α_d × (1 - α_s)
C_result = (C_s × α_s + C_d × α_d × (1 - α_s)) / α_result
```

其中 `C_s`、`C_d` 是前景和背景的颜色（R/G/B 各通道独立计算），`α_s`、`α_d` 是各自的不透明度（0~1 连续值）。

在 8 位整数域（0~255）中，ESDBox_IPGUI 使用以下等价形式：

```
result_channel = (fg × fg_alpha + bg × (255 - fg_alpha)) >> 8
```

这里 `fg_alpha` 是前景颜色的 Alpha 通道值。注意使用 `>> 8`（除以 256）而非 `/ 255`。两者在 8 位精度下的最大误差为 ±1——当 fg_alpha = 255 时，`bg × 0 >> 8 = 0`（正确，完全覆盖），除以 255 也是 0。当 fg_alpha = 128 时，`bg × 127 >> 8 = bg × 127 / 256`，理论值为 `bg × 127 / 255`，最大相对误差 = `127/256 - 127/255 ≈ 0.00196`，在 8 位通道中约等价于 ±0.5（半个灰度级），人眼完全不可分辨。

选择 `>> 8` 而非 `/ 255` 的原因是：ARM Cortex-M 系列处理器中，32 位整数除法（`SDIV` 指令）需要 2~12 个周期（取决于具体型号和操作数值），而 `>> 8` 仅需 1 个周期（逻辑右移）。在 320×240×3 = 230,400 个通道混合运算中，减少 11 个周期 × 230,400 = 节省约 2.5 毫秒——在 60fps 的帧预算（16.6ms）中占比约 15%。

### 2.2.2 Standard Alpha vs Premultiplied Alpha

Standard Alpha（标准 Alpha）格式存储的是"未与 Alpha 相乘的原始颜色值"。混合时需要先做三次前景乘法：

```
R = (R_s × α + R_d × (255 - α)) >> 8
G = (G_s × α + G_d × (255 - α)) >> 8
B = (B_s × α + B_d × (255 - α)) >> 8
```

Premultiplied Alpha（预乘 Alpha）格式在存储时就已经将颜色通道与 Alpha 通道相乘。混合时第一项直接使用存储值，无需再做前景乘法：

```
R = R_s_pm + ((R_d × (255 - α)) >> 8)
G = G_s_pm + ((G_d × (255 - α)) >> 8)
B = B_s_pm + ((B_d × (255 - α)) >> 8)
```

每个像素节省 3 次 8 位乘法。在 76800 像素的屏幕上，每帧节省 230,400 次乘法。在 Cortex-M 上，8 位乘法（`MUL`）约 1 个周期，总计节省约 0.23 兆周期——在 48MHz 主频下约 4.8 毫秒。这 4.8 毫秒可以用来多渲染一个控件或一行文本。

```c
/* ── 源文件: core/composite/blend_color/ipgui_blend_color.c:729-750 ── */
ipgui_color_t ipgui_color_premultiply(ipgui_color_t * color)
{
    ipgui_color_t res;
    u8_t ca = color->a;
    res.r = (u8_t)((color->r * ca) >> 8);
    res.g = (u8_t)((color->g * ca) >> 8);
    res.b = (u8_t)((color->b * ca) >> 8);
    res.a = ca;
    return res;
}
```

当 ca = 0（完全透明）时，预乘结果的所有通道为零，`color.v == 0` 可用于快速路径跳过。当 ca = 255（完全不透明）时，预乘结果与原始值相同。

### 2.2.3 颜色透明度组合函数

`ipgui_color_combine_opacity` 将外部 opacity（用户设置的全局不透明度）与颜色的固有 Alpha 通道相乘：

```c
ipgui_color_t ipgui_color_combine_opacity(ipgui_color_t * color, u8_t opacity)
{
    ipgui_color_t res;
    IPGUI_COLOR_SET_R(res, color->r);
    IPGUI_COLOR_SET_G(res, color->g);
    IPGUI_COLOR_SET_B(res, color->b);
    IPGUI_COLOR_SET_A(res, (u8_t)((color->a * opacity) >> 8));
    return res;
}
```

注意此函数**不改变** RGB 通道的值——它只缩放 Alpha 通道。真正的预乘在随后调用 `ipgui_color_premultiply()` 时完成。这两步分离的设计允许代码在 RGB 通道保持不变的情况下独立调整透明度，这在需要"同一颜色、不同透明度多次绘制"的场景中非常有用。

### 2.2.4 倒数查找表优化

对于 ARGB8888 等含 Alpha 通道的 32 位像素格式，混合后的颜色通道需要除以最终 Alpha 值（去预乘）。这涉及到 `r12 = (r12 << 8) / alpha12` 的除法操作。

为了避免运行时除法，系统维护了一张 256 项的倒数查找表：

```c
#if USE_INV_TABLE == 1
const u16_t g_inv_tbl[256] = {
    0, 65025, 32512, 21675, 16256, 13005, 10837, 9289,
    8128, 7225, 6502, 5911, 5418, 5001, 4644, 4335,
    /* ... 256 个预计算值 ... */
};
#endif
```

表中的值 = `255 × 255 / alpha`（Q8.8 格式）。使用查表替代除法：

```c
#if USE_INV_TABLE == 0
    r12 = alpha12 ? (r12 << 8) / alpha12 : 0;  /* 运行时除法 */
#else
    r12 = (r12 * g_inv_tbl[alpha12]) >> 8;     /* 查表乘 + 移位 */
#endif
```

256 项的 `u16_t` 表共占 512 字节。当 `USE_INV_TABLE` 设为 0（节省内存）时退化为运行时除法。这在配置头文件中通过 `USE_INV_TABLE` 宏控制——允许用户在"内存换速度"和"速度换内存"之间做编译期选择。

---

## 2.3 像素格式函数表——表驱动混合器

### 2.3.1 设计动机

不同的显示器使用不同的像素格式——即每个像素的 RGB 通道如何排列在内存中。常见的格式有：

| 格式 | 每像素位数 | 字节序 | 通道排列 | 典型用途 |
|------|----------|--------|---------|---------|
| RGB565 | 16 bit | 小端 | RRRRRGGG GGGBBBBB | 低成本 TFT LCD |
| BGR565 | 16 bit | 小端 | BBBBBGGG GGGRRRRR | 特定驱动芯片 |
| RGB888 | 24 bit | - | [R][G][B] | 中端 LCD |
| BGR888 | 24 bit | - | [B][G][R] | 特殊显示控制器 |
| ARGB8888 | 32 bit | 小端 | [A][R][G][B] | 高端 TFT、层叠合成 |
| ABGR8888 | 32 bit | 小端 | [A][B][G][R] | 特定 GPU 格式 |
| RGBA8888 | 32 bit | 小端 | [R][G][B][A] | OpenGL 默认格式 |
| BGRA8888 | 32 bit | 小端 | [B][G][R][A] | Windows GDI 格式 |

核心挑战：Alpha 混合公式是**格式无关**的——它在抽象的"R 通道、G 通道、B 通道"维度上运算。但实际写入屏幕内存时，必须知道具体的字节排布方式。

### 2.3.2 混合函数表

Composite 模块通过一张全局函数表消除格式差异：

```c
/* ── 源文件: core/composite/blend_color/ipgui_blend_color.c ── */
premult_blend_func_t premult_blend_table[PIX_FMT_MAX] = {
    [PIX_FMT_RGB565]   = ipgui_builtin_premultiplied_color_blend_to_rgb565,
    [PIX_FMT_BGR565]   = ipgui_builtin_premultiplied_color_blend_to_bgr565,
    [PIX_FMT_RGB888]   = ipgui_builtin_premultiplied_color_blend_to_rgb888,
    [PIX_FMT_BGR888]   = ipgui_builtin_premultiplied_color_blend_to_bgr888,
    [PIX_FMT_ARGB8888] = ipgui_builtin_premultiplied_color_blend_to_argb8888,
    [PIX_FMT_ABGR8888] = ipgui_builtin_premultiplied_color_blend_to_abgr8888,
    [PIX_FMT_RGBA8888] = ipgui_builtin_premultiplied_color_blend_to_rgba8888,
    [PIX_FMT_BGRA8888] = ipgui_builtin_premultiplied_color_blend_to_bgra8888,
};
```

调用方式极其简洁：

```c
premult_blend_table[surf->pix_fmt](color, pixel_ptr, blend_mode);
```

数组索引是 O(1) 操作（寄存器偏移寻址）。相比之下，if-elseif-else 链的最坏情况需要 8 次比较，switch-case 的跳转表同样依赖索引（但编译器可能生成 if-else 链），函数表是最确定性的优化方式。

与之平行的还有 `solid_conv_table`（纯色转换表），将 `ipgui_color_t` 转换为特定像素格式的原始字节。

### 2.3.3 RGB565 Packed Blend——通道掩码优化

对于 RGB565 格式（16 位，5+6+5 位），混合函数使用了一个精巧的 packed blend 优化。核心思想是**在 packed 空间中直接做 Alpha 混合**，而不是先拆分为独立通道、混合后再重组。

```c
/* ── 源文件: core/composite/blend_color/ipgui_blend_color.c ── */
#define MASK_RB     0xf81fU     /* 位15:11 + 位4:0，即高5位和低5位 */
#define MASK_G      0x07e0U     /* 位10:5，中间6位 */
#define MASK_MUL_RB 0x3e07c0U   /* MASK_RB << 6，乘以最大alpha(64)后的范围 */
#define MASK_MUL_G  0x1f800U    /* MASK_G  << 6 */

void ipgui_builtin_premultiplied_color_blend_to_rgb565(
    ipgui_color_t color, u8_t * rgb565, ipgui_blend_mode_t blend_mode)
{
    /* Alpha 转换为 6 位精度（+2 为四舍五入） */
    u32_t ialpha6 = (255 - IPGUI_COLOR_A(color) + 2) >> 2;
    u32_t bg = *(u16_t *)rgb565;

    /* 背景 R 和 B 通道（共用掩码 MASK_RB）同时做缩放 */
    u32_t bg_rb = ((ialpha6 * (bg & MASK_RB)) & MASK_MUL_RB) >> 6;
    /* 背景 G 通道单独缩放（在中间位，需要不同掩码防溢出） */
    u32_t bg_g  = ((ialpha6 * (bg & MASK_G )) & MASK_MUL_G ) >> 6;

    /* 前景色转换为 RGB565 packed 格式 */
    u32_t fg565 = ((u32_t)(IPGUI_COLOR_R(color) >> 3) << 11)
                | ((u32_t)(IPGUI_COLOR_G(color) >> 2) <<  5)
                | ((u32_t)(IPGUI_COLOR_B(color) >> 3)      );

    /* Packed 直接相加：premultiplied 保证各通道不溢出 */
    *(u16_t *)rgb565 = (u16_t)(
        ((fg565 & MASK_RB) + bg_rb) | ((fg565 & MASK_G) + bg_g)
    );
}
```

此算法的几个关键细节：

1. **Alpha 降精度为 6 位**：`ialpha6 = (255 - alpha + 2) >> 2`。`255 - alpha` 得到不透明度（inverse alpha），`+2` 是四舍五入修正，`>> 2` 等价于除以 4 映射到 0~63 范围。为什么用 6 位？因为 `MASK_MUL_RB = 0x3e07c0` 恰好是 `(MASK_RB << 6)`，6 位精度的 Alpha 与背景色相乘后不会溢出到相邻通道。

2. **R/B 通道共用一个掩码做批量乘法**：`(ialpha6 * (bg & MASK_RB))` 同时运算高 5 位（R）和低 5 位（B）的缩放，因为它们被 G 通道（6 位）隔开，`<< 6` 后的值范围 `0x3e07c0` 不会导致 R 溢出到 G 或 B 溢出到更高位。这是一个充分利用 RGB565 格式固有间距的位级优化。

3. **通道独立相加**：前景色是预乘格式（PM Alpha），各通道值已经缩放到合理范围。加上背景缩放值后，结果一定在各自通道的位数范围内，无需额外的溢出保护。

这种 packed blend 的性能远优于"拆开→混合→重组"的传统方式：每条 32 位指令处理多个通道，整个混合过程约 10 条指令完成。

对比 BGR565 的实现几乎完全对称——仅交换前景色中 R 和 B 的位移位置：
```
RGB565: R<<11 | G<<5 | B<<0
BGR565: B<<11 | G<<5 | R<<0
```

### 2.3.4 32 位 RGBA 格式的双层混合

对于 ARGB8888 等 32 位格式，混合逻辑需要处理两层的 Alpha 交互——背景层本身也有自己的 Alpha 通道（表示它在更底层背景上的透明程度）。这需要完整的两层 Alpha 合成：

```c
/* 两层透明度合并：
 * alpha12 = 1 - (1-alpha1) × (1-alpha2) = alpha1 + alpha2 - alpha1×alpha2
 * 转换为整数域：alpha12 = 255 - ((255-alpha)×invalpha2 >> 8)
 */
u32_t alpha12 = 255 - (((255 - alpha) * ialpha2) >> 8);

/* 颜色通道：C12 = (C1×alpha1×(1-alpha2) + C2×alpha2) / alpha12 */
u32_t r12 = ((r * alpha * ialpha2) >> 16) + IPGUI_COLOR_R(color);
```

其中 `(r * alpha * ialpha2) >> 16` 等价于 `r × alpha/256 × (256-alpha2)/256`。`>> 16` 是两次 `>> 8` 的合并——因为 `alpha` 和 `ialpha2` 都是 0~255 范围，它们的乘积在 0~65025 范围（16 位足够），再乘以 8 位通道值后才需要 24 位精度。

最终的"去预乘"（除以 alpha12）有两种实现策略：
- **运行时除法**（`USE_INV_TABLE == 0`）：`(r12 << 8) / alpha12`，含 ARM SDIV 指令
- **倒数表查表**（`USE_INV_TABLE == 1`）：`(r12 * g_inv_tbl[alpha12]) >> 8`，免除法

注意大端/小端的处理差异——32 位 `u32_t` 的字节序在内存中的排布决定了通道提取的位移公式完全不同。代码中通过 `#if IPGUI_ENDIAN_LITTLE == 1` 编译期条件分别处理。

---

## 2.4 三种"涂料"的混合实现

### 2.4.1 纯色混合

纯色是最常见也是最优化的涂料类型。大多数 UI 元素的背景和边框都是纯色。纯色混合的核心循环极其简洁：

```
for each pixel in dest:
    m = mask[pixel_index]
    if m == 0: continue              # 快速路径 1: 完全透明
    if m == 255:                     # 快速路径 2: 完全不透明
        write_premultiplied_color(color, pixel)
    else:
        blend_with_mask(color, pixel, m)
```

两个快速路径的命中率决定性能：
- **`m == 0`**：控件外部的像素，遮罩值为 0，跳过所有计算。在大多数 UI 场景中占比 > 60%。
- **`m == 255`**：控件内部的纯色区域，遮罩值为 255，直接用预乘颜色覆盖。占比约 20-30%。
- **`0 < m < 255`**：反走样边缘过渡带，只占总像素的 10-20%。

这种分支设计使得混合函数在大部分像素上开销极低（一跳过或一赋值），只有少数边缘像素走完整的 Alpha 混合路径。

### 2.4.2 渐变混合——方向优化的快速通道

渐变混合模块 `ipgui_blend_gradient_color` 支持线性渐变、径向渐变和锥形渐变三种类型。对于线性渐变，系统识别渐变方向并采取对应的优化策略。

**水平渐变**：颜色仅沿 x 方向变化，同一行所有像素使用相同的渐变采样值。只需要在每行开头算一次颜色，整行复用。

**垂直渐变**：颜色仅沿 y 方向变化，同一列所有像素使用相同的渐变采样值。只需在每列开头算一次颜色，整列复用。

方向识别通过对渐变向量的分量判断实现：
```
if (abs(grad.end_x - grad.start_x) > abs(grad.end_y - grad.start_y)):
    水平渐变 → O(h) 次颜色采样
else:
    垂直渐变 → O(w) 次颜色采样
```

这种优化将逐像素的渐变颜色计算从 `O(w × h)` 次降为 `O(w)` 或 `O(h)` 次——对于 200×200 的区域，从 40,000 次采样降到 200 次，加速 200 倍。

对于倾斜渐变，无法使用整行/整列复用，回退到逐像素采样。但由于倾斜渐变在 UI 中占比很小（大多数渐变是纯水平或纯垂直的），这一慢路径对总体性能影响微乎其微。

### 2.4.3 图像混合——格式交叉矩阵

图像混合是最复杂的涂料类型。源图像本身的像素格式（PNG = RGBA8888, JPEG = RGB888, BMP = 多种）与屏幕像素格式（RGB565/ARGB8888等）之间需要格式转换 + Alpha 混合的组合操作。

ESDBox_IPGUI 的 `ipgui_blend_image.c` 模块维护了一张"源格式 → 屏幕格式"的转换函数表，覆盖了所有实用的格式组合。表中每个函数都为特定的格式对做了专门优化——内联展开、无分支、充分利用 ARM 的位操作指令。

图像模块同时处理缩放（双线性插值或最近邻）和坐标变换（平铺、拉伸），这些与像素混合在同一个循环中完成，避免了为每个操作单独分配中间缓冲区的开销。

---

## 2.5 脏矩形系统——只渲染改变的区域

### 2.5.1 脏矩形的概念

在交互式 UI 中，每帧之间只有少数区域会发生变化。例如，用户点击一个按钮，导致状态栏文字从"就绪"变为"处理中"——只有状态栏区域（约 320×20 像素）需要重绘，其余 320×220 像素保持原样。

脏矩形系统的核心思想是：控件在状态变化时标记自身为"脏"，渲染调度器收集所有脏区域并只对脏区域执行绘制。这避免了每帧在 76,800 个像素上运行完整的渲染管线。

### 2.5.2 DFS 收集——深度优先遍历控件树

脏矩形管理器 `ipgui_dirty_rect_mgr` 通过深度优先遍历（DFS）控件树收集脏区域：

```
ipgui_dirty_rect_collect(widget_root, dirty_list):
    for each child in DFS order:
        if child->has_dirty:
            abs_pos = widget_abs_pos(child)   # 计算控件的屏幕绝对坐标
            dirty_rect_add(dirty_list, abs_pos, child->flags)
        collect(child, dirty_list)             # 递归子控件
```

DFS 遍历使用 Z-order（先序遍历）确保父控件先于子控件处理。

### 2.5.3 相邻合并策略

脏矩形管理器实现了相邻合并优化：如果两个脏矩形贴近或重叠，将它们合并为一个更大的矩形。合并判断基于矩形之间的间距——如果水平间距小于阈值且垂直重叠，合并为水平联合；如果垂直间距小于阈值且水平重叠，合并为垂直联合。

合并策略的权衡：
- **合并为一个大矩形**：单次渲染区域大，但只需一次 PFB 分配和一次 blit（屏幕刷入）
- **保持为多个小矩形**：总渲染面积小，但需要多次 PFB 分配和多次 blit

系统采用启发式算法：当两个矩形的重叠面积超过较小矩形面积的 50% 时合并，否则保持独立。这个阈值是在实际 UI 场景中通过基准测试确定的。

---

## 2.6 矩形切片器——大脏矩形的分片处理

### 2.6.1 内存约束

嵌入式系统的 PFB（部分帧缓冲）大小通常在 4KB~32KB 之间。对于 320×240 的 RGB565 屏幕，全帧缓冲区需要 `320×240×2 = 153,600` 字节（150KB），远超许多 MCU 的 RAM 容量。

当脏矩形过大（比如全屏刷新），一次分配覆盖整个脏矩形的 PFB 在物理上不可能。矩形切片器的作用是：将大矩形切成若干个"面条般的"小矩形片（slices）——每片的高度使得其所需内存不超过 PFB 缓冲区容量。

### 2.6.2 贪心列优先策略

矩形切片器采用"贪心列优先"策略：

```
ipgui_rect_slice(big_rect, slices):
    max_rows = PFB_SIZE / (big_rect.width × pix_size)
    for y from big_rect.start_y to big_rect.end_y step max_rows:
        slice.start_x = big_rect.start_x
        slice.end_x   = big_rect.end_x
        slice.start_y = y
        slice.end_y   = min(y + max_rows - 1, big_rect.end_y)
```

"贪心"的含义是：每片尽可能宽（使用脏矩形的全宽），使每片处理的行数最大化，减少切片总数。"列优先"指的是水平方向不切——保持全宽——只在垂直方向上切成多条。这是因为水平跨行会导致额外的内存寻址开销（stride 跳转），而垂直切分不增加寻址复杂度。

切片器还包含除法消除优化：`big_rect.width × pix_size` 在循环外预计算一次（编译时常量），循环内只做减法（`min(y + max_rows - 1, ...)`）和赋值，零除法操作。

---

## 2.7 屏幕渲染管线——从输入到像素

### 2.7.1 完整渲染流程

`ipgui_screen_render` 函数是 ESDBox_IPGUI 渲染管线的顶层入口。一次完整的屏幕刷新经过以下阶段：

```
T=0ms   用户事件触发控件状态变化
        ↓
T=1ms   控件回调标记自身为脏（ipgui_widget_set_dirty）
           → 沿控件树向上传播脏标记到根节点
        ↓
T=2ms   脏矩形收集（DFS 遍历控件树）
           → 收集所有脏标记控件的屏幕绝对 AABB
           → 自动合并相邻矩形
        ↓
T=3ms   对每个脏矩形：
        1. 矩形切片器切分出若干片（每片高度 ≤ PFB 容量）
        2. 为每片分配临时像素缓冲区（PFB slice）
        3. 逐片执行：
        ↓
T=4ms      a) DFS 遍历控件树（仅遍历与当前片相交的控件分支）
T=5ms      b) 对每个相交控件调用 render() 回调
               → gfx 模块绘制控件 → 输出 mask[] + paint
               → composite 模块混合 → 写入 PFB
T=7ms      c) 将 PFB 内容刷到屏幕驱动（flush / blit）
        ↓
T=9ms   所有脏矩形处理完毕 → 屏幕上的像素更新完成
T=10ms  等待下一帧（剩余 6.6ms 用于用户代码和其他任务）
```

总耗时约 10 毫秒。帧预算 16.6 毫秒（60fps）。余量约 6.6 毫秒——充足。

### 2.7.2 DFS 渲染中的两级裁剪

DFS 渲染遍历（`ipgui_screen_render_widget_dfs`）使用两级裁剪策略来避免不必要的渲染：

**第一级：脏矩形裁剪**。控件的全局 AABB 与当前脏矩形片做求交。如果控件完全在脏矩形外（`ipgui_aabb_overlap` 返回非零），跳过该控件及其整个子树。

**第二级：父控件边界裁剪**。当父控件设置了 `OVERFLOW_HIDDEN`（默认），子控件超出父控件边界的部分被裁剪。裁剪逻辑通过 `parent_clip` 在 DFS 递归中累积——每深入一层，将当前控件的边界与上层累计的裁剪区求交。超出交集的像素不渲染。

`OVERFLOW_VISIBLE` 标志位允许特定控件（如弹出菜单、下拉列表）突破父控件的裁剪边界。当设置了此标志时，DFS 不将当前控件的边界加入子控件的累积裁剪区。

### 2.7.3 坐标系统选择

ESDBox_IPGUI 的控件渲染采用 Option C——**控件本地坐标系**。在调用控件自身的 `render()` 回调之前，系统将所有坐标（surf、clip、parent_clip）从全局坐标平移为以控件左上角为原点的本地坐标：

```c
local_surf.surf.start.x = global_surf.surf.start.x - global_aabb.start.x;
local_clip.start.x      = ctx->clip->start.x - global_aabb.start.x;
```

这个设计选择使得控件绘制代码无需关心自己在屏幕上的绝对位置——它始终从 `(0, 0)` 开始绘制，`widget->w` 和 `widget->h` 就是画布边界。这种"相对坐标系"的设计极大地简化了控件实现——开发者编写一个按钮控件时，不需要了解它最终会被放在屏幕的哪个位置。

坐标平移是纯的加减法（每个坐标减一个偏移量），无需乘法或除法，在 DFS 遍历中几乎零开销。

### 2.7.4 PFB 缓冲区管理

PFB 初始化函数 `ipgui_scr_create_pfb` 负责管理部分帧缓冲区的生命周期：

```c
ipgui_err_t ipgui_scr_create_pfb(
    ipgui_scr_t * scr, u8_t * buf, u32_t buf_size, ipgui_pix_fmt_t pix_fmt)
{
    /* 1. 对齐到 32 位边界（ARM 未对齐访问的性能惩罚） */
    u8_t * pfb_buf = IPGUI_ALIGN_U32(buf);
    u32_t valid_size = (buf + buf_size) - pfb_buf;

    /* 2. 根据像素格式计算每像素字节数 */
    u8_t px_size;
    switch (pix_fmt) {
        case PIX_FMT_RGB565: case PIX_FMT_BGR565: px_size = 2; break;
        case PIX_FMT_RGB888: case PIX_FMT_BGR888: /* fall through */
        case PIX_FMT_RGBA8888: case PIX_FMT_BGRA8888: px_size = 4; break;
    }

    /* 3. 计算最大可存储的像素数 */
    scr->pfb.num_pixs = valid_size / px_size;
    scr->pfb.color    = pfb_buf;

    /* 4. 清零 */
    ipgui_memset(pfb_buf, 0, valid_size);
}
```

对齐要求（`IPGUI_ALIGN_U32`）是因为 ARM Cortex-M3/4 在未对齐的 32 位访问上会触发硬件异常（UsageFault）。即使某些 M4 配置允许未对齐访问，其性能惩罚也高达 2-3 倍——总线需要两次内存事务来完成一次未对齐的字读取。

### 2.7.5 像素格式兼容性处理

注意 `RGB888` 和 `BGR888` 共享 `px_size = 4`（而非 3）。这是因为 24 位像素在 ARM 上无法高效对齐——`3 × n` 几乎永远不是 4 的倍数。系统将 24 位像素填充为 32 位（padding byte），以确保每行对齐到 32 位边界。这一设计在屏幕驱动层面完成——驱动内部分配 `width × 4` 的缓冲区，写入时跳过 padding 字节。

---

## 本章小结

本章从架构设计出发，逐层拆解了 ESDBox_IPGUI 颜色合成系统的完整技术栈。

**Alpha 混合数学基础**（2.2 节）奠定了颜色合成的理论根基。Porter-Duff Over 算子在 8 位整数域等价于 `(fg × α + bg × (255-α)) >> 8`。预乘 Alpha 格式在存储时预先完成颜色与 Alpha 的乘法，使得混合时每像素节省 3 次 8 位乘法——在 320×240 屏幕上每帧节省约 4.8 毫秒。`>> 8` 替代 `/ 255` 的优化利用位移替代除法，在 Cortex-M 上节省 2~12 个周期每次。

**Paint 分发器**（2.1.2 节）是整个系统的解耦枢纽。Tagged union + switch 的设计将纯色、渐变、图像三种涂料统一抽象，新增涂料类型只需在复合模块内部修改三处，gfx 层不受影响。

**像素格式函数表**（2.3 节）通过 O(1) 数组索引将格式差异完全消除。RGB565 的 packed blend 优化利用通道掩码在 packed 空间内并行处理 R/B 两个通道，整次混合仅约 10 条指令。ARGB8888 的双层混合完整实现两层 Alpha 交互，支持倒数表优化（512 字节换零除法运算）。

**三种涂料混合**（2.4 节）分别做了针对性优化：纯色走 `m==0` / `m==255` 快速路径（70%+ 命中率），渐变走方向识别将采样次数从 O(w×h) 降至 O(w) 或 O(h)，图像走格式交叉矩阵内联展开。

**脏矩形系统与切片器**（2.5-2.6 节）构成渲染调度层。DFS 控件树遍历收集脏区域 + 相邻合并启发式减少重绘碎片。矩形切片器用贪心列优先策略将大脏矩形切分为 PFB 可容纳的小片，除零除法外全为加减法操作。

**屏幕渲染管线**（2.7 节）展示了从输入事件到像素更新的完整时序。两级裁剪策略消除不可见控件的遍历和渲染开销。控件本地坐标系（Option C）将控件实现与其屏幕位置解耦。PFB 管理保证 32 位对齐以避免 ARM 未对齐访问异常。

### 重点知识回顾

1. **Porter-Duff Over 算子**：`Result = Fg × α + Bg × (255 - α) >> 8`。用 `>> 8`（/256）替代 `/ 255`，8 位精度下误差 ≤ ±1 人眼不可辨，每像素省 2~12 个除法周期。
2. **预乘 Alpha**：颜色通道预先乘 Alpha 存储。混合时省 3 次前景乘法，320×240 屏幕每帧省约 4.8ms。
3. **RGB565 Packed Blend**：在 packed 16 位空间内用通道掩码同时处理 R 和 B 通道的缩放，省去拆包/重组操作。
4. **Paint 分发器**：Tagged union（枚举 + union + switch），解耦 gfx 形状计算与 composite 颜色混合。
5. **像素格式函数表**：O(1) 派发到格式专属混合函数，与 `surf->pix_fmt` 直接索引。
6. **脏矩形 DFS 收集**：Z-order 遍历控件树，筛选带脏标记的控件的屏幕 AABB，相邻合并减少重绘次数。
7. **矩形切片器**：贪心列优先将大矩形切为 PFB 可容纳的行片，水平不切以保持 cache 友好。
8. **两级裁剪渲染**：脏矩形裁剪（AABB 求交）+ 父控件边界裁剪（累积 parent_clip），消除不可见部分的渲染开销。

### 思考问题

1. 预乘 Alpha 格式对完全透明的像素（α = 0）会丢失原始颜色信息。在什么场景下这可能成为问题？如何在需要时恢复原始颜色？
2. RGB565 packed blend 中，如果背景色与 mask 相乘后，R 通道溢出了其 5 位范围，`MASK_MUL_RB` 掩码如何保证溢出不会污染相邻的 G 通道？请用位运算符进行验证。
3. 方向优化的渐变快通道（2.4.2）在判断渐变方向时使用了 `abs(dx) > abs(dy)` 的阈值。如果渐变方向接近 45°（`abs(dx) ≈ abs(dy)`），此优化是否还能生效？是否需要考虑第三种"45°快通道"？
4. 矩形切片器的"贪心"策略是每片宽度使用脏矩形全宽。如果脏矩形是一个"L"形区域（如两个控件更新了但位置形成 L 形），合并后的大矩形的"贪心切片"会产生多少无效像素？如何改进？
5. 控件本地坐标系（Option C）的坐标平移公式为 `local = global - widget_abs_start`。在 DFS 渲染递归中，surf->color 指针始终指向 PFB 首字节。本地坐标下的 `(x - surf.surf.start.x)` 为何等价于全局坐标下的像素偏移？请用代数推导验证。

---

## 附录 A: 完整渲染管线数据流图

```
                     ┌──────────────────────────┐
                     │   用户输入 / 定时器事件    │
                     └────────────┬─────────────┘
                                  │
                     ┌────────────▼─────────────┐
                     │   控件回调更新状态          │
                     │   widget_set_dirty()      │
                     └────────────┬─────────────┘
                                  │
                     ┌────────────▼─────────────┐
                     │  脏矩形收集 (DFS)          │
                     │  dirty_rect_collect()     │
                     │  ├─ 遍历控件树             │
                     │  ├─ 计算全局 AABB          │
                     │  └─ 相邻合并               │
                     └────────────┬─────────────┘
                                  │
                     ┌────────────▼─────────────┐
                     │  矩形切片器                │
                     │  rect_slice()             │
                     │  └─ 贪心列优先切分         │
                     └────────────┬─────────────┘
                                  │
              ┌───────────────────┤
              │                   │
     ┌────────▼────────┐  ┌───────▼────────┐
     │    PFB Slice 1  │  │  PFB Slice N   │
     └────────┬────────┘  └───────┬────────┘
              │                   │
     ┌────────▼───────────────────▼────────┐
     │  DFS 渲染遍历 (每片独立)              │
     │  screen_render_widget_dfs()          │
     │  ├─ 裁剪求交 → 跳过不可见控件         │
     │  ├─ 坐标平移 → 控件本地坐标系         │
     │  ├─ widget->render()                 │
     │  │   ├─ gfx 模块: 遮罩生成           │
     │  │   │   ├─ edge_halfplane_mask     │
     │  │   │   ├─ ring_mask (SDF + LRU)   │
     │  │   │   └─ draw_primitive_*        │
     │  │   │       → mask[] + paint        │
     │  │   │                               │
     │  │   └─ composite 模块: 颜色合成     │
     │  │       ├─ ipgui_blend (分发器)     │
     │  │       │   ├─ blend_color          │
     │  │       │   │   └─ 函数表 + packed  │
     │  │       │   ├─ blend_gradient       │
     │  │       │   └─ blend_image          │
     │  │       │       → PFB 像素写入       │
     │  │                                   │
     │  ├─ DFS 递归子控件                   │
     │  └─ 子控件绘制覆盖父控件区域          │
     └────────────────┬────────────────────┘
                      │
             ┌────────▼────────┐
             │   Flush to LCD  │
             │   scr_drv->     │
             │   flush()       │
             └────────┬────────┘
                      │
             ┌────────▼────────┐
             │   屏幕像素更新    │
             └─────────────────┘
```

## 附录 B: 性能基准数据（参考值）

基于 STM32F429 (Cortex-M4 @ 168MHz)、320×240 RGB565 屏幕、64KB PFB 的参考性能数据：

| 操作 | 耗时 | 备注 |
|------|------|------|
| 纯色全屏填充 (无遮罩) | ~1.2ms | premult write 快路径 |
| 纯色全屏 + 反走样遮罩 | ~3.8ms | 每像素 PM alpha blend |
| 水平渐变全屏 | ~2.1ms | 方向优化：每行算 1 次色 |
| 倾斜渐变全屏 | ~12ms | 回退到逐像素采样 |
| Box 阴影 (blur=8) | ~2.5ms | SDF+1D 多项式 LUT |
| 100×100 PNG 图像混合 | ~1.8ms | 格式交叉矩阵 |
| DFS 控件树遍历 (30 控件) | ~0.3ms | 裁剪过滤 |
| 脏矩形收集 (5 脏区域) | ~0.2ms | 含相邻合并 |

---
