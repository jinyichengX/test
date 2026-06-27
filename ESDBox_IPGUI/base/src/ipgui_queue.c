#include "ipgui_queue.h"
#include "ipgui_memory.h"

#define LCHILD_IDX(i)           (((i) << 1) + 1)
#define RCHILD_IDX(i)           (((i) + 1) << 1)
#define PARENT_IDX(i)           (((i) - 1) / 2)
#define BHP_ITEM(bh,n)          ((void *)((bh)->pbPool + (bh)->usItemSize * (n)))

#define FREE_SlOT_IDX_INVALID ((u16_t)(-1))

__IPGUI_STATIC__ __IPGUI_INLINE__ void ipgui_queue_item_increase(ipgui_queue_t * pstQ)
{
    ++ pstQ->usItemUsed;
}

__IPGUI_STATIC__ __IPGUI_INLINE__ void ipgui_queue_item_decrease(ipgui_queue_t * pstQ)
{
    -- pstQ->usItemUsed;
}

__IPGUI_STATIC__ __IPGUI_INLINE__ s32_t ipgui_queue_is_full(ipgui_queue_t * pstQ)
{
    return (pstQ->usItemUsed == pstQ->usItemCnt);
}

__IPGUI_STATIC__ __IPGUI_INLINE__ s32_t ipgui_queue_is_empty(ipgui_queue_t * pstQ)
{
    return !pstQ->usItemUsed;
}

__IPGUI_STATIC__ __IPGUI_INLINE__ void ipgui_queue_get_free_slot(ipgui_queue_t * pstQ, u16_t * pusSlot)
{
    u32_t usFirstSlot = pstQ->usFirstFreeSlot;
    pstQ->usFirstFreeSlot = pstQ->pstFreeSlot[usFirstSlot].usFreeSlot;
    * pusSlot = usFirstSlot;
}

__IPGUI_STATIC__ __IPGUI_INLINE__ void ipgui_queue_return_free_slot(ipgui_queue_t * pstQ, u16_t usSlot)
{
    pstQ->pstFreeSlot[usSlot].usFreeSlot = pstQ->usFirstFreeSlot;
    pstQ->usFirstFreeSlot = usSlot;
}

__IPGUI_STATIC__ __IPGUI_INLINE__ void ipgui_queue_item_push(ipgui_queue_t * pstQ, u16_t usSlot, void * pvItem)
{   
    ipgui_memcpy(&pstQ->pbDpool[usSlot * pstQ->usItemSize], pvItem, pstQ->usItemSize);
}

__IPGUI_STATIC__ __IPGUI_INLINE__ void ipgui_queue_item_fetch(ipgui_queue_t * pstQ, u16_t usSlot, void * pvItem)
{   
    ipgui_memcpy(pvItem, &pstQ->pbDpool[usSlot * pstQ->usItemSize], pstQ->usItemSize);
}

__IPGUI_STATIC__ __IPGUI_INLINE__ s32_t binary_heap_full(bhp_t * b)
{
    return b->usItemCnt == b->usItemMax;
}

__IPGUI_STATIC__ __IPGUI_INLINE__ s32_t binary_heap_empty(bhp_t * b)
{
    return b->usItemCnt == 0;
}

__IPGUI_STATIC__ __IPGUI_INLINE__ void binary_heap_read(bhp_t * b, void * pvItem)
{
    ipgui_memcpy(pvItem, BHP_ITEM(b, 0), b->usItemSize);
}

__IPGUI_STATIC__ __IPGUI_INLINE__ void binary_heap_write(bhp_t * b, void * pvItem)
{
    ipgui_memcpy(BHP_ITEM(b, b->usItemCnt), pvItem, b->usItemSize);
}

__IPGUI_STATIC__ __IPGUI_INLINE__ void binary_heap_item_increase(bhp_t * b)
{
    ++ b->usItemCnt;
}

