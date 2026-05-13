/* 多边形

           static ipgui_aabb_t clip1 = {
                .start = {0, 0},

                .end = {0, 0}
            };
            clip1.end.x += 1;

            clip1.end.y += 1;
ipgui_point_t star[10] = {

    { 400,  90 },

    { 433, 194 },

    { 543, 194 },

    { 454, 258 },

    { 488, 361 },

    { 400, 297 },

    { 312, 361 },

    { 346, 258 },

    { 257, 194 },

    { 367, 194 },
};
ipgui_polygon_style_t style;
style.color = g_color;
style.alpha = 255;
style.blend_mode = IPGUI_BLEND_NORMAL;
            ipgui_draw_polygon(&surf, 
                &clip1,
                star, 10, 
                &g_ras,
                &style);

*/