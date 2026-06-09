# 第二章 混合合成系统：从遮罩到像素

> 模块位置：`core/composite/` + `al/hal/`  
> 核心职责：接收 gfx 输出的 Mask + Paint → 逐像素混合 → 写入 Surface  
> 关键设计：Paint 分发器、Premultiplied Alpha、像素格式函数表、RGB565 Packed Blend、脏矩形系统、矩形切片器、屏幕渲染管线

---

## 2.1 架构概览

### 2.1.1 模块定位与职责分界

composite 模块在整个渲染管线中承担"颜色落笔"职责。它接收来自 gfx 模块的两样东西：

1. **Mask（遮罩）**：一个 `u8_t` 数组，每个字节 0-255，指示对应像素的"颜色透过权"
2. **Paint（涂料）**：一个 `ipgui_paint_t` 结构体，描述用于填充的颜色来源

composite 模块不关心遮罩来自直线还是圆圈——它只关心"在这个像素上，遮罩值是多少"。这种分工使得两个模块完全解耦：

```
gfx 模块输出: mask[] ──────────────┐
                                   ├──→ ipgui_blend() → 像素颜色写入 surf
gfx 模块输出: paint ───────────────┘
```

### 2.1.2 Paint 分发器

`ipgui_paint_t` 是 gfx 和 composite 之间的核心桥梁。它使用 tagged union 设计：

```c
// ── 源文件: core/composite/ipgui_blend.h ──
typedef enum {
    IPGUI_PAINT_COLOR,    // 纯色
    IPGUI_PAINT_GRADIENT, // 渐变
    IPGUI_PAINT_IMAGE,    // 图像
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

分发函数 `ipgui_blend()` 仅做类型判别和参数传递，三个分支完全对称：

```c
// ── 源文件: core/composite/ipgui_blend.c ──
void ipgui_blend(ipgui_surf_t * surf, ipgui_aabb_t * clip,
    ipgui_aabb_t * dest, ipgui_paint_t * paint, u8_t opacity,
    u8_t * mask, ipgui_aabb_t * mask_aabb, ipgui_blend_mode_t blend_mode)
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
    }
}
```

分发器的价值在于隔离性：如果要新增一种涂料类型，只需在枚举中添加一个值、在 `union` 中添加对应成员、在此函数中添加一个 case 分支即可。所有调用 `ipgui_blend()` 的 gfx 层代码无需修改。

### 2.1.3 composite 模块的文件结构

```
core/composite/
├── ipgui_blend.c/h                    # Paint 分发器
├── blend_color/ipgui_blend_color.c/h  # 纯色混合
├── blend_gradient/ipgui_blend_gradient_color.c/h  # 渐变混合
├── blend_image/ipgui_blend_image.c/h  # 图像混合

core/ui/widget_manager/
└── ipgui_dirty_rect.c/h              # 脏矩形管理器

al/hal/
├── ipgui_screen.c/h                   # 屏幕渲染主入口 + DFS 控件遍历
└── ipgui_rect_slice.c/h              # 脏矩形切片器
```

注意脏矩形管理器和屏幕渲染入口虽然不直接属于 `core/composite`，但它们是 composite 完整渲染管线的必要组成部分。本章从完整的渲染链路视角将二者纳入讨论。

---

## 2.2 Alpha 混合的数学基础

### 2.2.1 Porter-Duff Over 算子

Porter 和 Duff 在 1984 年推导的 alpha 合成公式是计算机图形学的基础。最重要的算子"over"——将前景（源 S）叠加在背景（目标 D）上——的公式为：

```
α_result = α_s + α_d × (1 - α_s)
C_result = (C_s × α_s + C_d × α_d × (1 - α_s)) / α_result
```

在 8 位整数域（0-255）中，系统使用以下等价形式：

```
α_combined = (opacity × mask[p]) >> 8
C_result = (C_s × α_combined + C_d × (255 - α_combined)) >> 8
```

### 2.2.2 Standard Alpha vs Premultiplied Alpha

Standard Alpha 格式中，颜色通道存储未与 alpha 相乘的原始值。混合时每次需要三次乘法（`R_s × α`、`G_s × α`、`B_s × α`）：

```
R = (R_s × α + R_d × (255 - α)) >> 8
G = (G_s × α + G_d × (255 - α)) >> 8
B = (B_s × α + B_d × (255 - α)) >> 8
```

Premultiplied Alpha 格式在存储时就预先将颜色通道与 alpha 相乘。混合时第一项直接使用存储值：

```
R = R_s_pm + ((R_d × (255 - α)) >> 8)
G = G_s_pm + ((G_d × (255 - α)) >> 8)
B = B_s_pm + ((B_d × (255 - α)) >> 8)
```

每个像素节省 3 次 8 位乘法。对于 320×240 的屏幕，一帧节省约 230,400 次乘法。在 48MHz 的 Cortex-M 处理器上，这节省约 5ms——在 60fps 的帧预算（16.6ms）中占比约 30%。

```c
// ── 源文件: core/composite/blend_color/ipgui_blend_color.c:729-750 ──
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

使用 `>> 8` 而非 `/ 255` 是嵌入式环境的关键优化。位移操作在无硬件除法器的 MCU 上比除法快一个数量级，且 256 vs 255 的差异在 8 位精度下误差仅为 ±1。

### 2.2.3 颜色透明度组合函数

```c
// ── 源文件: core/composite/blend_color/ipgui_blend_color.c:587-598 ──
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

此函数将外部 opacity（用户设置的全局透明度）与颜色的固有 alpha 通道相乘，生成新的颜色值。RGB 通道不变（保持 standard alpha 状态），只有 A 通道被缩放。真正的预乘在随后的 `ipgui_color_premultiply()` 中完成。

---

## 2.3 像素格式函数表

### 2.3.1 表驱动混合器选择

不同像素格式有不同的通道排布和位数。composite 模块通过一张全局函数表消除格式差异：

```c
// ── 源文件: core/composite/blend_color/ipgui_blend_color.c:579-587 ──
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

表索引使用 `surf->pix_fmt`，O(1) 派发。未使用的格式槽位为 NULL，调用前通常有 NULL 检查。

与之平行的是 `solid_conv_table`（纯色转换表）：

