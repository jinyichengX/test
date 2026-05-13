#include <stdio.h>
#include "ipgui_queue.h"

/* a为插入的元素，小根堆 */
static int Compare(void * a, void * b)
{
    int * a1 = (int *)a;
    int * b1 = (int *)b;
    return (*a1 < *b1) ? 1:0;
}
int main(void)
{
    bhp_t * pstBhp;
    pstBhp = binary_heap_create(4, 20, Compare);
    uint32_t aaa1 = 64;
    uint32_t aaa2 = 12;
    uint32_t aaa3 = 56;
    uint32_t aaa4 = 1;
    uint32_t aaa5 = 46;
    uint32_t aaa6 = 17;
    uint32_t aaa7 = 22;
    uint32_t aaa8 = 0;
    uint32_t aaa9 = 84;
    uint32_t aaa10 = 9;
    binary_heap_insert(pstBhp, (void *)&aaa1, 4);
    binary_heap_insert(pstBhp, (void *)&aaa2, 4);
    binary_heap_insert(pstBhp, (void *)&aaa3, 4);
    binary_heap_insert(pstBhp, (void *)&aaa4, 4);
    binary_heap_insert(pstBhp, (void *)&aaa5, 4);
    binary_heap_insert(pstBhp, (void *)&aaa6, 4);
    binary_heap_insert(pstBhp, (void *)&aaa7, 4);
    binary_heap_insert(pstBhp, (void *)&aaa8, 4);
    binary_heap_insert(pstBhp, (void *)&aaa9, 4);
    binary_heap_insert(pstBhp, (void *)&aaa10, 4);
    binary_heap_fetch(pstBhp, (void *)&aaa1, 4);
    binary_heap_fetch(pstBhp, (void *)&aaa2, 4);
    binary_heap_fetch(pstBhp, (void *)&aaa3, 4);
    binary_heap_fetch(pstBhp, (void *)&aaa4, 4);
    binary_heap_fetch(pstBhp, (void *)&aaa5, 4);
    binary_heap_fetch(pstBhp, (void *)&aaa6, 4);
    binary_heap_fetch(pstBhp, (void *)&aaa7, 4);
    binary_heap_fetch(pstBhp, (void *)&aaa8, 4);
    binary_heap_fetch(pstBhp, (void *)&aaa9, 4);
    binary_heap_fetch(pstBhp, (void *)&aaa10, 4);
    return 0;
}