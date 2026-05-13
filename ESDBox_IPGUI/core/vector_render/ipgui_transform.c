/*
 * MIT License
 *
 * Copyright (c) 2025 JinYiCheng
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "ipgui_transform.h"
#include "ipgui_memory.h"

/* 二维平移变换 */
__IPGUI_API__ void ipgui_2d_translation(ipgui_matrix_t * matrix, float x, float y, float z)
{

}

/* 二维缩放变换 */
__IPGUI_API__ void ipgui_2d_scale(ipgui_matrix_t * matrix, float x, float y, float z)
{

}

/* 二维旋转变换 */
__IPGUI_API__ void ipgui_2d_rotation(ipgui_matrix_t * matrix, ipgui_coord_t * x, ipgui_coord_t * y, float angle)
{

}

/* 二维反射变换 */
__IPGUI_API__ void ipgui_2d_reflection(ipgui_matrix_t * matrix, ipgui_coord_t * x, ipgui_coord_t * y, float angle)
{

}

/* 二维错切变换 */
__IPGUI_API__ void ipgui_2d_shear(ipgui_matrix_t * matrix, ipgui_coord_t * x, ipgui_coord_t * y, float angle)
{
    
}

__IPGUI_API__ void ipgui_2d_transfrom_generate(ipgui_matrix_t * matrix, ipgui_coord_t * x, ipgui_coord_t * y, float angle)
{

}

__IPGUI_API__ void ipgui_2d_trans_init(ipgui_matrix_t * mat, float a, float b, float c, float d, float tx, float ty)
{
    mat->a = a;
    mat->b = b;
    mat->c = c;
    mat->d = d;
    mat->tx = tx;
    mat->ty = ty;
}

/* generate 2d identify matrix */
__IPGUI_API__ void ipgui_2d_identity_generate(ipgui_matrix_t * mat)
{
    mat->a = 1;
    mat->b = 0;
    mat->c = 0;
    mat->d = 1;
    mat->tx = 0;
    mat->ty = 0;
}

/*    the invert matrix format is       */
/* d/ad-bc   -b/ad-bc   b*ty-d*tx/ad-bc */
/* -c/ad+bc   d/ad-bc   c*tx-a*ty/ad-bc */
/* 0         0          1               */
__IPGUI_API__ int ipgui_transform_invert(ipgui_matrix_t * mat)
{   
    float a = mat->a;
    float b = mat->b;
    float c = mat->c;
    float d = mat->d;
    float tx = mat->tx;
    float ty = mat->ty;
    float det = mat->a * mat->d - mat->b * mat->c;
    if (det == 0)
        return -1;

    det = 1 / det;
    mat->a = d * det;
    mat->b = -b * det;
    mat->c = -c * det;
    mat->d = a * det;
    mat->tx = (b * ty - d * tx) * det;
    mat->ty = (c * tx - a * ty) * det;

    return 0;
}

/* generate 2d translation matrix */
__IPGUI_API__ void ipgui_2d_translation_generate(ipgui_matrix_t * mat, float tx, float ty)
{
    mat->a = 1;
    mat->b = 0;
    mat->tx = tx;
    mat->c = 0;
    mat->d = 1;
    mat->ty = ty;
}

/* generate 2d scale matrix */
__IPGUI_API__ void ipgui_2d_scale_generate(ipgui_matrix_t * mat, float sx, float sy)
{
    mat->a = sx;
    mat->b = 0;
    mat->c = 0;
    mat->d = sy;
    mat->tx = 0;
    mat->ty = 0;
}

#include <math.h>
/* generate 2d rotate matrix */
__IPGUI_API__ void ipgui_2d_rotate_generate(ipgui_matrix_t * mat, float r)
{
    float c = cos(r);
    float s = sin(r);

    mat->a = c;
    mat->b = -s;
    mat->tx = 0;
    mat->c = s;
    mat->d = c;
    mat->ty = 0;
}

/* apply transformation matrix to point */
void ipgui_2d_coord_transform(ipgui_matrix_t * m, ipgui_coord_t x, ipgui_coord_t y, ipgui_coord_t * rx, ipgui_coord_t * ry)
{
    float x1, y1;
    x1 = m->a * x + m->b * y + m->tx;
    y1 = m->c * x + m->d * y + m->ty;

    * rx = (x1 > 0) ? (ipgui_coord_t)(x1 + 0.5f) : (ipgui_coord_t)(x1);
    * ry = (y1 > 0) ? (ipgui_coord_t)(y1 + 0.5f) : (ipgui_coord_t)(y1);
}

/* multiply two transformation matrix */
void ipgui_transform_multiply(ipgui_matrix_t * m, ipgui_matrix_t * m1, ipgui_matrix_t * m2)
{
	ipgui_matrix_t t;

	t.a = m1->a * m2->a + m1->b * m2->c;
    t.b = m1->a * m2->b + m1->b * m2->d;
    t.c = m1->c * m2->a + m1->d * m2->c;
    t.d = m1->c * m2->b + m1->d * m2->d;
    t.tx = m1->a * m2->tx + m1->b * m2->ty + m1->tx;
    t.ty = m1->c * m2->tx + m1->d * m2->ty + m1->ty;

	ipgui_memcpy(m, &t, sizeof(ipgui_matrix_t));
}

/* generate composite transformation matrix */
void ipgui_composite_transform_generate(ipgui_matrix_t * m, ipgui_matrix_t m_a[], int n)
{
    while(n --)
    {
        ipgui_transform_multiply(m, m, &m_a[n]);
    }
}

ipgui_matrix_t * ipgui_matrix_translate(ipgui_matrix_t* m, ipgui_coord_t x, ipgui_coord_t y)
{
    float a = m->a, c = m->c, b = m->b, d = m->d, tx = m->tx, ty = m->ty;

    m->a = a;
    m->c = c;
    m->b = b;
    m->d = d;
    m->tx = a * x + b * y + tx;
    m->ty = c * x + d * y + ty;

    return m;
}

ipgui_matrix_t * ipgui_matrix_scale(ipgui_matrix_t* m, float sx, float sy)
{
    float a = m->a, c = m->c, b = m->b, d = m->d, tx = m->tx, ty = m->ty;
    m->a = a * sx;
    m->c = c * sx;
    m->b = b * sy;
    m->d = d * sy;
    m->tx = tx;
    m->ty = ty;

    return m;
}

ipgui_matrix_t * ipgui_matrix_rotate(ipgui_matrix_t* m, float rad)
{
    float a = m->a, c = m->c, b = m->b, d = m->d, tx = m->tx, ty = m->ty, s = sin(rad),
        co = cos(rad);

    m->a = a * co + b * s;
    m->c = co * co + d * s;
    m->b = a * -s + b * co;
    m->d = co * -s + d * co;
    m->tx = tx;
    m->ty = ty;

    return m;
}

//画布（Canvas）的概念类似于视口，它定义了绘图的区域和上下文。设置视口、平移、缩放和裁剪的方法。
/* 窗视口变换 */
int ipgui_apply_window_to_viewport_transform()
{

}