__IPGUI_STATIC__ __IPGUI_INLINE__ void binary_heap_item_decrease(bhp_t * b)
{
    -- b->usItemCnt;
}

/* init binary heap */
__IPGUI_API__ s32_t binary_heap_init(bhp_t * pstBhp, u16_t usItemSize, u16_t usItemMax, s32_t (*pfCompare)(void *, void *))
{
    if ((bhp_t *)0 == pstBhp || 0 == usItemSize || 0 == pfCompare)
        goto _return;

    pstBhp->usItemCnt = 0;
    pstBhp->pfCompare = pfCompare;
    pstBhp->usItemMax = usItemMax;
    pstBhp->usItemSize = usItemSize;
    return 0;
_return:
    return -1;
}

#if USE_BINARY_HEAP_STATIC == 0
/* create binary heap */
__IPGUI_API__ bhp_t * binary_heap_create(u16_t usItemSize, u16_t usItemMax, s32_t (*pfCompare)(void *, void *))
{
    bhp_t * pstBhp = (bhp_t *)0;

    if(usItemSize == 0 || usItemMax == 0)
        goto _return;

    if((bhp_t *)0 == (pstBhp = (bhp_t *)ipgui_mem_alloc(ipgui_smem, sizeof(bhp_t) + usItemSize * usItemMax)))
        goto _return;

    if(0 != binary_heap_init(pstBhp, usItemSize, usItemMax, pfCompare))
    {
        ipgui_mem_free(ipgui_smem, pstBhp);
        return (bhp_t *)0;
    }
_return:
    return pstBhp;
}

/* destroy binary heap */
__IPGUI_API__ void binary_heap_destroy(bhp_t * pstBhp)
{
    if(pstBhp) 
        ipgui_mem_free(ipgui_smem, pstBhp);
}
#endif

/* insert item */
__IPGUI_API__ s32_t binary_heap_insert(bhp_t * pstBhp, void * pvItem, u16_t usItemSize)
{
    u16_t usPholeIdx;
    u16_t usHoleIdx = 0;
    void * pvPholeItem = (void *)0;
    void * pvHoleItem = (void *)0;

    if(pstBhp == (bhp_t *)0 || usItemSize != pstBhp->usItemSize || pvItem == (void *)0)
        return -1;
    
    if(binary_heap_full(pstBhp))
        return -2;
    
    /* write item to next slot */
    binary_heap_write(pstBhp, pvItem);

    /* do shift up */
    usHoleIdx   = pstBhp->usItemCnt;
    usPholeIdx  = PARENT_IDX(usHoleIdx);
    pvHoleItem  = BHP_ITEM(pstBhp, usHoleIdx);
    pvPholeItem = BHP_ITEM(pstBhp, usPholeIdx);

    while( pstBhp->pfCompare(pvItem, pvPholeItem) && (usHoleIdx != 0))
    {
        pvHoleItem  = BHP_ITEM(pstBhp, usHoleIdx);
        pvPholeItem = BHP_ITEM(pstBhp, usPholeIdx);
        ipgui_memcpy(pvHoleItem, pvPholeItem, usItemSize);
        usHoleIdx   = usPholeIdx;
        usPholeIdx  = PARENT_IDX(usHoleIdx);
        pvPholeItem = BHP_ITEM(pstBhp, usPholeIdx);
    }
    pvHoleItem = BHP_ITEM(pstBhp, usHoleIdx);
    ipgui_memcpy(pvHoleItem, pvItem, usItemSize);

    /* item number increase */
    binary_heap_item_increase(pstBhp);

    return 0;
}