```c
solid_convert_func_t solid_conv_table[PIX_FMT_MAX] = {
    [PIX_FMT_RGB565]   = ipgui_solid_color_2_rgb565,
    [PIX_FMT_BGR565]   = ipgui_solid_color_2_bgr565,
    // ...
};
```

### 2.3.2 端序感知的 32 位访问

所有 32 位像素格式的 blend 函数都使用 `#if IPGUI_ENDIAN_LITTLE == 1` 做编译期端序分支：

```c
// ── 源文件: core/composite/blend_color/ipgui_blend_color.c:211-255 ──
void ipgui_builtin_premultiplied_color_blend_to_argb8888(
    ipgui_color_t color, u8_t * argb8888, ipgui_blend_mode_t blend_mode)
{
    u32_t cr = *(u32_t *)argb8888;  // 一次 32 位读获取整个像素
    u32_t alpha, r, g, b;

#if IPGUI_ENDIAN_LITTLE == 1
    // 小端序内存布局 [A][R][G][B] → cr = 0xBBGGRRAA
    alpha =  cr        & 0xff;
    r     = (cr >>  8) & 0xff;
    g     = (cr >> 16) & 0xff;
    b     = (cr >> 24) & 0xff;
#else
    // 大端序内存布局 [A][R][G][B] → cr = 0xAARRGGBB
    alpha = (cr >> 24) & 0xff;
    r     = (cr >> 16) & 0xff;
    g     = (cr >>  8) & 0xff;
    b     =  cr        & 0xff;
#endif
    // ... 混合计算后写回
}
```

关键：`#if` 是编译期常量条件，编译后仅保留目标平台的对应分支代码，零运行时开销。32 位一次性读写（`*(u32_t *)argb8888`）比逐字节读写减少 3/4 的内存访问次数。

### 2.3.3 ARGB8888 的复合 Alpha 与倒数查表

32 位格式（ARGB8888 等）的背景自带 alpha 通道，意味着可能存在"多层半透明叠加"。两层叠加后的 alpha 值为：

```
α_12 = 1 - (1 - α_1) × (1 - α_2) = α_1 + α_2 - α_1 × α_2
```

在整数域中直接计算 `α_1 × α_2` 可能溢出。系统使用等价的不溢出形式：

```c
u32_t alpha12 = 255 - (((255 - alpha) * ialpha2) >> 8);
```

`(255 - alpha)` 最大为 255，`ialpha2` 最大为 255，乘积最大 65025，32 位整数完全可以容纳。

组合后的 premultiplied 颜色需要除以新的复合 alpha 恢复为 straight alpha：

```c
#if USE_INV_TABLE == 1
    r12 = (r12 * g_inv_tbl[alpha12]) >> 8;  // 查表替代除法
#else
    r12 = alpha12 ? (r12 << 8) / alpha12 : 0;  // 运行时除法
#endif
```

`g_inv_tbl` 是倒数查表，存储 `255 × 255 / alpha` 的预计算值（共 256 项，每项 2 字节，共 512 字节 ROM）：

```c
const u16_t g_inv_tbl[256] = {
    0, 65025, 32512, 21675, 16256, 13005, 10837, 9289,
    8128, 7225, 6502, 5911, 5418, 5001, 4644, 4335,
    // ... 共 256 项
    256, 255,
};
```

查表将除法转换为乘法 + 移位：`value × table[alpha] >> 8`——在无硬件除法器的 MCU 上快 10-20 倍。

---

## 2.4 RGB565 Packed Blend 详解

### 2.4.1 像素格式的位布局

RGB565 是嵌入式 LCD 控制器最常用的 16 位格式：

```
位:  15 14 13 12 11  10  9  8  7  6  5   4  3  2  1  0
     [ R4 R3 R2 R1 R0 ][ G5 G4 G3 G2 G1 G0 ][ B4 B3 B2 B1 B0 ]
       ─────红色 5 位─────  ─────绿色 6 位─────  ─────蓝色 5 位─────
```

红色和蓝色各 5 位（32 个灰度级），绿色 6 位（64 个灰度级）。绿色多 1 位是因为人眼对绿色波长的亮度感知最敏感（视觉亮度公式中绿色贡献约 59%）。

BGR565 是变体——红色和蓝色位置互换：

```
RGB565: [RRRRR][GGGGGG][BBBBB]
BGR565: [BBBBB][GGGGGG][RRRRR]
```

### 2.4.2 Packed 空间的直接混合

RGB565 混合的核心创新是不拆包——直接在 16 位 packed 空间完成背景的 alpha 缩放。精要在于三个位掩码：

```c
// ── 源文件: core/composite/blend_color/ipgui_blend_color.c:8-11 ──
#define MASK_RB     0xf81fU   // 二进制: 11111 000000 11111
#define MASK_G      0x07e0U   // 二进制: 00000 111111 00000
#define MASK_MUL_RB 0x3e07c0U // MASK_RB << 6，用于剪切乘法后的溢出
#define MASK_MUL_G  0x1f800U  // MASK_G  << 6
```

完整的 4 步算法（小端版本）：

```c
// ── 源文件: core/composite/blend_color/ipgui_blend_color.c:51-67 ──
void ipgui_builtin_premultiplied_color_blend_to_rgb565(
    ipgui_color_t color, u8_t * rgb565, ipgui_blend_mode_t blend_mode)
{
    // 步骤 1：alpha 从 8 位缩放到 6 位（0-64）
    u32_t ialpha6 = (255 - IPGUI_COLOR_A(color) + 2) >> 2;
    u32_t bg = *(u16_t *)rgb565;

    // 步骤 2：在 packed 空间缩放背景通道
    u32_t bg_rb = ((ialpha6 * (bg & MASK_RB)) & MASK_MUL_RB) >> 6;
    u32_t bg_g  = ((ialpha6 * (bg & MASK_G )) & MASK_MUL_G ) >> 6;

    // 步骤 3：将前景色转换为 RGB565 packed 格式
    u32_t fg565 = ((u32_t)(IPGUI_COLOR_R(color) >> 3) << 11)
                | ((u32_t)(IPGUI_COLOR_G(color) >> 2) <<  5)
                | ((u32_t)(IPGUI_COLOR_B(color) >> 3)      );

    // 步骤 4：packed 空间相加（premultiplied 保证不溢出）
    *(u16_t *)rgb565 = (u16_t)(
        ((fg565 & MASK_RB) + bg_rb) | ((fg565 & MASK_G) + bg_g)
    );
}
```

