#ifndef IPGUI_DARRAY_H
#define IPGUI_DARRAY_H

#include "ipgui_utils.h"

typedef struct  ipgui_dynamic_array_context ipgui_darray_t;

typedef void * (* darray_alloc)(u32_t, ipgui_darray_t *);

typedef void * (* darray_free)(void *, ipgui_darray_t *);

typedef void * (* darray_realloc)(u32_t, ipgui_darray_t *);

typedef struct  ipgui_dynamic_array_context
{
    u32_t size;      /* number of elements can put in array */   /* 总共能容纳的元素数目 */

    u32_t elem_num;  /* number of elements in array */           /* 当前数组中已经容纳的元素的数目 */

    u32_t elem_size; /* size(bytes) of each element */           /* 每个元素的大小 */

    s8_t * elements;        /* pointer to first element */              /* 指向数组中第一个元素的指针 */

    darray_alloc alloc;

    darray_free free;

    darray_realloc realloc;
}ipgui_darray_t;

extern __IPGUI_API__ void ipgui_darray_init(ipgui_darray_t * darray, u32_t elem_size);

extern __IPGUI_API__ void ipgui_darray_deinit(ipgui_darray_t * darray);

extern __IPGUI_API__ void ipgui_darray_truncate(ipgui_darray_t * darray, u32_t new_elem_size);

extern __IPGUI_API__ void * ipgui_darray_index(ipgui_darray_t * darray, u32_t index);

extern __IPGUI_API__ const void * ipgui_darray_index_const(const ipgui_darray_t * darray, u32_t index);

extern __IPGUI_API__ void ipgui_darray_element_copy(ipgui_darray_t * darray, u32_t index, void * to);

extern __IPGUI_API__ void ipgui_darray_element_updata(ipgui_darray_t * darray, u32_t index, void * from);

extern __IPGUI_API__ s32_t ipgui_darray_element_append(ipgui_darray_t * darray, void * elements, u32_t num);

extern __IPGUI_API__ void * ipgui_darray_last_element(ipgui_darray_t * darray);

extern __IPGUI_API__ void * ipgui_darray_first_element(ipgui_darray_t * darray);

extern __IPGUI_API__ s32_t ipgui_darray_element_pop(ipgui_darray_t * darray, void * to);

#endif