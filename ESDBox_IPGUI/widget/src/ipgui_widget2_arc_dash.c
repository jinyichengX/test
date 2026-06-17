
#include "ipgui_blend.h"
#include "ipgui_pattle.h"
#include "ipgui_draw_arc.h"
#include "ipgui_widget.h"
#include "ipgui_draw_image.h"
#include "ipgui_image_geometry_transform.h"

void widget2_render(struct ipgui_widget * widget, ipgui_widget_render_ctx_t * ctx)
{
        ipgui_img_dsc_t img4_dsc;
    if(0 != test_bmp("M:/test/ESDBox_IPGUI/widget/src/dash.bmp", &img4_dsc))
    {
        printf("Failed to load bitmap image, please run the program at the main.c page\n");
        return -1;
    }
    ipgui_image_data_t dash_img;
    dash_img.pixmap = img4_dsc.pixmap;
    dash_img.px_size = img4_dsc.stride / img4_dsc.w;
    dash_img.fmt = IPGUI_IMG_FMT_BGRA8888;//鍙互鏀规垚L8鎴栬€匧A88鎴栬€匯GB565璇曡瘯锛岃櫧鐒惰繖涔堟敼閫昏緫涓婁笉瀵癸紝浣嗘槸鏈夋晥鏋?
    dash_img.stride = img4_dsc.stride;
    dash_img.w = img4_dsc.w;
    dash_img.h = img4_dsc.h;
    
    ipgui_coord_t ori_x, ori_y;
    ori_x = 102;
    ori_y = 282;

    ipgui_image_draw_style_t draw_style;
    draw_style.blend_mode = IPGUI_BLEND_NORMAL;
    draw_style.opacity = 255;

    ipgui_img_geo_trans_t trans1;

    ipgui_point_t pivot2;
    pivot2.x = dash_img.w / 2;
    pivot2.y = dash_img.h / 2 + 130;

    ipgui_point_t anchor;
    anchor.x = ori_x;
    anchor.y = ori_y;
    for (s32_t i = 0; i < 90; i ++)
    {
        ipgui_image_trans_init(&trans1);
        ipgui_image_trans_pivot(&trans1, pivot2);
        ipgui_image_trans_rotate_degree(&trans1, i * 4);

        ipgui_draw_image(
            ctx->surf, 
            (ipgui_aabb_t *)0, 
            &dash_img, 
            &trans1.pivot,
            &anchor, 
            (ipgui_trans_mat_t *)&trans1.mat,
            &draw_style);
    }
}
