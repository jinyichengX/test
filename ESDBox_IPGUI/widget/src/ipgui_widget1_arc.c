#include "ipgui_blend.h"
#include "ipgui_pattle.h"
#include "ipgui_draw_arc.h"
#include "ipgui_widget.h"

void widget1_render(struct ipgui_widget * widget, ipgui_widget_render_ctx_t * ctx)
{
    // 定义彩虹色环参数
    ipgui_arc_t arc;
    arc.cx =  90;       // 圆心X坐标（屏幕左侧居中）
    arc.cy = 290;       // 圆心Y坐标（屏幕垂直居中）
    arc.dir = IPGUI_ARC_DRAW_DIR_CCW;
    arc.er = 175;       // 外半径
    arc.ir = arc.er - 13;       // 内半径，环宽度30px
    arc.start = 0;      // 起始角度0度
    arc.angle = 360;    // 完整360度圆环

    // 配置圆弧样式
    ipgui_arc_style_t arc_style;
    memset(&arc_style, 0, sizeof(arc_style));
    arc_style.blend_mode = IPGUI_BLEND_NORMAL;
    arc_style.sep_type = IPGUI_ARC_ENDPOINT_TYPE_BUTT;
    arc_style.eep_type = IPGUI_ARC_ENDPOINT_TYPE_BUTT;
    arc_style.opacity = 255;
    arc_style.paint.type = IPGUI_PAINT_GRADIENT;

    // 配置锥形（圆锥）渐变，和圆环同心
    arc_style.paint.src.grad_src.grad_type = IPGUI_GRADIENT_TYPE_CONIC;
    ipgui_conic_gradient_init(&arc_style.paint.src.grad_src.grad.conic_grad, arc.cx, arc.cy, 180);

    // 彩虹渐变色标（15种颜色，相邻色系严格不同：紫→蓝→绿→黄绿→黄→橙→红→品红→紫→蓝→绿→黄→橙→红→紫）
    ipgui_gradient_color_stop_t stop1;
    stop1.pos = 0;
    IPGUI_COLOR_SET(stop1.color, 255, IPGUI_COLOR_92); // 丹紫红
    ipgui_conic_gradient_add_stop(&arc_style.paint.src.grad_src.grad.conic_grad, &stop1);

    ipgui_gradient_color_stop_t stop2;
    stop2.pos = 17;
    IPGUI_COLOR_SET(stop2.color, 255, IPGUI_COLOR_149); // 飞燕草蓝
    ipgui_conic_gradient_add_stop(&arc_style.paint.src.grad_src.grad.conic_grad, &stop2);

    ipgui_gradient_color_stop_t stop3;
    stop3.pos = 34;
    IPGUI_COLOR_SET(stop3.color, 255, IPGUI_COLOR_213); // 美蝶绿
    ipgui_conic_gradient_add_stop(&arc_style.paint.src.grad_src.grad.conic_grad, &stop3);

    ipgui_gradient_color_stop_t stop4;
    stop4.pos = 51;
    IPGUI_COLOR_SET(stop4.color, 255, IPGUI_COLOR_280); // 槐花黄绿
    ipgui_conic_gradient_add_stop(&arc_style.paint.src.grad_src.grad.conic_grad, &stop4);

    ipgui_gradient_color_stop_t stop5;
    stop5.pos = 68;
    IPGUI_COLOR_SET(stop5.color, 255, IPGUI_COLOR_306); // 柠檬黄
    ipgui_conic_gradient_add_stop(&arc_style.paint.src.grad_src.grad.conic_grad, &stop5);

    ipgui_gradient_color_stop_t stop6;
    stop6.pos = 85;
    IPGUI_COLOR_SET(stop6.color, 255, IPGUI_COLOR_365); // 橙皮黄
    ipgui_conic_gradient_add_stop(&arc_style.paint.src.grad_src.grad.conic_grad, &stop6);

    ipgui_gradient_color_stop_t stop7;
    stop7.pos = 102;
    IPGUI_COLOR_SET(stop7.color, 100, IPGUI_COLOR_14); // 苋菜红
    ipgui_conic_gradient_add_stop(&arc_style.paint.src.grad_src.grad.conic_grad, &stop7);

    ipgui_gradient_color_stop_t stop8;
    stop8.pos = 119;
    IPGUI_COLOR_SET(stop8.color, 255, IPGUI_COLOR_71); // 品红
    ipgui_conic_gradient_add_stop(&arc_style.paint.src.grad_src.grad.conic_grad, &stop8);

    ipgui_gradient_color_stop_t stop9;
    stop9.pos = 136;
    IPGUI_COLOR_SET(stop9.color, 255, IPGUI_COLOR_135); // 满天星紫
    ipgui_conic_gradient_add_stop(&arc_style.paint.src.grad_src.grad.conic_grad, &stop9);

    ipgui_gradient_color_stop_t stop10;
    stop10.pos = 153;
    IPGUI_COLOR_SET(stop10.color, 255, IPGUI_COLOR_153); // 景泰蓝
    ipgui_conic_gradient_add_stop(&arc_style.paint.src.grad_src.grad.conic_grad, &stop10);

    ipgui_gradient_color_stop_t stop11;
    stop11.pos = 170;
    IPGUI_COLOR_SET(stop11.color, 255, IPGUI_COLOR_235); // 翠绿
    ipgui_conic_gradient_add_stop(&arc_style.paint.src.grad_src.grad.conic_grad, &stop11);

    ipgui_gradient_color_stop_t stop12;
    stop12.pos = 187;
    IPGUI_COLOR_SET(stop12.color, 255, IPGUI_COLOR_326); // 浅烙黄
    ipgui_conic_gradient_add_stop(&arc_style.paint.src.grad_src.grad.conic_grad, &stop12);

    ipgui_gradient_color_stop_t stop13;
    stop13.pos = 204;
    IPGUI_COLOR_SET(stop13.color, 255, IPGUI_COLOR_377); // 雄黄
    ipgui_conic_gradient_add_stop(&arc_style.paint.src.grad_src.grad.conic_grad, &stop13);

    ipgui_gradient_color_stop_t stop14;
    stop14.pos = 221;
    IPGUI_COLOR_SET(stop14.color, 255, IPGUI_COLOR_6); // 艳红
    ipgui_conic_gradient_add_stop(&arc_style.paint.src.grad_src.grad.conic_grad, &stop14);

    ipgui_gradient_color_stop_t stop15;
    stop15.pos = 238;
    IPGUI_COLOR_SET(stop15.color, 255, IPGUI_COLOR_112); // 青莲
    ipgui_conic_gradient_add_stop(&arc_style.paint.src.grad_src.grad.conic_grad, &stop15);

    ipgui_gradient_color_stop_t stop16;
    stop16.pos = 255;
    IPGUI_COLOR_SET(stop16.color, 255, IPGUI_COLOR_92); // 回到丹紫红闭环
    ipgui_conic_gradient_add_stop(&arc_style.paint.src.grad_src.grad.conic_grad, &stop16);

    // 绘制圆环到当前surface
    ipgui_draw_arc(ctx->surf, &ctx->surf->surf, &arc, &arc_style);
}