### 2.4.3 位隔离原理

理解这个优化需要从二进制角度解析 packed 数据的结构。`bg & MASK_RB` 的结果保留 R 和 B 通道的原始位位置，G 通道归零：

```
bg & MASK_RB =    RRRRR 000000 BBBBB   （绿色位被清零）
× ialpha6 后:
结果在 [31:0]:    RRRRR_RRRRRR_000000_BBBBB_BBBBBB

用 & MASK_MUL_RB 剪切:
MASK_MUL_RB  =    00000_1111100_000000_000011111_000000
                  ^^^^^^^^^^^^^^^       ^^^^^^^^^^^^^
                  保留扩展后的R(10位)    保留扩展后的B(10位)

最后 >> 6: 恢复到原始的 [RRRRR][000000][BBBBB] 位宽
```

R 和 B 通道被中间的 6 位 G 区域隔绝，乘法结果的溢出不会跨通道污染。如果 RGB565 的位布局是连续排列（如 `[RRRRR][GGGGGGG][BBBBB]` 无间隔），这种优化就无法进行——因为 R 的溢出位会直接覆盖 G 的高位。这里利用的是 RGB565 标准的天然通道间隙。

### 2.4.4 Alpha 8→6 位缩放

`ialpha6 = (255 - alpha + 2) >> 2` 中的 `+2` 用于四舍五入。因为 `>> 2` 直接截断为向下舍入，加上 `(4/2) = 2` 后实现向最近整数舍入。例如 `(255 - 3) = 252`，`252 >> 2 = 63`；`(255 - 3 + 2) >> 2 = 254 >> 2 = 63`，舍入结果不变。但 `(255 - 1 + 2) >> 2 = 256 >> 2 = 64`（原本 `254 >> 2 = 63.5`，向最近整数舍入为 64）。

---

## 2.5 渐变填充的混合优化

### 2.5.1 方向感知的快速通道

在 `ipgui_fill_gradient_color()` 和 `ipgui_blend_gradient_color()` 中，进入逐像素循环前先检测渐变的水平/垂直方向：

```c
// ── 源文件: core/composite/blend_gradient/ipgui_blend_gradient_color.c:73-89 ──
if (ipgui_if_liner_gradient_hor(&gradient->grad.liner_grad)) {
    // 水平渐变：每列颜色相同 → 每列计算一次，整列填充
    band.start.x = blend_aabb.start.x;
    band.end.x = band.start.x;  // 宽度为 1 像素
    band.start.y = blend_aabb.start.y;
    band.end.y = blend_aabb.end.y;
    for (; band.start.x <= blend_aabb.end.x; band.start.x++) {
        ipgui_liner_gradient_color_get(&gradient->grad.liner_grad,
            ipgui_get_liner_gradient_pos_at_xy(
                &gradient->grad.liner_grad, band.start.x, band.start.y), &cr);
        ipgui_fill_color(surf, NULL, &band, cr, opacity, blend_mode);
    }
    return;
}
```

水平渐变下，每一列（x 固定）的所有像素颜色相同。原 O(w×h) 的逐像素计算降为 O(w) 的逐列计算——每次整列传递一个 1×h 的矩形给 `ipgui_fill_color()`，由纯色填充路径处理内部的逐像素混合。

### 2.5.2 带遮罩的渐变混合快速通道

当同时有渐变和遮罩时，水平渐变的优化逻辑略有不同——不能简单地调用 `ipgui_fill_color()`（纯色填充不认遮罩），而需调用 `ipgui_blend_color()`：

```c
// ── 源文件: core/composite/blend_gradient/ipgui_blend_gradient_color.c:190-210 ──
if (ipgui_if_liner_gradient_hor(&gradient->grad.liner_grad)) {
    // 水平渐变 + mask：每列颜色相同 → 每列调用一次 ipgui_blend_color
    band.start.x = blend_aabb.start.x;
    band.end.x = band.start.x;
    for (; band.start.x <= blend_aabb.end.x; band.start.x++) {
        ipgui_liner_gradient_color_get(&gradient->grad.liner_grad,
            ipgui_get_liner_gradient_pos_at_xy(...), &cr);
        ipgui_blend_color(surf, NULL, &band,
            cr, opacity, mask, mask_aabb, blend_mode);
    }
    return;
}
```

### 2.5.3 逐像素路径

对于非水平/垂直的渐变（对角、径向、锥形），引擎走逐像素路径：

```c
// ── 源文件: core/composite/blend_gradient/ipgui_blend_gradient_color.c:119-140 ──
for (y = 0; y < y_span; y++) {
    row_pix_off = 0;
    for (x = 0; x < x_span; x++) {
        pos = ipgui_gradient_pos_at_xy(gradient, abs_x0 + x, abs_y0 + y);
        ipgui_gradient_color_get(gradient, pos, &grad_cr);
        premult = ipgui_color_combine_opacity_and_premultiply(
            &grad_cr, opacity);
        if (IPGUI_COLOR_A(premult) > 2) {
            blend_fn(premult, &dest_cr_buf[row_pix_off], blend_mode);
        }
        row_pix_off += pix_size;
    }
    dest_cr_buf += stride;
}
```

逐像素路径的性能关键点：

1. **阈值跳过**：`IPGUI_COLOR_A(premult) > 2` 过滤alpha极低的像素（在 8 位精度下视觉上不可见），避免无意义的 blend 操作
2. **指针行跳**：`dest_cr_buf += stride` 而非每次重算地址
3. **绝对坐标**：`abs_x0 + x` 而非相对坐标，因为渐变位置计算依赖全局坐标（渐变可能跨越多个控件的区域）

### 2.5.4 位置与颜色获取的类型派发

渐变类型（线性/径向/锥形）的派发在两个维度各做一次：

```c
// ── 源文件: core/composite/blend_gradient/ipgui_blend_gradient_color.c ──
__IPGUI_STATIC__ __IPGUI_INLINE__ u8_t ipgui_gradient_pos_at_xy(
    ipgui_grad_src_t * src, ipgui_coord_t x, ipgui_coord_t y)
{
    if (src->grad_type == IPGUI_GRADIENT_TYPE_LINEAR)
        return ipgui_get_liner_gradient_pos_at_xy(&src->grad.liner_grad, x, y);
    else if (src->grad_type == IPGUI_GRADIENT_TYPE_RADIAL)
        return ipgui_get_radial_gradient_pos_at_xy(&src->grad.radial_grad, x, y);
    else if (src->grad_type == IPGUI_GRADIENT_TYPE_CONIC)
        return ipgui_get_conic_gradient_pos_at_xy(&src->grad.conic_grad, x, y);
    else return 0;
}
```