/* fetch first item */
__IPGUI_API__ s32_t binary_heap_fetch(bhp_t * pstBhp, void * pvItem, u16_t usItemSize)
{
    u16_t usLchdIdx, usRchdIdx, usHoleIdx, usChdIdx;
    void * pvHoleItem, * pvLchdItem, * pvRchdItem, * pvChdItem;
    void * pvLastItem = (void *)0;

    if(pstBhp == (bhp_t *)0 || usItemSize != pstBhp->usItemSize || pvItem == (void *)0)
        return -1;

    if(binary_heap_empty(pstBhp))
        return -2;

    /* read item from last slot */
    binary_heap_read(pstBhp, pvItem);

    /* do shift down */
    usHoleIdx = 0;
    usLchdIdx  = LCHILD_IDX(usHoleIdx);
    usRchdIdx  = RCHILD_IDX(usHoleIdx);
    pvHoleItem = BHP_ITEM(pstBhp, usHoleIdx);
    pvLchdItem = BHP_ITEM(pstBhp, usLchdIdx);
    pvRchdItem = BHP_ITEM(pstBhp, usRchdIdx);

    usChdIdx   = (*(s32_t *)pvLchdItem < *(s32_t *)pvRchdItem) ? usLchdIdx : usRchdIdx;
    pvChdItem  = BHP_ITEM(pstBhp, usChdIdx);

    pvLastItem = BHP_ITEM(pstBhp, pstBhp->usItemCnt - 1);
    while( pstBhp->pfCompare(pvChdItem, pvLastItem) && (usChdIdx < pstBhp->usItemCnt))
    {
        ipgui_memcpy(pvHoleItem, pvChdItem, usItemSize);
        
        usHoleIdx  = usChdIdx;
        usLchdIdx  = LCHILD_IDX(usHoleIdx);
        usRchdIdx  = RCHILD_IDX(usHoleIdx);
        pvHoleItem = BHP_ITEM(pstBhp, usHoleIdx);
        pvLchdItem = BHP_ITEM(pstBhp, usLchdIdx);
        pvRchdItem = BHP_ITEM(pstBhp, usRchdIdx);

        usChdIdx   = pstBhp->pfCompare(pvLchdItem, pvRchdItem) ? usLchdIdx : usRchdIdx;
        pvChdItem  = BHP_ITEM(pstBhp, usChdIdx);
    }
    ipgui_memcpy(pvHoleItem, pvLastItem, usItemSize);

    /* item number decrease */
    binary_heap_item_decrease(pstBhp);

    return 0;
}

/* compare function for binary heap */
__IPGUI_STATIC__ s32_t ipgui_queue_prio_compare(void * pvMap1, void * pvMap2)
{
    prio_map_slot_t * pstSlot1 = (prio_map_slot_t *)pvMap1;
    prio_map_slot_t * pstSlot2 = (prio_map_slot_t *)pvMap2;
    return pstSlot1->prio < pstSlot2->prio;
}

/* 不用 */
#if 0
__IPGUI_STATIC__ s32_t ipgui_queue_prio_compare1(void * pvMap1, void * pvMap2)
{
    prio_map_slot_t * pstSlot1 = (prio_map_slot_t *)pvMap1;
    prio_map_slot_t * pstSlot2 = (prio_map_slot_t *)pvMap2;
    return pstSlot1->prio < pstSlot2->prio;
}
#endif

