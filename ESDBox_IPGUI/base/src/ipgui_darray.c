#include "ipgui_darray.h"
#include "ipgui_memory.h"

void * ipgui_darray_alloc_default(u32_t size, ipgui_darray_t * darray)
{
    return ipgui_mem_alloc(ipgui_smem, size);
}

void * ipgui_darray_free_default(void * p, ipgui_darray_t * darray)
{
    ipgui_mem_free(ipgui_smem, p);
}

void * ipgui_darray_realloc_default(u32_t size, ipgui_darray_t * darray)
{
    return ipgui_mem_realloc(ipgui_smem, darray->elements, size);
}

/* initialize the array */
__IPGUI_API__ void ipgui_darray_init(ipgui_darray_t * darray, u32_t elem_size)
{
    darray->size = 0;
    darray->elem_num = 0;
    darray->elem_size = elem_size;
    darray->elements = (s8_t *)0;
    darray->alloc = (darray_alloc)ipgui_darray_alloc_default;
    darray->free = (darray_free)ipgui_darray_free_default;
    darray->realloc = (darray_realloc)ipgui_darray_realloc_default;
}

__IPGUI_API__ void ipgui_darray_deinit(ipgui_darray_t * darray)
{
    darray->size = 0;
    darray->elem_num = 0;
    darray->elem_size = 0;
    ipgui_mem_free(ipgui_smem, darray->elements);
    darray->elements = (s8_t *)0;
    darray->alloc = (darray_alloc)0;
    darray->free = (darray_free)0;
}

/* get total size of array（能容纳多少个元素） */
__IPGUI_STATIC__ __IPGUI_INLINE__ u32_t ipgui_darray_size(ipgui_darray_t * darray)
{
    return darray->size;
}

/* get number of elements in array（有多少个元素） */
__IPGUI_STATIC__ __IPGUI_INLINE__ u32_t ipgui_darray_num_elements(const ipgui_darray_t * darray)
{
    return darray->elem_num;
}

/* truncate the array to a new size */
__IPGUI_API__ void ipgui_darray_truncate(ipgui_darray_t * darray, u32_t new_elem_size)
{
    if (new_elem_size < darray->elem_num)
    {
        darray->elem_num = new_elem_size;
    }
}

/* get element at index */
/* 必须保证有够量的元素能被index到，否则访问越界 */
__IPGUI_API__ void * ipgui_darray_index(ipgui_darray_t * darray, u32_t index)
{
    if(darray && darray->elem_num && index < darray->elem_num)
    {
        return (void *)(darray->elements + (uintptr_t)index * darray->elem_size);
    }
    return (void *)0;
}

/* get const element at index */
__IPGUI_API__ const void * ipgui_darray_index_const(const ipgui_darray_t * darray, u32_t index)
{
    if(darray && darray->elem_num && index < darray->elem_num)
    {
        return darray->elements + (uintptr_t)index * darray->elem_size;
    }
    return (void *)0;
}

/* copy element at index to param to */
__IPGUI_API__ void ipgui_darray_element_copy(ipgui_darray_t * darray, u32_t index, void * to)
{
    const void * src = ipgui_darray_index_const(darray, index);
    if(src)
    {
        ipgui_memcpy(to, src, darray->elem_size);
    }
}

/* update element at index */
__IPGUI_API__ void ipgui_darray_element_updata(ipgui_darray_t * darray, u32_t index, void * from)
{
    void * src = ipgui_darray_index(darray, index);
    if(src)
    {
        ipgui_memcpy(src, from, darray->elem_size);
    }
}

/* expand the array to contain the extra num(formal parameter) elements */
__IPGUI_STATIC__ void ipgui_darray_expand_num(ipgui_darray_t * darray, u32_t num, void ** copy_to)
{
    void * new_elements = (void *)0;
    u32_t cur_cap = ipgui_darray_size(darray);
    u32_t need_cap = darray->elem_num + num;
    u32_t new_cap;

    if( need_cap > 2147483647/* s32_t max */ ){
        return;
    }

    if( cur_cap >= need_cap ){
        * copy_to = (void *)(darray->elements + darray->elem_num * darray->elem_size);
        darray->elem_num += num;
        return;
    }

    if( !cur_cap ){
        new_cap = 1;
    }else{
        new_cap = cur_cap << 1;
    }
    while( new_cap < need_cap )
    {
        new_cap <<= 1;
    }
    
    // new_elements = ipgui_mem_realloc(ipgui_smem, darray->elements, new_cap * darray->elem_size);
    new_elements = darray->realloc(new_cap * darray->elem_size, darray);
    
    if( new_elements == (void *)0 ){
        return;
    }
    darray->size = new_cap;
    darray->elements = new_elements;
    darray->elem_num += num;
    * copy_to = (void *)((s8_t *)new_elements + cur_cap * darray->elem_size);
}

/* append elements to array */
/* 添加成功就返回（被添加的这些元素的）起始索引，否则返回-1 */
__IPGUI_API__ s32_t ipgui_darray_element_append(ipgui_darray_t * darray, void * elements, u32_t num)
{
    void * copy_to = (void *)0;
    ipgui_darray_expand_num(darray, num, &copy_to);
    if( copy_to == (void *)0 ){
        return -1;
    }
    ipgui_memcpy(copy_to, elements, num * darray->elem_size);
    return ((s8_t *)copy_to - darray->elements) / darray->elem_size;
}

__IPGUI_API__ void * ipgui_darray_last_element(ipgui_darray_t * darray)
{
    return ipgui_darray_index(darray, darray->elem_num - 1);
}

__IPGUI_API__ void * ipgui_darray_first_element(ipgui_darray_t * darray)
{
    return ipgui_darray_index(darray, 0);
}

/* pop the last element */
__IPGUI_API__ s32_t ipgui_darray_element_pop(ipgui_darray_t * darray, void * to)
{
    void * p;

    if( !to )
        return -1;
    p = ipgui_darray_last_element(darray);
    if( p == (void *)0 ){
        return -1;
    }
    ipgui_darray_element_copy(darray, darray->elem_num - 1, to);
    darray->elem_num --;
    return 0;
}