---

## 2.6 图像合成的全像素格式矩阵

### 2.6.1 问题规模

图像合成面临的格式组合远多于纯色混合。以当前版本为例：

- **源图像格式**（10 种）：L8、LA88、RGB565、BGR565、RGB888、BGR888、ARGB8888、ABGR8888、RGBA8888、BGRA8888
- **目标像素格式**（8 种）：同混色表中的 8 种
- **总计组合数** = 10 × 8 = 80

每种组合对应一个独立的 blend 函数。

### 2.6.2 层次化编排

`ipgui_blend_image.c` 按源格式组织 blend 函数：

```
blend_l8     → 8 个目标格式函数
blend_la88   → 8 个目标格式函数（委托 L8）
blend_rgb565 → 8 个目标格式函数
blend_bgr565 → 8 个目标格式函数
...
```

### 2.6.3 LA88 的复合 Alpha 委托

LA88（16 位：亮度 L + Alpha A）是将 Alpha 从图像自带的 alpha 通道（`src[1]`）与外部传入的 alpha 结合后，委托给对应的 L8 blend 函数：

```c
// ── 源文件: core/composite/blend_image/ipgui_blend_image.c ──
// LA88 → RGB565:
u8_t combined_a = (u8_t)((u32_t)src[1] * alpha >> 8);
blend_l8_2_rgb565(src, dst, combined_a, blend_mode);
```

这种委托避免了为每个 LA88 函数重新实现完整的 RGB 混合逻辑。

### 2.6.4 跨格式的 RGB 通道重组

当源格式为 RGB565 而目标为 BGR565（或反之），需要进行通道重组。引擎选择了"先拆包、重组、再打包"的策略：

```c
// ── 源文件: core/composite/blend_image/ipgui_blend_image.c ──
// rgb565 → bgr565:
u8_t r = (fg >> 11) << 3;          // 5-bit R → 8-bit
u8_t g = ((fg >> 5) & 0x3F) << 2;  // 6-bit G → 8-bit
u8_t b = (fg & 0x1F) << 3;         // 5-bit B → 8-bit
// 重组为 BGR565:
u32_t fg_bgr = ((u32_t)(b >> 3) << 11)
             | ((u32_t)(g >> 2) << 5)
             | (u32_t)(r >> 3);
```

这种"拆包→重组→打包"策略虽然多了一次格式转换，但避免了维护 16（4×4）种 RGB565 系列间的交叉组合函数。

### 2.6.5 同格式的 Packed 直接混合

当源格式与目标格式相同（如 RGB565→RGB565），可以直接在 packed 空间做整体混合：

```c
// rgb565 → rgb565:
u32_t a6 = (alpha + 2) >> 2; u32_t ia6 = 64 - a6;
u32_t fg = *(u16_t *)src; u32_t bg = *(u16_t *)dst;
u32_t out_rb = (((fg & MASK_RB) * a6) >> 6) + (((bg & MASK_RB) * ia6) >> 6);
u32_t out_g  = (((fg & MASK_G)  * a6) >> 6) + (((bg & MASK_G)  * ia6) >> 6);
*(u16_t *)dst = (out_rb & MASK_RB) | (out_g & MASK_G);
```

这比纯色的 premultiplied blend 又多了一步：前景也需要用 alpha 缩放——因为图像源的颜色值通常是 straight alpha。公式为：

```
result = fg × α + bg × (1 - α)
```

---

## 2.7 脏矩形系统

### 2.7.1 设计动机

屏幕渲染的最高开销是"绘制不需要更新的区域"。如果一个按钮的悬停仅改变了颜色，理论上只需重新绘制该按钮的矩形区域，而非整个屏幕。

脏矩形（dirty rectangle）系统实现了这一目标：每次 UI 状态变化时，记录需要重绘的区域；渲染时只遍历这些区域。

### 2.7.2 数据结构

```c
// ── 源文件: core/ui/widget_manager/ipgui_dirty_rect.h ──
#ifndef IPGUI_DIRTY_RECT_POOL
#define IPGUI_DIRTY_RECT_POOL  8  // 脏矩形池的最大容量
#endif

#ifndef IPGUI_MERGE_COST_THRESHOLD
#define IPGUI_MERGE_COST_THRESHOLD  0  // 合并代价阈值
#endif

typedef struct {
    ipgui_coord_t x1, y1, x2, y2;
} ipgui_dirty_rect_t;

typedef struct {
    ipgui_dirty_rect_t pool[IPGUI_DIRTY_RECT_POOL];
    s32_t pool_num;
} ipgui_dirty_rect_mgr_t;
```

`IPGUI_DIRTY_RECT_POOL` 默认 8，意味着同时最多追踪 8 个不重叠的脏区域。这是嵌入式环境下的实用限制——超过 8 个时进行强制合并。

### 2.7.3 两阶段合并策略

系统将脏矩形管理分为"积累"和"刷新"两个阶段：

**阶段 1（积累——每个 UI 操作调用）**：

`ipgui_dirty_rect_add()` 在每次控件状态变化时被调用。新脏矩形的添加逻辑为：

```c
// ── 源文件: core/ui/widget_manager/ipgui_dirty_rect.c ──
__IPGUI_STATIC__ void pool_add(ipgui_dirty_rect_mgr_t * mgr,
                                 ipgui_dirty_rect_t dr)
{
    // 1. 检查 dr 是否被已有矩形完全包含 → 丢弃 dr
    // 2. 检查 dr 是否完全包含已有矩形 → 删除被包含的矩形
    // 3. 如果池中有空位 → 直接插入
    // 4. 如果池已满 → 调用 arr_merge_best_pair() 找最优合并对
}
```

`arr_merge_best_pair()` 的核心逻辑是找到"合并后面积增长最小"的两个矩形对。它分两步：

