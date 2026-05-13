#include "ipgui_memory.h"
int main(void)
{
    ipgui_mem_t * ipgui_smem = NULL;
    ipgui_mem_unit_type_t HeapSize = 0;
    ipgui_mem_unit_type_t StartAddr = 0;
    unsigned char pubUsg1 = 0, pubUsg2 = 0;

    volatile void *p1, *p2, *p3, *p4, *p5, *p6, *p7, *p8, *p9, *p10;
    volatile void *p11, *p12, *p13, *p14, *p15, *p16, *p17, *p18, *p19, *p20;

    ipgui_smem = ipgui_mem_init((void*)ipgui_memheap, (void *)(ipgui_memheap + IPGUI_SMEM_SIZE));
    ipgui_mem_statistics_take(ipgui_smem, &HeapSize, &StartAddr, NULL, &pubUsg1, &pubUsg2);

    p1 = ipgui_mem_alloc(ipgui_smem, 100);
    ipgui_mem_statistics_take(ipgui_smem, &HeapSize, &StartAddr, NULL, &pubUsg1, &pubUsg2);
    p2 = ipgui_mem_alloc(ipgui_smem, 20);
    ipgui_mem_statistics_take(ipgui_smem, &HeapSize, &StartAddr, NULL, &pubUsg1, &pubUsg2);
    p3 = ipgui_mem_alloc(ipgui_smem, 800);
    ipgui_mem_statistics_take(ipgui_smem, &HeapSize, &StartAddr, NULL, &pubUsg1, &pubUsg2);
    p4 = ipgui_mem_alloc(ipgui_smem, 465);
    ipgui_mem_statistics_take(ipgui_smem, &HeapSize, &StartAddr, NULL, &pubUsg1, &pubUsg2);
    p5 = ipgui_mem_alloc(ipgui_smem, 48);
    ipgui_mem_statistics_take(ipgui_smem, &HeapSize, &StartAddr, NULL, &pubUsg1, &pubUsg2);
    p6 = ipgui_mem_alloc(ipgui_smem, 12);
    ipgui_mem_statistics_take(ipgui_smem, &HeapSize, &StartAddr, NULL, &pubUsg1, &pubUsg2);
    p7 = ipgui_mem_alloc(ipgui_smem, 489);
    ipgui_mem_statistics_take(ipgui_smem, &HeapSize, &StartAddr, NULL, &pubUsg1, &pubUsg2);
    p8 = ipgui_mem_alloc(ipgui_smem, 125);
    ipgui_mem_statistics_take(ipgui_smem, &HeapSize, &StartAddr, NULL, &pubUsg1, &pubUsg2);
    p9 = ipgui_mem_alloc(ipgui_smem, 88);
    ipgui_mem_statistics_take(ipgui_smem, &HeapSize, &StartAddr, NULL, &pubUsg1, &pubUsg2);
    p10 = ipgui_mem_alloc(ipgui_smem, 79);
    ipgui_mem_statistics_take(ipgui_smem, &HeapSize, &StartAddr, NULL, &pubUsg1, &pubUsg2);

    ipgui_mem_free(ipgui_smem, p8);
    ipgui_mem_statistics_take(ipgui_smem, &HeapSize, &StartAddr, NULL, &pubUsg1, &pubUsg2);
    ipgui_mem_free(ipgui_smem, p7);
    ipgui_mem_statistics_take(ipgui_smem, &HeapSize, &StartAddr, NULL, &pubUsg1, &pubUsg2);
    ipgui_mem_free(ipgui_smem, p3);
    ipgui_mem_statistics_take(ipgui_smem, &HeapSize, &StartAddr, NULL, &pubUsg1, &pubUsg2);
    ipgui_mem_free(ipgui_smem, p10);
    ipgui_mem_statistics_take(ipgui_smem, &HeapSize, &StartAddr, NULL, &pubUsg1, &pubUsg2);

    p11 = ipgui_mem_alloc(ipgui_smem, 324);
    ipgui_mem_statistics_take(ipgui_smem, &HeapSize, &StartAddr, NULL, &pubUsg1, &pubUsg2);

    p12 = ipgui_mem_alloc(ipgui_smem, 1);
    ipgui_mem_statistics_take(ipgui_smem, &HeapSize, &StartAddr, NULL, &pubUsg1, &pubUsg2);

    p13 = ipgui_mem_alloc(ipgui_smem, 700);
    ipgui_mem_statistics_take(ipgui_smem, &HeapSize, &StartAddr, NULL, &pubUsg1, &pubUsg2);

    ipgui_mem_free(ipgui_smem, p1);
    ipgui_mem_statistics_take(ipgui_smem, &HeapSize, &StartAddr, NULL, &pubUsg1, &pubUsg2);

    p14 = ipgui_mem_alloc(ipgui_smem, 366);
    ipgui_mem_statistics_take(ipgui_smem, &HeapSize, &StartAddr, NULL, &pubUsg1, &pubUsg2);

    p15 = ipgui_mem_alloc(ipgui_smem, 92);
    ipgui_mem_statistics_take(ipgui_smem, &HeapSize, &StartAddr, NULL, &pubUsg1, &pubUsg2);

    ipgui_mem_free(ipgui_smem, p4);
    ipgui_mem_statistics_take(ipgui_smem, &HeapSize, &StartAddr, NULL, &pubUsg1, &pubUsg2);

    p16 = ipgui_mem_alloc(ipgui_smem, 12);
    ipgui_mem_statistics_take(ipgui_smem, &HeapSize, &StartAddr, NULL, &pubUsg1, &pubUsg2);

    ipgui_mem_free(ipgui_smem, p9);
    ipgui_mem_statistics_take(ipgui_smem, &HeapSize, &StartAddr, NULL, &pubUsg1, &pubUsg2);
    ipgui_mem_free(ipgui_smem, p5);
    ipgui_mem_statistics_take(ipgui_smem, &HeapSize, &StartAddr, NULL, &pubUsg1, &pubUsg2);

    p17 = ipgui_mem_alloc(ipgui_smem, 489);
    ipgui_mem_statistics_take(ipgui_smem, &HeapSize, &StartAddr, NULL, &pubUsg1, &pubUsg2);

    ipgui_mem_free(ipgui_smem, p2);
    ipgui_mem_statistics_take(ipgui_smem, &HeapSize, &StartAddr, NULL, &pubUsg1, &pubUsg2);
    ipgui_mem_free(ipgui_smem, p6);
    ipgui_mem_statistics_take(ipgui_smem, &HeapSize, &StartAddr, NULL, &pubUsg1, &pubUsg2);

    ipgui_mem_free(ipgui_smem, p17);
    ipgui_mem_statistics_take(ipgui_smem, &HeapSize, &StartAddr, NULL, &pubUsg1, &pubUsg2);

    p6 = ipgui_mem_alloc(ipgui_smem, 20*1024);
    ipgui_mem_statistics_take(ipgui_smem, &HeapSize, &StartAddr, NULL, &pubUsg1, &pubUsg2);

    p18 = ipgui_mem_alloc(ipgui_smem, 153);
    ipgui_mem_statistics_take(ipgui_smem, &HeapSize, &StartAddr, NULL, &pubUsg1, &pubUsg2);

    p19 = ipgui_mem_alloc(ipgui_smem, 4856);
    ipgui_mem_statistics_take(ipgui_smem, &HeapSize, &StartAddr, NULL, &pubUsg1, &pubUsg2);

    ipgui_mem_free(ipgui_smem, p11);
    ipgui_mem_statistics_take(ipgui_smem, &HeapSize, &StartAddr, NULL, &pubUsg1, &pubUsg2);

    p20 = ipgui_mem_alloc(ipgui_smem, 2024);
    ipgui_mem_statistics_take(ipgui_smem, &HeapSize, &StartAddr, NULL, &pubUsg1, &pubUsg2);

    ipgui_mem_free(ipgui_smem, p12);
    ipgui_mem_statistics_take(ipgui_smem, &HeapSize, &StartAddr, NULL, &pubUsg1, &pubUsg2);
    ipgui_mem_free(ipgui_smem, p13);
    ipgui_mem_statistics_take(ipgui_smem, &HeapSize, &StartAddr, NULL, &pubUsg1, &pubUsg2);
    ipgui_mem_free(ipgui_smem, p14);
    ipgui_mem_statistics_take(ipgui_smem, &HeapSize, &StartAddr, NULL, &pubUsg1, &pubUsg2);
    ipgui_mem_free(ipgui_smem, p15);
    ipgui_mem_statistics_take(ipgui_smem, &HeapSize, &StartAddr, NULL, &pubUsg1, &pubUsg2);
    ipgui_mem_free(ipgui_smem, p16);
    ipgui_mem_statistics_take(ipgui_smem, &HeapSize, &StartAddr, NULL, &pubUsg1, &pubUsg2);
    ipgui_mem_free(ipgui_smem, p18);
    ipgui_mem_statistics_take(ipgui_smem, &HeapSize, &StartAddr, NULL, &pubUsg1, &pubUsg2);
    ipgui_mem_free(ipgui_smem, p19);
    ipgui_mem_statistics_take(ipgui_smem, &HeapSize, &StartAddr, NULL, &pubUsg1, &pubUsg2);
    ipgui_mem_free(ipgui_smem, p20);
    ipgui_mem_statistics_take(ipgui_smem, &HeapSize, &StartAddr, NULL, &pubUsg1, &pubUsg2);

    ipgui_mem_free(ipgui_smem, p6);
    ipgui_mem_statistics_take(ipgui_smem, &HeapSize, &StartAddr, NULL, &pubUsg1, &pubUsg2);
    return 0;
}