/* create queue */
__IPGUI_API__ __IPGUI_INIT__ ipgui_err_t ipgui_queue_create(ipgui_queue_t ** ppstQ, u16_t usItemSize, u16_t usItemCnt)
{
    void * pvSlots;
    bhp_t * pstBhp;
    * ppstQ = (ipgui_queue_t *)0;

    if( !usItemSize || !usItemCnt )
        return IPGUI_ERR_PARAM;

    * ppstQ = ipgui_mem_alloc(ipgui_smem, sizeof(ipgui_queue_t) + usItemSize * usItemCnt);
    if(* ppstQ == (ipgui_queue_t *)0)  
        return IPGUI_ERR_MEM;

    pvSlots = ipgui_mem_alloc(ipgui_smem, sizeof(for_alloc_t) * usItemCnt);
    if( pvSlots == (void *)0 ){
        ipgui_mem_free(ipgui_smem, (void *)(* ppstQ));
        return IPGUI_ERR_MEM;
    }
    pstBhp = (bhp_t *)ipgui_mem_alloc(ipgui_smem, sizeof(bhp_t) + sizeof(prio_map_slot_t) * usItemCnt);
    if( pstBhp == (bhp_t *)0 ){
        ipgui_mem_free(ipgui_smem, (void *)(* ppstQ));
        ipgui_mem_free(ipgui_smem, (void *)pvSlots);
        return IPGUI_ERR_MEM;
    }

    (* ppstQ)->pstFreeSlot = (for_alloc_t *)pvSlots;
    (* ppstQ)->usItemSize = usItemSize;
    (* ppstQ)->usItemCnt  = usItemCnt;
    (* ppstQ)->usItemUsed = 0;
    (* ppstQ)->stBhp = pstBhp;
    binary_heap_init((* ppstQ)->stBhp, sizeof(prio_map_slot_t), usItemCnt, ipgui_queue_prio_compare);
    (* ppstQ)->usFirstFreeSlot = 0;
    for( s32_t i = 0; i < usItemCnt; ++ i )
    {
        (* ppstQ)->pstFreeSlot[i].usFreeSlot = i + 1;
    }
    (* ppstQ)->pstFreeSlot[usItemCnt - 1].usFreeSlot = FREE_SlOT_IDX_INVALID;

    return IPGUI_ERR_OK;
}

/* destroy queue */
__IPGUI_API__ __IPGUI_DEINIT__ ipgui_err_t ipgui_queue_destroy(ipgui_queue_t ** ppstQ)
{
    if( !ppstQ || !* ppstQ )
        return IPGUI_ERR_PARAM;

    ipgui_mem_free(ipgui_smem, (void *)(* ppstQ)->pstFreeSlot);
    ipgui_mem_free(ipgui_smem, (void *)(* ppstQ)->stBhp);
    ipgui_mem_free(ipgui_smem, (void *)(* ppstQ));
    * ppstQ = (ipgui_queue_t *)0;
    
    return IPGUI_ERR_OK;
}

/* publish item to queue */
__IPGUI_API__ ipgui_err_t ipgui_queue_publish(ipgui_queue_t * pstQ, void * pvItem, u16_t usItemSize, ipgui_prio_t prio)
{
    u16_t usSlot;
    prio_map_slot_t stMap;

    if( usItemSize != pstQ->usItemSize )
        return IPGUI_ERR_PARAM;

    if( ipgui_queue_is_full(pstQ) )
        return IPGUI_ERR_QUEUE_FULL;

    ipgui_queue_get_free_slot(pstQ, &usSlot);
    ipgui_queue_item_push(pstQ, usSlot, pvItem);
    stMap.prio = prio;
    stMap.usSlotIdx = usSlot;

    binary_heap_insert(pstQ->stBhp, (void *)&stMap, sizeof(prio_map_slot_t));
    ipgui_queue_item_increase(pstQ);

    return IPGUI_ERR_OK;
}

/* fetch item from queue */
__IPGUI_API__ ipgui_err_t ipgui_queue_subscribe(ipgui_queue_t * pstQ, void * pvItem, u16_t usItemSize)
{    
    u16_t usSlot;
    prio_map_slot_t stMap;

    if( usItemSize != pstQ->usItemSize )
        return IPGUI_ERR_PARAM;

    if( ipgui_queue_is_empty(pstQ) )
        return IPGUI_ERR_QUEUE_EMPTY;

    binary_heap_fetch(pstQ->stBhp, (void *)&stMap, sizeof(prio_map_slot_t));
    ipgui_queue_return_free_slot(pstQ, stMap.usSlotIdx);
    ipgui_queue_item_fetch(pstQ, stMap.usSlotIdx, pvItem);
    ipgui_queue_item_decrease(pstQ);

    return IPGUI_ERR_OK;
}