```c
// 步骤 1：在所有已有矩形对 (C(n,2) = n(n-1)/2 对) 中找最小代价对
for (s32_t i = 0; i < *num; i++)
    for (s32_t j = i + 1; j < *num; j++) {
        cost = dirty_rect_merge_cost(&arr[i], &arr[j]);
        if (cost < best_cost) { best_cost = cost; best_i = i; best_j = j; }
    }

// 步骤 2：在已有矩形与新矩形的对 (n 对) 中找最小代价对
for (s32_t i = 0; i < *num; i++) {
    cost = dirty_rect_merge_cost(&arr[i], new_dr);
    if (cost < best_cost) { best_cost = cost; best_i = i; }
}

// 比较步骤 1 和步骤 2 的最优结果，执行代价更小的合并
```

合并代价的定义：

```c
// 代价 = 合并后面积 - 面积_a - 面积_b
// 负值：合并后面积小于两矩形之和（有重叠，好）
// 正值：合并后面积大于两矩形之和（有空隙，差）
// 零值：合并后面积等于两矩形之和（恰好相邻）
```

**阶段 2（刷新——每帧渲染前调用一次）**：

`ipgui_dirty_rect_flush()` 做全局最优合并：

```c
// ── 源文件: core/ui/widget_manager/ipgui_dirty_rect.c ──
__IPGUI_API__ void ipgui_dirty_rect_flush(ipgui_dirty_rect_mgr_t * mgr)
{
    s32_t improved = 1;
    while (improved && mgr->pool_num > 1) {
        improved = 0;
        // 在所有矩形对中找代价 ≤ IPGUI_MERGE_COST_THRESHOLD 的最小代价对
        for (s32_t i = 0; i < mgr->pool_num; i++)
            for (s32_t j = i + 1; j < mgr->pool_num; j++) {
                cost = dirty_rect_merge_cost(&pool[i], &pool[j]);
                if (cost <= IPGUI_MERGE_COST_THRESHOLD && cost <= best_cost) {
                    best_cost = cost; best_i = i; best_j = j;
                }
            }
        if (best_i >= 0) {
            // 合并这对矩形，继续迭代
            pool[best_i] = dirty_rect_merge(&pool[best_i], &pool[best_j]);
            arr_remove(pool, &mgr->pool_num, best_j);
            improved = 1;
        }
    }
}
```

该算法的迭代性质保证了局部最优——只要还有一对矩形的合并不超过阈值，就继续合并。当任意两对合并的代价都超过阈值时停止。

---

## 2.8 矩形切片器

### 2.8.1 问题定义

在内存极度受限的嵌入式环境中，不可能为整个脏矩形一次性分配帧缓冲区。例如，一个 240×320 的脏矩形在 RGB565 格式下需要 153.6KB——这已经超过了许多 MCU 的 SRAM 总量。

方案是将脏矩形切分为多个小块（slice），逐个渲染。矩形切片器的职责就是执行这种切分。

### 2.8.2 贪心列优先策略

```c
// ── 源文件: al/hal/ipgui_rect_slice.h ──
typedef struct {
    ipgui_rect_t * rect;
    ipgui_coord_t remain_w, remain_h;
    ipgui_coord_t slice_len;         // 单片最大像素数
    ipgui_coord_t full_w, full_h;
    ipgui_coord_t orig_start_x, orig_end_x, orig_bottom;
    ipgui_coord_t rows_when_narrow;   // 窄列的预计算行数
} ipgui_rect_slice_ctx;
```

每次调用 `ipgui_get_rect_slice()` 返回一条水平条带，其策略是：

1. **列宽决策**：`strip_w = MIN(remain_w, slice_len)`——像素预算允许取多少宽度就取多少
2. **行数决策**：`rows = MIN(remain_h, slice_len / strip_w)`——在当前列内，像素预算允许塞多少行

遍历策略：列优先，从左到右推进列；每列内部从上到下输出条带。当一列耗尽后，向右前进到下一列继续。

### 2.8.3 除法消除优化

原本每次调用 `ipgui_get_rect_slice()` 都需要一次整数除法 `rows = slice_len / strip_w`。通过数学分析可以发现：

`strip_w` 在整个矩形遍历中仅取两个值：
- `strip_w == slice_len`（常规条带，占 >99% 的调用）：此时 `rows = slice_len / slice_len = 1`，除法结果恒为 1
- `strip_w == full_w % slice_len`（末尾窄列，占 0~2 次调用）：一个每矩形恒定的值

据此，系统在初始化时预计算 `rows_when_narrow`，在热路径中用分支替代除法：

```c
// ── 源文件: al/hal/ipgui_rect_slice.c ──
// 初始化：预计算窄列的行数
ipgui_coord_t remainder_w = ctx->full_w % ctx->slice_len;
if (remainder_w > 0)
    ctx->rows_when_narrow = ctx->slice_len / remainder_w;  // 仅此一次除法
else
    ctx->rows_when_narrow = 0;  // 无窄列，标记跳过分支

// 热路径：分支替代除法
if (strip_w == ctx->slice_len) {
    rows = 1;  // 热路径：常量折叠，零开销
} else {
    rows = IPGUI_MIN(ctx->remain_h, ctx->rows_when_narrow);  // 冷路径
}
```

对于 100×100 的矩形、slice_len=99：原需 102 次除法，优化后仅需 2 次（初始化时的 1 次求余 + 1 次除法）。除法减少 98%。

这个优化利用了两个关键数学不变式：
- `strip_w` 在绝大多数迭代中等于 `slice_len`
- 当 `strip_w != slice_len` 时，`strip_w` 恒等于 `full_w % slice_len`，因此 `rows_when_narrow` 在初始化时即可确定

分支 `strip_w == slice_len` 是 CPU 分支预测器 100% 可预测的——99%+ 的迭代走相同路径，预测正确率接近 100%。

### 2.8.4 贪心 vs 全局最优：工程分析

源码注释中包含贪心与全局最优方案的详细对比分析：

| 比较维度 | 贪心策略 | 全局规划 |
|---------|---------|---------|
| 初始化时间 | O(1)：1 次取模 + 1 次除法 | O(full_w × d(slice_len)) 次除法 |
| 单次调用时间 | O(1)：比较 + 赋值 | O(1)，但列数更多 |
| 代码量 | ~15 行 | 50+ 行，含嵌套循环枚举 |

在实际脏矩形渲染场景中，宽矩形（`full_w >= slice_len`）是绝大多数，贪心策略下天然 1 列——已是最优。窄矩形虽然可能差 15%-30%，但其像素总量小，绝对成本微不足道。

