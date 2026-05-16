

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
1. 画三角形现在的逻辑是逐像素计算mask，效率一般，可以优化掉(已经优化解决)
2. ringmask中添加实时计算mask
3. edgemask没有对水平线的处理
4. 现在的圆弧端点圆的画法是直接在端点处补圆，而不是用edge切除，这样会导致opacity<255时端点颜色加深
5. ipgui_draw_line.c完成度不高，需要继续写
6. edge_x_at_y除法优化


各类图形支持的paint类型
1. 细线（thin line）（1px）：只支持渐变和纯色
2. 粗线：支持纯色和渐变
2. 圆角矩形： 
   边框：支持纯色/渐变/图片
   背景：支持纯色/渐变/图片
3.


edge_halfplane_mask和edge_wdf_mask的区别：
edge_halfplane_mask是用于生成一个edge的半平面的mask
edge_wdf_mask是用于生成一个edge的距离场(width distance-field)的mask