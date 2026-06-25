# IPGUI 图形引擎移植手册

> 将 `core/gfx/` 和 `core/composite/` 移植到新工程所需的所有外部文件清单。
> 共 53 个 gfx/composite 内部文件，依赖 19 个外部文件（含 6 个纯头文件）。
>
> 生成时间: 2026-06-24

---

## 目录

1. [Layer 0 - 基础类型（纯头文件，零实现）](#layer-0)
2. [Layer 1 - 基础库（头文件 + .c 实现）](#layer-1)
3. [Layer 2 - 图形支撑（纯头文件）](#layer-2)
4. [Layer 3 - 可选 / 调试](#layer-3)
5. [文件清单速查表](#checklist)
6. [移植步骤](#steps)
7. [依赖关系图](#diagram)

---

<a id="layer-0"></a>
## Layer 0 — 基础类型（8 个，全部包含在 Include/ 下）

**所有 gfx/composite 文件都直接或间接依赖这些头文件。它们只定义类型和宏，不需要 .c 实现。**

| 文件 | 提供的内容 | 被依赖次数 |
|---|---|---|
| `Include/ipgui_types.h` | `s32_t`, `u32_t`, `ipgui_err_t`, `ipgui_tick_t` 等基础类型 | 22 |
| `Include/ipgui_conf.h` | `IPGUI_ENDIAN_LITTLE`, `USE_INV_TABLE`, `IPGUI_SMEM_SIZE` 等编译配置宏 | 19 |
| `Include/ipgui_utils.h` | `__IPGUI_API__`, `__IPGUI_STATIC__`, `IPGUI_MIN/MAX/ABS` 等工具宏 | 21 |
| `Include/ipgui_coord.h` | `ipgui_coord_t`, `ipgui_coordf_t` 坐标类型 | 22 |
| `Include/ipgui_color.h` | `ipgui_color_t` 颜色联合体 | 19 |
| `core/misc/ipgui_lcd_pix_fmt.h` | `ipgui_pix_fmt_t` 像素格式枚举 | 17 |
| `core/misc/ipgui_prim.h` | `ipgui_point_t`, `ipgui_rect_t`, `ipgui_aabb_t`, `ipgui_circle_t` 及其 inline 函数 | 21 |
| `Include/ipgui_core.h` | `ipgui_surf_t`（帧缓冲）, `ipgui_pfb_t`（离屏缓冲） | 17 |

**额外需要的 .c 实现：**
- `core/misc/ipgui_prim.c` — 实现 `ipgui_aabb_intersect()` 等 extern 声明的函数

---

<a id="layer-1"></a>
## Layer 1 — 基础库（6 组，base/ 下）

**gfx/composite 的 .c 文件直接引用了这些库的函数。需要移植 .h 和对应的 .c。**

### 1.1 ipgui_memory — 内存分配

- `base/inc/ipgui_memory.h` + `base/src/ipgui_memory.c`

被依赖的 .c 文件: ipgui_blend_color, ipgui_blend_gradient_color, ipgui_draw_box_shadow,
ipgui_draw_image, ipgui_draw_polygon, ipgui_draw_image_api, ipgui_draw_image_ex,
ipgui_gradient_color, ipgui_mask_buf, ipgui_ring_mask, ipgui_edge_wdf_mask（11 个）

**职责：** 提供 `ipgui_mem_alloc_def()` / `ipgui_mem_free_def()` 和内存模块初始化。
你需要实现这两个函数适配目标平台的堆分配器。

### 1.2 ipgui_membox — 定长块分配器

- `base/inc/ipgui_membox.h` + `base/src/ipgui_membox.c`

被依赖的 .c 文件: ipgui_draw_polygon（多边形光栅化用）

**职责：** 快速分配固定大小的内存块，用于光栅化管线中的顶点/边缓存。
如果不需要多边形绘制功能，可以跳过。

### 1.3 ipgui_mempool — 内存池

- `base/inc/ipgui_mempool.h` + `base/src/ipgui_mempool.c`

被依赖的 .c 文件: ipgui_mask_buf

**职责：** 蒙版缓冲区的内存池管理。

### 1.4 ipgui_math — 数学函数

- `base/inc/ipgui_math.h` + `base/src/ipgui_math.c`
- `core/misc/ipgui_angle.h`（被 ipgui_math.h 依赖，纯头文件）

被依赖的 .c 文件: ipgui_draw_arc, ipgui_edge_wdf_mask（三角函数用）

**职责：** `sin`, `cos`, `sqrt`, `atan2` 等数学函数的定点/浮点实现。

### 1.5 ipgui_avl — AVL 平衡树 / 链表

- `base/inc/ipgui_avl.h` + `base/src/ipgui_avl.c`
- `base/src/ipgui_list.h`（纯头文件，双向链表实现，被 ipgui_memory.h 包含）

被依赖的 .c 文件: ipgui_draw_polygon（多边形边表用 AVL 树管理）

### 1.6 ipgui_debug — 调试日志

- `base/inc/ipgui_debug.h` + `base/src/ipgui_debug.c`

被依赖的 .c 文件: 12 个（几乎所有 gfx .c 都用了 `IPGUI_LOG_D` 等宏）

**职责：** `IPGUI_LOG_D()`, `IPGUI_LOG_E()` 等调试打印宏。
如果不需要日志，可以将其定义为空宏，跳过 .c 移植。

---

<a id="layer-2"></a>
## Layer 2 — 图形支撑

### 2.1 ipgui_graphic2 — 图形基础类型

- `core/misc/ipgui_graphic2.h`（纯头文件）

被依赖文件: ipgui_draw_polygon.h

**职责：** 定义 `ipgui_ras_edge_t`、`ipgui_poly2_t` 等光栅化用结构体。

### 2.2 ipgui_pattle — 调色板

- `core/misc/ipgui_pattle.h`（纯头文件）

被依赖文件: ipgui_edge_halfplane_mask.c, ipgui_edge_wdf_mask.c

**职责：** 边缘抗锯齿掩码的调色板 LUT 生成。

---

<a id="layer-3"></a>
## Layer 3 — 可选 / 调试

### 3.1 ipgui_image / ipgui_image_geometry_transform — 图像几何变换

> 仅当移植 `ipgui_draw_image_api.c` 时需要。

- `core/image/ipgui_image.h` + `core/image/ipgui_image.c`
- `core/image/proc/ipgui_image_geometry_transform.h` + `core/image/proc/ipgui_image_geometry_transform.c`

`ipgui_draw_image_api.c` 通过 `ipgui_image_geometry_transform.h` 间接依赖这些文件。

---

<a id="checklist"></a>
## 文件清单速查表

### 必须移植

```
Include/
  ipgui_types.h          # 基础类型定义
  ipgui_conf.h           # 编译配置宏
  ipgui_utils.h          # 工具宏 (min/max/align/IPGUI_API)
  ipgui_coord.h          # 坐标类型
  ipgui_color.h          # 颜色类型
  ipgui_core.h           # 帧缓冲 surf/pfb 类型

core/misc/
  ipgui_lcd_pix_fmt.h    # 像素格式枚举
  ipgui_prim.h           # 基础几何类型 (point/rect/aabb)
  ipgui_prim.c           # 几何计算实现
  ipgui_graphic2.h       # 光栅化图形类型
  ipgui_pattle.h         # 调色板
  ipgui_angle.h          # 角度类型 (被 math.h 依赖)

base/inc/  +  base/src/
  ipgui_list.h           # 双向链表 (纯头文件，在 src 下)
  ipgui_avl.h    + .c    # AVL 平衡树
  ipgui_memory.h + .c    # 内存分配器
  ipgui_membox.h + .c    # 定长块分配器
  ipgui_mempool.h + .c   # 内存池
  ipgui_math.h   + .c    # 数学函数
```

### 建议移植

```
base/inc/ipgui_debug.h + base/src/ipgui_debug.c   # 调试日志
```

### 按需移植

```
core/image/
  ipgui_image.h + .c                              # 图像数据结构
  proc/ipgui_image_geometry_transform.h + .c      # 图像几何变换
```

---

<a id="steps"></a>
## 移植步骤

### 第一步：复制 Layer 0 头文件

将 `Include/` 下的 6 个头文件 + `core/misc/` 下的 5 个头文件 + `core/misc/ipgui_prim.c` 复制到新工程的对应目录。

**验证：** 将 `core/composite/ipgui_blend.h` 放入新工程，编译通过即 Layer 0 完整。

### 第二步：实现内存分配

`ipgui_memory.c` 是移植的关键 — 它调用了 `ipgui_mem_alloc_def()` 和 `ipgui_mem_free_def()`，
你需要对接目标平台的 `malloc/free`：

```c
// ipgui_memory.c 的默认实现框架
void * ipgui_mem_alloc_def(u32_t size) {
    return malloc(size);  // 替换为你的平台 API
}
void ipgui_mem_free_def(void * ptr) {
    free(ptr);
}
```

### 第三步：实现数学函数

`ipgui_math.h` 声明了 `ipgui_sin()`, `ipgui_cos()`, `ipgui_sqrt()` 等函数。
在 `ipgui_math.c` 中按目标平台实现（可能是定点 Q16.16，也可能直接调 math.h）。

### 第四步：移植 base/ 库

按顺序移植：`ipgui_list.h` → `ipgui_avl` → `ipgui_memory` → `ipgui_membox` → `ipgui_mempool` → `ipgui_math`

**验证：**
```c
#include "ipgui_prim.h"   // Layer 0
#include "ipgui_memory.h" // Layer 1
// 编译通过说明 Layer 0+1 完整
```

### 第五步：移植 gfx/ 和 composite/

将 `core/gfx/` 和 `core/composite/` 全部复制到新工程。编译，根据错误提示补漏（通常就是 ipgui_debug.h 或图像变换）。

---

<a id="diagram"></a>
## 依赖关系图

```
                         gfx/ + composite/ (53 files)
                               │
          ┌────────────────────┼────────────────────┐
          │                    │                    │
    Layer 0 (类型)        Layer 1 (基础库)    Layer 2 (图形)
          │                    │                    │
    ┌─────┼─────┐     ┌───────┼───────┐     ┌──────┼──────┐
    │     │     │     │       │       │     │      │      │
  types  coord color  memory membox mempool prim   graphic2 pattle
  conf   core         math   avl    list    .c          (纯h)
  utils  pix_fmt      debug(可选)   angle(纯h)

所有箭头最终汇聚到 ipgui_types.h
```

---

## 移植后编译顺序

```makefile
# 1. Layer 0 (纯头文件，无需编译)
# 2. Layer 1 base 库
SRC_BASE = \
    ipgui_list.c      # 实际是纯头，但可能需要在编译路径中
    ipgui_avl.c       \
    ipgui_memory.c    \
    ipgui_membox.c    \
    ipgui_mempool.c   \
    ipgui_math.c      \
    ipgui_debug.c     # 可选

# 3. core/misc
SRC_MISC = \
    ipgui_prim.c

# 4. gfx + composite (全部)
SRC_GFX_COMPOSITE = \  # 53 .c 文件
    ipgui_blend.c                  \
    ipgui_blend_color.c            \
    ipgui_blend_gradient_color.c   \
    ipgui_blend_image.c            \
    ipgui_draw_arc.c               \
    ipgui_draw_box_background.c    \
    ipgui_draw_box_border.c        \
    ipgui_draw_box_shadow.c        \
    ipgui_draw_builtin_font.c      \
    ipgui_draw_filled_circle.c     \
    ipgui_draw_image.c             \
    ipgui_draw_image_api.c         \
    ipgui_draw_image_ex.c          \
    ipgui_draw_line.c              \
    ipgui_draw_pixel.c             \
    ipgui_draw_polygon.c           \
    ipgui_draw_triangle.c          \
    ipgui_edge_halfplane_mask.c    \
    ipgui_edge_wdf_mask.c          \
    ipgui_gradient_color.c         \
    ipgui_image_buf.c              \
    ipgui_image_mask.c             \
    ipgui_mask_buf.c               \
    ipgui_mask_gradient.c          \
    ipgui_ring_mask.c
```