---

## 2.9 屏幕渲染完整管线

### 2.9.1 数据结构

```c
// ── 源文件: al/hal/ipgui_screen.h ──
typedef struct {
    void            * pri_data;
    ipgui_coord_t     xreso, yreso;  // 屏幕分辨率
    ipgui_pix_fmt_t   pix_fmt;

    void (* put_pixel)(ipgui_scr_t * scr,
        ipgui_coord_t x, ipgui_coord_t y, u8_t * pix);
    void (* fill_region)(ipgui_scr_t * scr,
        ipgui_coord_t x1, ipgui_coord_t y1,
        ipgui_coord_t x2, ipgui_coord_t y2,
        u8_t * pix_buf, s32_t stride);
    void (* flush)(ipgui_scr_t * scr);
} ipgui_scr_drv_t;

typedef struct ipgui_scr_ctx {
    void                 * pri_data;
    ipgui_scr_drv_t      * drv;
    struct widget_tree_t   tree;
    ipgui_dirty_rect_mgr_t dirty;
    ipgui_pfb_t            pfb;      // 部分帧缓冲区
} ipgui_scr_t;
```

`ipgui_scr_drv_t` 提供了三个硬件抽象接口：`put_pixel`、`fill_region`、`flush`。任何支持这三个接口的 LCD 驱动都可以接入系统，无需修改上层代码。

### 2.9.2 PFB（Partial Frame Buffer）的创建

```c
// ── 源文件: al/hal/ipgui_screen.c ──
ipgui_err_t ipgui_scr_create_pfb(ipgui_scr_t * scr,
    u8_t * buf, u32_t buf_size, ipgui_pix_fmt_t pix_fmt)
{
    // 将 buf 对齐到 4 字节边界
    u8_t * pfb_buf = IPGUI_ALIGN_U32(buf);
    u32_t valid_size = (buf + buf_size) - pfb_buf;

    // 根据像素格式计算每像素字节数
    u8_t px_size = 0;
    switch (pix_fmt) {
        case PIX_FMT_RGB565:
        case PIX_FMT_BGR565:   px_size = 2; break;
        case PIX_FMT_RGB888:
        case PIX_FMT_BGR888:   px_size = 4; break;
        case PIX_FMT_RGBA8888:
        case PIX_FMT_BGRA8888: px_size = 4; break;
    }

    scr->pfb.num_pixs = valid_size / px_size;
    scr->pfb.color    = pfb_buf;
    scr->pfb.pix_fmt  = pix_fmt;
    scr->pfb.pix_size = px_size;
}
```

`IPGUI_ALIGN_U32(buf)` 保证缓冲区对齐到 4 字节边界——32 位像素格式需要对齐访问才能保证单指令读写的正确性和性能。

`num_pixs` 是 PFB 的总像素数，被用作 `ipgui_rect_slice_ctx_init()` 的 `slice_len` 参数——即每次切片不超过 PFB 的总像素数。

### 2.9.3 帧渲染主循环

```c
// ── 源文件: al/hal/ipgui_screen.c ──
__IPGUI_API__ void ipgui_screen_render(ipgui_scr_t * scr)
{
    if (scr->dirty.pool_num == 0) return;  // 快速返回：无脏区域

    ipgui_dirty_rect_flush(&scr->dirty);    // 全局最优合并

    for (s32_t idx = 0; idx < scr->dirty.pool_num; idx++) {
        ipgui_dirty_rect_t * dirty = &scr->dirty.pool[idx];
        ipgui_screen_render_dirty_rect(scr, dirty);  // 逐个渲染
    }

    ipgui_dirty_rect_mgr_reset(&scr->dirty);  // 重置脏矩形管理器
}
```

函数入口处的 `if (scr->dirty.pool_num == 0) return;` 是一个关键的性能优化。当屏幕没有变化时（静止画面），这个条件判断使得整个渲染管线在 O(1) 时间内返回，不浪费任何 CPU 周期。

### 2.9.4 单个脏矩形的渲染

```c
__IPGUI_STATIC__ void ipgui_screen_render_dirty_rect(
    ipgui_scr_t * scr, ipgui_dirty_rect_t * dirty)
{
    ipgui_aabb_t _dirty = {
        .start = {.x = dirty->x1, .y = dirty->y1},
        .end   = {.x = dirty->x2, .y = dirty->y2}
    };

    // 切分为多个小块，逐个渲染
    ipgui_rect_slice_ctx slice_ctx;
    ipgui_rect_slice_ctx_init(&slice_ctx, &_dirty, scr->pfb.num_pixs);

    ipgui_aabb_t slice_rect;
    while (ipgui_get_rect_slice(&slice_ctx, &slice_rect)) {
        ipgui_screen_render_dirty_rect_slice(scr, &scr->pfb, &slice_rect);
    }
}
```

### 2.9.5 单片渲染与表面构造

```c
__IPGUI_STATIC__ void ipgui_screen_render_dirty_rect_slice(
    ipgui_scr_t * scr, ipgui_pfb_t * pfb, ipgui_aabb_t * dirty)
{
    // 清空 PFB
    ipgui_memset(pfb->color, 0,
        (u32_t)ipgui_aabb_width(dirty) * ipgui_aabb_height(dirty) * pfb->pix_size);

    // 构造 surf：零拷贝
    ipgui_surf_t surf;
    surf.surf    = *dirty;  // 全局坐标
    surf.color   = pfb->color;
    surf.stride  = (u32_t)ipgui_aabb_width(dirty) * pfb->pix_size;
    surf.pix_fmt = pfb->pix_fmt;
    surf.pix_size = pfb->pix_size;

    // 构造渲染上下文并遍历控件树
    ipgui_widget_render_ctx_t render_ctx;
    render_ctx.surf        = &surf;
    render_ctx.clip        = dirty;
    render_ctx.parent_clip = (ipgui_aabb_t *)0;

    ipgui_screen_render_widget_dfs(&scr->tree.root, &render_ctx);
}
```

这里 `surf.color = pfb->color` 实现了零拷贝表面构造——surf 的描述符直接指向 PFB 缓冲区，不需要额外的内存分配或复制。在嵌入式环境中，零拷贝意味着零 malloc 和零 memcpy。

