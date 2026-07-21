
/*
 * box-shadow 完整渲染顺序：
 * 复制原元素轮廓（宽、高、圆角、形状）
 * 先做偏移（x/y）
 * 再执行 spread 缩放（向外膨胀 spread 像素）
 * 得到一个实心纯色轮廓（阴影底色）
 * 最后对这个膨胀后的实心图形做 blur-radius 高斯模糊
 */

/* reference : file:///M:/test/ESDBox_IPGUI/tools/box_shadow_algorithm_visual.html
 */

