/* ============================================================
 * RoboKit - RRT 路径可视化
 *
 * 用 IPGUI（SDL 驱动）绘制 RRT 规划结果：
 *   - 蓝色：RRT 树的所有边（分叉路径）
 *   - 红色：回溯路径（start -> end 的主干）
 *
 * 同时链接 elNET（网络协议栈，C 库）。
 * ============================================================ */

#include <cstdio>
#include <cstdint>

#include <SDL.h>

#include "planner_rrt.hpp"

extern "C" {
#include "ipgui_core.h"
#include "ipgui_screen.h"
#include "ipgui_widget.h"
#include "ipgui_memory.h"
#include "ipgui_blend.h"
#include "ipgui_draw_line.h"
#include "sdl_draw.h"
#include "elnet.h"
}

#undef main

/* ipgui_widget.c 中 extern 引用的全局屏幕，需在此定义 */
extern "C" {
ipgui_scr_t main_screen;
}

static ipgui_scr_drv_t sdl_drv;
static u8_t main_screen_frame_buf[800 * 4 * 480];

static test::RRTPlanner* g_planner = nullptr;

/* ---------------- 绘制辅助 ---------------- */

/* 地图坐标 (0,0)-(100,100) -> 屏幕坐标（缩放 4 倍 + 偏移） */
static ipgui_point_t MapToScreen(double mx, double my)
{
    ipgui_point_t p;
    p.x = (ipgui_coord_t)(mx * 4.0 + 200.0);
    p.y = (ipgui_coord_t)(my * 4.0 + 40.0);
    return p;
}

static void DrawLine(ipgui_surf_t* surf, ipgui_aabb_t* clip,
                     const ipgui_point_t& a, const ipgui_point_t& b,
                     ipgui_color_t color)
{
    ipgui_line_t line;
    line.start = a;
    line.end   = b;

    ipgui_line_style_t style;
    style.width      = 2;
    style.cap        = IPGUI_LINE_CAP_ROUND;
    style.opacity    = 255;
    style.blend_mode = IPGUI_BLEND_NORMAL;
    style.paint.type = IPGUI_PAINT_COLOR;
    style.paint.src.color = color;

    ipgui_draw_line_generic(surf, clip, &line, &style);
}

/* ---------------- 控件渲染回调 ---------------- */

static void widget_render(ipgui_widget_t* widget, ipgui_widget_render_ctx_t* ctx)
{
    (void)widget;
    ipgui_surf_t* surf = ctx->surf;

    /* 背景 */
    ipgui_color_t bg;
    IPGUI_COLOR_SET(bg, 255, 0x202830);
    ipgui_blend_color(surf, NULL, &surf->surf, bg, 255, NULL, NULL, IPGUI_BLEND_NORMAL);

    if (!g_planner)
        return;

    ipgui_color_t blue, red;
    IPGUI_COLOR_SET(blue, 255, 0x4090FF);   /* 分叉路径（树边） */
    IPGUI_COLOR_SET(red,  255, 0xFF4040);   /* 回溯路径 */

    /* 蓝色：RRT 树的所有边 */
    const auto& nodes = g_planner->node_array_;
    for (size_t i = 1; i < nodes.size(); i++) {
        if (nodes[i].parent_idx < 0)
            continue;
        DrawLine(surf, &surf->surf,
                 MapToScreen(nodes[i].point.x, nodes[i].point.y),
                 MapToScreen(nodes[nodes[i].parent_idx].point.x,
                             nodes[nodes[i].parent_idx].point.y),
                 blue);
    }

    /* 红色：回溯路径 */
    const auto& path = g_planner->GetPath();
    for (size_t i = 1; i < path.size(); i++) {
        DrawLine(surf, &surf->surf,
                 MapToScreen(path[i - 1].x, path[i - 1].y),
                 MapToScreen(path[i].x, path[i].y),
                 red);
    }
}

/* ---------------- 主程序 ---------------- */

int main()
{
    /* 1. RRT 规划（地图 0-100，起点 (10,10) 到终点 (90,90)） */
    test::RRTPlanner planner(test::Vec2(0, 0), test::Vec2(100, 100), 5.0, 10000);
    planner.DoRRTPlan(test::Vec2(10, 10), test::Vec2(90, 90));
    std::printf("RRT nodes=%zu, path_len=%zu\n",
                planner.node_array_.size(), planner.GetPath().size());
    g_planner = &planner;

    /* 2. IPGUI 屏幕初始化（SDL 驱动，参考 m:\test\main.c） */
    ipgui_memset(&main_screen, 0, sizeof(main_screen));
    ipgui_memset(&sdl_drv, 0, sizeof(sdl_drv));
    ipgui_memset(&g_sdl_private, 0, sizeof(g_sdl_private));

    sdl_drv.xreso      = 800;
    sdl_drv.yreso      = 480;
    sdl_drv.pri_data   = &g_sdl_private;
    sdl_drv.put_pixel  = sdl_put_pixel;
    sdl_drv.fill_region = sdl_fill_region;
    sdl_drv.flush      = sdl_flush;

    if (ipgui_screen_init(&main_screen, &sdl_drv) != IPGUI_ERR_OK) {
        std::printf("screen init failed\n");
        return 1;
    }
    if (ipgui_sdl_screen_init(&main_screen) != 0) {
        std::printf("sdl init failed\n");
        return 1;
    }
    ipgui_scr_create_pfb(&main_screen, main_screen_frame_buf,
                         sizeof(main_screen_frame_buf), PIX_FMT_RGBA8888);
    ipgui_init();

    /* 3. 全屏 widget 画 RRT */
    ipgui_widget_t* widget = ipgui_widget_create(NULL);
    widget->render = widget_render;
    widget->x = 0;
    widget->y = 0;
    widget->w = 800;
    widget->h = 480;

    /* 4. 事件循环 */
    bool running = true;
    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT)
                running = false;
        }
        ipgui_widget_mark_dirty(widget);
        ipgui_screen_render(&main_screen);   /* 内部最后会调用 drv->flush */
        SDL_Delay(16);
    }

    return 0;
}
