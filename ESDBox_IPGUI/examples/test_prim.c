    #include "ipgui_prim.h"
    int main(void)
    {
        ipgui_rect_t aabb;
        ipgui_point_t points[] = {
            { 25, 50 },
            { 50, 40 },
            { 70, 65 },
            { 100, 40 },
            { 120, 60 },
            { 70, 130 },
            { 50, 100 },
            { 40, 140 },
            { 10, 60 },
            { 38, 66 },
        };
        ipgui_aabb_generate(&aabb, points, IPGUI_ARRAY_LEN(points));
        return 0;
    }