### 2.9.6 DFS 控件树遍历与两级裁剪

控件的渲染使用先序遍历（DFS），保证父控件先于子控件绘制（Z-order）：

```c
// ── 源文件: al/hal/ipgui_screen.c ──
__IPGUI_STATIC__ void ipgui_screen_render_widget_dfs(
    struct widget_link_t * link, ipgui_widget_render_ctx_t * ctx)
{
    struct widget_link_t ** child = &link->first_child;
    while (*child) {
        widget = ipgui_container_of(*child, ipgui_widget_t, link);

        // 步骤 1：检查可见性，跳过 INVISIBLE 控件
        if (widget->flags & IPGUI_WIDGET_FLAG_INVISIBLE) {
            child = &((*child)->sib_next);
            continue;
        }

        // 步骤 2：计算全局 AABB
        ipgui_aabb_t global_aabb;
        ipgui_widget_abs_pos(widget, &global_aabb);

        // 步骤 3：与 clip（脏矩形切片）求交
        ipgui_aabb_t intersect;
        if (0 != ipgui_aabb_overlap(&intersect, &global_aabb, ctx->clip)) {
            child = &((*child)->sib_next);  // 完全不可见，跳过整棵子树
            continue;
        }

        // 步骤 4：与 parent_clip（父控件边界）求交
        if (ctx->parent_clip) {
            ipgui_aabb_t clipped;
            if (0 != ipgui_aabb_overlap(&clipped, &intersect, ctx->parent_clip)) {
                child = &((*child)->sib_next);  // 完全被裁剪，跳过
                continue;
            }
            intersect = clipped;
        }

        // 步骤 5：调用控件的 render 回调
        if (widget->render) widget->render(widget, &child_ctx);

        // 步骤 6：递归渲染子控件
        if ((*child)->first_child) {
            ipgui_screen_render_widget_dfs(*child, &sub_ctx);
        }

        child = &((*child)->sib_next);
    }
}
```

两级裁剪策略：

1. **第一级（脏矩形裁剪）**：控件的全局 AABB 与脏矩形切片求交。如果控件完全在切片之外，直接跳过整棵子树——这是一种 AABB 粗筛，O(1) 的剔除操作。

2. **第二级（父控件边界裁剪）**：子控件被限制在父控件的可见区域内。`OVERFLOW_VISIBLE` 标志可以让子控件突破这种限制（用于下拉菜单、tooltip 等需要"溢出"的场景），此时跳过第二级裁剪。

### 2.9.7 渲染上下文传递

```c
typedef struct {
    ipgui_surf_t * surf;          // 绘制目标表面
    ipgui_aabb_t * clip;          // 脏矩形切片（当前渲染范围）
    ipgui_aabb_t * parent_clip;   // 父控件边界约束（NULL 为根）
    void         * user_data;     // 用户数据
} ipgui_widget_render_ctx_t;
```

渲染上下文在 DFS 遍历中逐级累积。每次递归进入子控件时构造新的 `child_ctx`，将当前控件的边界作为子控件的 `parent_clip`。

```c
if (widget->flags & IPGUI_WIDGET_FLAG_OVERFLOW_VISIBLE) {
    sub_ctx.parent_clip = ctx->parent_clip;  // 不限制子控件
} else {
    sub_ctx.parent_clip = &child_clip;       // 限制在父控件边界内
}
```

---

## 2.10 完整渲染链路：从状态变化到像素

### 2.10.1 追踪一个按钮的颜色变化

以下追踪"用户点击按钮，按钮背景变为高亮色"的完整链路：

```
第 1 步：标记脏区域
    button->set_highlight(button, true)
    → ipgui_widget_mark_dirty(button)
    → ipgui_dirty_rect_add_xywh(&scr->dirty, abs_x, abs_y, w, h)
    → pool_add(&scr->dirty, dirty_rect)
        → 包含关系检查（丢弃被已有矩形完全包含的新矩形）
        → 如果池满 → arr_merge_best_pair() 局部最优合并

第 2 步：帧渲染触发
    ipgui_screen_render(scr)
    → if (pool_num == 0) return;  // 静止帧快速退出

第 3 步：脏矩形全局合并
    ipgui_dirty_rect_flush(&scr->dirty)
    → 迭代贪心合并，直到无合并代价 ≤ 阈值的矩形对

第 4 步：遍历合并后的脏矩形
    for each dirty in pool:
        ipgui_screen_render_dirty_rect(scr, dirty)
        → ipgui_rect_slice_ctx_init(&ctx, dirty, pfb->num_pixs)
        → while (ipgui_get_rect_slice(&ctx, &slice)):
            ipgui_screen_render_dirty_rect_slice(scr, pfb, &slice)

第 5 步：单片渲染
    ipgui_screen_render_dirty_rect_slice(scr, pfb, &slice)
    → ipgui_memset(pfb->color, 0)     // 清空 PFB
    → 构造 surf（零拷贝：surf.color = pfb->color）
    → DFS 遍历控件树

第 6 步：控件绘制
    button->render(button, &ctx)
    → ipgui_draw_box_background(surf, clip, &abs, &shape, &bg)
        → get_max_radius → 5 矩形条 + 4 圆角区域
        → 圆角: ipgui_fetch_ring_mask → mask
        → ipgui_blend(surf, clip, dest, paint, opacity, mask, ...)

第 7 步：Paint 分发 → 像素混合
    ipgui_blend()
    → switch (paint->type):
        COLOR → ipgui_blend_color()
        GRADIENT → ipgui_blend_gradient_color()
        IMAGE → ipgui_blend_image_v2()

    最终：premult_blend_table[dst_fmt](premult_color, dest_pixel, blend_mode)
         → 像素混合写入 PFB

第 8 步：PFB 刷新到物理屏幕
    scr->drv->fill_region(scr, x1, y1, x2, y2, pfb->color, stride)
    → 硬件 DMA / SPI 传输到 LCD 控制器

第 9 步：重置脏矩形
    ipgui_dirty_rect_mgr_reset(&scr->dirty)
    → pool_num = 0
```

---

## 2.11 性能分析

### 2.11.1 关键优化策略量化评估

