# 前言

## 本书立意

嵌入式图形系统是一个交叉领域：它既需要计算机图形学的理论基础，又必须直面嵌入式环境的严苛约束——有限的处理器主频、匮乏的随机存储器、没有硬件浮点单元，以及常常只有十几KB的代码空间。ESDBox_IPGUI 正是在这些约束下从零构建的一套图形渲染引擎，纯 C 语言实现，覆盖从基础像素着色到完整控件系统的全链路。

本书聚焦于该引擎的两个核心子模块——**图形绘制引擎（core/gfx）** 与**颜色合成系统（core/composite）**。全书采用"先读源码、再讲设计"的叙述方式：每一节都从实际源码出发，分析其数据结构、核心算法、边界处理和性能优化策略，最终回归到架构设计层面的思考。读者将看到的不是抽象的伪代码或原理示意，而是可以直接编译运行的工程代码及其背后的取舍逻辑。

## 适用读者

- 具有 C 语言基础，希望理解图形渲染底层实现的开发者
- 正在构建或维护嵌入式 GUI 系统的工程师
- 对实时图形算法和嵌入式优化策略感兴趣的研究者
- 需要参考完整工程实践来设计自研渲染模块的架构师

## 阅读建议

两章构成递进关系。第一章建立的"遮罩"概念和"表面坐标系"是第二章理解 Paint 分发器和 premultiplied alpha 混合的基础。建议按顺序阅读。

每节内容独立成篇：你可以在需要时直接跳转到某一节，查阅特定的算法或数据结构。但理解完整的设计动机，需要跟随全书脉络。

## 工程结构导航

```
ESDBox_IPGUI/
├── core/gfx/                    # 第一章：图形绘制引擎
│   ├── ipgui_edge_halfplane_mask.c/h  # 半平面边缘遮罩（2.6 定点数）
│   ├── ipgui_edge_wdf_mask.c/h        # 宽线厚度遮罩
│   ├── ipgui_ring_mask.c/h            # 环形遮罩（圆角矩形 SDF + LRU 缓存）
│   ├── ipgui_draw_line.c/h            # 线段绘制（Wu 经典 + 宽线 + 线帽 + 渐变）
│   ├── ipgui_draw_arc.c/h             # 弧/圆/扇形/圆环统一接口
│   ├── ipgui_draw_polygon.c/h         # 多边形光栅化（扫描线 + AVL 活动边表）
│   ├── ipgui_draw_box_background.c/h  # Box 背景绘制
│   ├── ipgui_draw_box_border.c/h      # Box 边框绘制
│   ├── ipgui_draw_box_shadow.c/h      # Box 阴影（SDF + 1D 多项式 CDF 近似）
│   ├── ipgui_gradient_color.c/h       # 渐变色彩（线性/径向/锥形）
│   ├── ipgui_mask_gradient.c/h        # 遮罩渐变
│   ├── ipgui_image_buf.c/h            # 图像缓冲区管理
│   └── ipgui_mask_buf.c/h             # 遮罩缓冲区管理
├── core/composite/              # 第二章：颜色合成系统
│   ├── ipgui_blend.c/h                # Paint 分发器
│   ├── ipgui_blend_color.c/h          # 纯色混合（函数表 + packed blend）
│   ├── ipgui_blend_gradient_color.c/h # 渐变混合（方向优化快速通道）
│   └── ipgui_blend_image.c/h          # 图像混合（全像素格式交叉矩阵）
└── al/hal/                      # 渲染调度层
    ├── ipgui_screen.c/h               # 屏幕渲染主入口
    └── ipgui_rect_slice.c/h           # 矩形切片器（贪心列优先 + 除法消除）
```

## 排版约定

- **常量**：`IPGUI_COLOR_A`、`MASK_RB`（全大写 + 下划线）
- **函数**：`ipgui_blend()`、`ipgui_edge_wdf_mask()`（小写 + 下划线）
- **类型**：`ipgui_color_t`、`ipgui_surf_t`（小写 + `_t` 后缀）
- **文件路径**：`core/gfx/ipgui_draw_line.c`（等宽字体）
- 书中出现的所有源码片段均来自真实工程文件，行号与文件实际行号一致

## 术语对照

| 术语 | 对应概念 | 说明 |
|------|---------|------|
| 遮罩（Mask） | Alpha mask / coverage mask | 0-255 的通道权重数组，控制颜色的像素级透明度 |
| 表面（Surface） | Framebuffer slice | 描述一段像素缓冲区的元信息（坐标、跨度、格式） |
| 脏矩形（Dirty rect） | Invalidation region | 屏幕中需要重绘的区域 |
| PFB（Partial FB） | 部分帧缓冲区 | 由于内存限制，每次只分配覆盖一个脏矩形的像素缓冲区 |
| Premultiplied alpha | 预乘 Alpha | 颜色通道在混合前已与 alpha 通道相乘的格式 |
| 26.6 定点数 | Q26.6 fixed-point | 32 位整数，高 26 位为整数部，低 6 位为小数部（1/64 精度） |
| SDF | Signed Distance Field | 有符号距离场：每个像素存储到最近形状边界的距离 |

---
