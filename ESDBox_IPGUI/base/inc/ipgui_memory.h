#ifndef IPGUI_MEMORY_H
#define IPGUI_MEMORY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ipgui_utils.h"
#include "ipgui_list.h"
#include "ipgui_conf.h"

#define ipgui_mem_unit_type_t uintptr_t

typedef struct stHeapControlBlock ipgui_mem_t;

#define IPGUI_MEM_STATISTICS_EN 1

#define IPGUI_MEM_ALLOC_PATTERN_AUTO_SWITCH 0

typedef enum {
	BEST_FIT,							
	FIRST_FIT,							
	WORST_FIT,							
	MAX_PATTERN,						
}AllocStrategy_t;

typedef struct{
	AllocStrategy_t alloc_pttn;
	void * (*php_alloc[MAX_PATTERN])( ipgui_mem_t *, ipgui_mem_unit_type_t);
}alloc_stg_t;

typedef struct stHeapControlBlock{
	struct list_head hook;
	struct list_head stFreeEntry;
	ipgui_mem_unit_type_t HeapSize;
	ipgui_mem_unit_type_t ValidSize;
	ipgui_mem_unit_type_t StartAddr;
	alloc_stg_t strat;
#if IPGUI_MEM_STATISTICS_EN == 1
	/* eg1: usage_int = 1 , bUsageFlo = 9 --------->1.09%  */
	/* eg2: usage_int = 23, bUsageFlo = 68--------->23.68% */
    /* eg3: usage_int = 99, bUsageFlo = 0 --------->99.00% */
	char bUsageInt;
	char bUsageFlo;
#endif
}ipgui_mem_t;

typedef struct{ 
	struct list_head stBlockLink;
	ipgui_mem_unit_type_t BlockSize; 
}linknode_t;

struct usdinfo{
	ipgui_mem_unit_type_t UsedSize; 
};

extern ipgui_mem_t * ipgui_smem;

extern u8_t ipgui_memheap[IPGUI_SMEM_SIZE];

extern __IPGUI_API__ ipgui_mem_t *  ipgui_mem_init(void *,void *);

extern __IPGUI_API__ void *         ipgui_mem_alloc(ipgui_mem_t *, ipgui_mem_unit_type_t);

extern __IPGUI_API__ void *         ipgui_mem_alloc_align(ipgui_mem_t *, ipgui_mem_unit_type_t **, ipgui_mem_unit_type_t, int);

extern __IPGUI_API__ void *         ipgui_mem_calloc(ipgui_mem_t *, ipgui_mem_unit_type_t);

extern __IPGUI_API__ void           ipgui_mem_free(ipgui_mem_t *, void *);

extern __IPGUI_API__ ipgui_err_t    ipgui_mem_statistics_take(ipgui_mem_t *, ipgui_mem_unit_type_t *, ipgui_mem_unit_type_t *, u8_t *, u8_t *, u8_t *);

extern __IPGUI_API__ void *         ipgui_mem_realloc(ipgui_mem_t *, void *, ipgui_mem_unit_type_t);

extern __IPGUI_API__ int            ipgui_strlen(const char *);

extern __IPGUI_API__ void           ipgui_memset(void *, u8_t, u32_t);

extern __IPGUI_API__ void           ipgui_memset_0(void *, u32_t);

extern __IPGUI_API__ void           ipgui_memcpy(void *, const void *, u32_t);

extern __IPGUI_API__ void           ipgui_mem_usage_statistics_take(u8_t *, u8_t *);

extern __IPGUI_API__ void *         ipgui_mem_alloc_def(ipgui_mem_unit_type_t size);

extern __IPGUI_API__ void           ipgui_mem_free_def(void * p);

extern __IPGUI_API__ __IPGUI_INIT__ ipgui_err_t ipgui_mem_module_init(void);

#ifdef __cplusplus
}
#endif

#endif