| 优化策略 | 适用场景 | 时间复杂度变化 | 典型节省 |
|---------|---------|---------------|---------|
| Premultiplied Alpha | 所有 blend 操作 | 每像素 -3 次乘法 | ~5ms/帧 (320×240) |
| 像素格式函数表 | 混合器派发 | O(n) switch → O(1) 查表 | ~0.3ms/帧 |
| RGB565 Packed Blend | RGB565/BGR565 目标 | 无需拆包/打包 | ~40% blend 时间 |
| 倒数查表 g_inv_tbl | 32位复合 alpha | 除法 → 乘法+移位 | ~10-20× 加速 |
| 渐变方向快速通道 | 水平/垂直线性渐变 | O(w×h) → O(w) | ~90% 像素计算 |
| 脏矩形系统 | 屏幕局部更新 | 全屏重绘 → 增量 | ~50-95% 减少 |
| 矩形切片除法消除 | 切片热路径 | N 次除法 → 2 次 | ~98% 除法减少 |
| 两级裁剪 | DFS 控件遍历 | 全遍历 → 空间剔除 | ~30-70% 控件跳过 |
| Alpha 阈值跳过 | 逐像素 blend | 无 → 跳过不可见像素 | ~5-15% blend 减少 |

### 2.11.2 内存使用平衡

| 组件 | 内存需求 | 备注 |
|------|---------|------|
| PFB | `slice_len × pix_size` 字节 | 可配置大小 |
| Mask Buffer | `w × res_h` 字节 | 降级分配保证不失败 |
| Dirty Rect Pool | `8 × 16` 字节 | 固定 128 字节 |
| Rect Slice Ctx | ~48 字节 | 栈分配 |
| premult_blend_table | `PIX_FMT_MAX × 4` 字节 | ~40 字节 |
| g_inv_tbl | 512 字节 ROM | 可选（USE_INV_TABLE） |
| mask_blend_table | 64KB ROM | 可选（IPGUI_MASK_GRADIENT_LUT_EN） |

---

## 2.12 本章小结

本章从源码层面完整剖析了 ESDBox_IPGUI 的颜色混合合成系统和渲染调度管线：

| 子系统 | 核心机制 | 关键优化 |
|--------|---------|---------|
| Paint 分发器 | tagged union + switch 派发 | 三个分支完全对称，O(1) 派发 |
| Premultiplied Alpha | 存储时预乘 alpha 通道 | 每像素节省 3 次乘法 |
| 像素格式函数表 | `premult_blend_table[PIX_FMT_MAX]` | O(1) 表查找替代 switch/if-else |
| RGB565 Packed Blend | MASK_RB/MASK_G 位隔离 | 不拆包的 packed 空间混合 |
| 端序感知 32 位访问 | `#if ENDIAN_LITTLE` 编译期分支 | 编译后零开销 |
| 复合 Alpha + 倒数查表 | `g_inv_tbl[256]` 替代除法 | 10-20× 除法加速 |
| 渐变方向快速通道 | 水平/垂直渐变 → 逐列/逐行 | O(w×h) → O(w) |
| 图像格式交叉矩阵 | 80 种组合的层次化编排 | LA88 委托、跨格式重组 |
| 脏矩形两阶段合并 | 积累时局部最优 + 刷新时全局最优 | 分离增量和批量优化 |
| 矩形切片器 | 贪心列优先 + 除法消除 | 热路径零除法，98% 除法减少 |
| DFS 控件遍历 | 先序遍历 + 两级裁剪 | 空间剔除跳过不可见子树 |
| PFB 零拷贝 surf | 描述符直接指向 PFB 缓冲区 | 零 malloc + 零 memcpy |

---

### 思考问题

1. Premultiplied Alpha 节省了每像素 3 次乘法，但如果目标格式本身就是 premultiplied（如 ARGB8888 的可选两种解释），背景的乘法是否也可以省略？分析完整优化后的每像素运算量。

2. RGB565 Packed Blend 的位隔离利用了 G 区域的 6 位间隙。如果将 RGB565 改为每通道 5 位的 RGB555（无绿色特权），Packed Blend 还能工作吗？位掩码需要如何调整？

3. `g_inv_tbl` 存储的是 `255 × 255 / alpha`。为什么不是 `65536 / alpha`（Q16 查表）？比较两种查表方式在精度和乘法次数上的差异。

4. 脏矩形合并的代价函数中，`IPGUI_MERGE_COST_THRESHOLD = 0` 表示"只有不增加总面积的合并才执行"。如果将阈值设为 `屏幕像素的 1%`，在什么 UI 场景下有利于减少渲染调用次数？在什么场景下会浪费渲染带宽？

5. 矩形切片器的除法消除依赖 `strip_w` 在热路径中恒等于 `slice_len`。在 `full_w < slice_len` 的窄矩形场景中，这个假设还成立吗？优化后的代码如何处理这种情况？

6. DFS 控件遍历中，控件 AABB 与脏矩形求交是一个 O(1) 的比较操作。但在控件数量达到数百个的界面上，每个脏矩形切片都要遍历全部控件。是否存在更高效的空间索引（如四叉树）来优化这个 O(n) 遍历？

---

**全书结语**：

本书从嵌入式图形系统的两个核心——几何绘制引擎和颜色合成系统——出发，以 ESDBox_IPGUI 的 4000+ 行工程源码为依托，逐层解析了从子像素坐标到屏幕像素的完整技术栈。

两个模块的设计遵循共同的工程哲学：

- **解耦优先**：gfx 管几何不碰颜色，composite 管颜色不碰几何——通过 `ipgui_blend(mask, paint)` 这一个接口完成所有协作
- **空间换时间**：修正因子表（127 字节）、倒数查表（512 字节）、Premultiplied Alpha（颜色存储格式本身）、LRU 缓存——所有用内存换取计算量的选择都是经过量化评估的决策
- **位级优化**：RGB565 Packed Blend 的掩码位隔离、Q26.6 定点数的位运算转换、端序编译期分支——这些优化的共同特点是"常数成本、零运行时开销"
- **降级优雅**：遮罩缓冲区的逐行降级分配、PfB 的切片渲染——系统在任何内存约束下都能完成渲染，质量可降级但决不崩溃

嵌入式图形系统不是桌面图形系统的"缩小版"，而是一个完全不同的工程空间——在这里，每一条乘法指令都需要为它的存在提供理由，每一个查找表都要为它占用的 ROM 空间做出辩护。ESDBox_IPGUI 的源码展示了如何在这个空间中做出精确的工程决策。

