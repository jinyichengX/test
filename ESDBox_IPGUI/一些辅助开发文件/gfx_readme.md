

ring_mask问题
1. cache总数应该限制而不是无脑从内存中分配,否则内存过大时，查找效率也会变低(解决)
2. DEBUG_CORNER_MASK关掉
3. 添加直接生成mask部分


重要思想
增量法


一些坑
__IPGUI_API__ ipgui_edge_coord_t edge_x_at_y(
    ipgui_edge_param_t * p, 
    ipgui_coord_t y)
{
    if (p->dy == 0) return 0;
    s64_t temp;
    ipgui_edge_coord_t dy;

    dy = (y * 64) - p->y1;
    temp = (s64_t)dy * p->dx;
    return temp / p->dy + p->x1;
}
如果将上面的代码改为
__IPGUI_API__ ipgui_edge_coord_t edge_x_at_y(
    ipgui_edge_param_t * p, 
    ipgui_coord_t y)
{
    if (p->dy == 0) return 0;
    s64_t temp;
    ipgui_edge_coord_t dy;

    dy = (y * 64) - p->y1;
    temp = dy * p->dx;
    return temp / p->dy + p->x1;
}
在斜率太小时（大概几百分之一吧）就会发生temp计算溢出
原因是：C 语言标准，两个 s32_t 相乘，其临时中间结果也会被放在一个 s32_t 宽度的寄存器里。
尽管temp是64位，但是也不行,这时候再大的“桶”（s64_t）也接不到已经洒掉的水了


需要继续优化的地方
1. 画三角形现在的逻辑是逐像素计算mask，效率一般，可以优化掉(已经解决)
2. ringmask中添加实时计算mask
3. edgemask没有对水平线的处理
4. 现在的圆弧端点圆的画法是直接在端点处补圆，而不是用edge切除，这样会导致opacity<255时端点颜色加深
5. ipgui_draw_line_generic.c完成度不高，需要继续写(已经解决)
6. edge_x_at_y除法优化
7. 直线的圆形端点，需要优化，目前是直接在端点处补圆，会导致当opacity<255时，端点处有半个圆的颜色加深
8. ipgui_edge_wdf_mask计算x_halfspan是采用逐点步进试探法，可以改为二倍步进 + 二分缩进法
9. ipgui_draw_image中插值那一块只支持px_fmt为rgb888的图像，需要增加不同像素的插值函数（已经解决）
10. 为了保证图片质量，ipgui_draw_image现在只支持二次插值，需要支持一次线性插值
11. 需要再写一个文件ipgui_draw_icon.c，用于绘制图标因为现在的ipgui_draw_image.c绘制不了，会将L8格式转换为灰度像素
12. ipgui_draw_box_shadow.c自己重写，需要达到CSS的阴影效果

需要注意的地方
1. 调用ipgui_draw_image，img_data中的px_size不能大于10，因为在函数中写死了u8_t cr[10];

edge_halfplane_mask和edge_wdf_mask的区别：
edge_halfplane_mask是用于生成一个edge的半平面的mask
edge_wdf_mask是用于生成一个edge的距离场(width distance-field)的mask，但是只用于整数端点线段！！！

有时间研究下egui_mask_circle_edge_smoothstep