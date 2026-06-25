#ifndef IPGUI_QUEUE_H
#define IPGUI_QUEUE_H

#include "ipgui_utils.h"
#include "ipgui_types.h"
#include "ipgui_defs.h"

#define USE_BINARY_HEAP_STATIC 0

typedef struct binary_heap_st
{
    u16_t usItemSize;
    u16_t usItemMax;
    u16_t usItemCnt;
    s32_t (* pfCompare)(void *, void *);
    u8_t pbPool[];
}bhp_t;

typedef u8_t ipgui_prio_t;

typedef struct
{
    u16_t usFreeSlot;
}for_alloc_t;

typedef struct 
{
    ipgui_prio_t prio;
    u32_t usSlotIdx;
}prio_map_slot_t;

typedef struct ipgui_queue_t {
    u16_t usFirstFreeSlot;
    for_alloc_t * pstFreeSlot;
    bhp_t * stBhp;
    u16_t usItemSize;
    u16_t usItemCnt;
    u16_t usItemUsed;
    u8_t pbDpool[];
} ipgui_queue_t;

/* binary heap */
extern __IPGUI_API__ s32_t binary_heap_init(bhp_t *, u16_t, u16_t, s32_t (*)(void *, void *));

#if USE_BINARY_HEAP_STATIC == 0
extern __IPGUI_API__ bhp_t * binary_heap_create(u16_t, u16_t, s32_t (*)(void *, void *));
extern __IPGUI_API__ void binary_heap_destroy(bhp_t *);
#endif

extern __IPGUI_API__ s32_t binary_heap_insert(bhp_t *, void *, u16_t);

extern __IPGUI_API__ s32_t binary_heap_fetch(bhp_t *, void *, u16_t);

/* queue v1 */
extern __IPGUI_API__ __IPGUI_INIT__ ipgui_err_t ipgui_queue_create(ipgui_queue_t **, u16_t, u16_t);

extern __IPGUI_API__ __IPGUI_DEINIT__ ipgui_err_t ipgui_queue_destroy(ipgui_queue_t **);

extern __IPGUI_API__ ipgui_err_t ipgui_queue_publish(ipgui_queue_t *, void *, u16_t, ipgui_prio_t);

extern __IPGUI_API__ ipgui_err_t ipgui_queue_subscribe(ipgui_queue_t *, void *, u16_t);

#endif
