#include "ipgui_utils.h"
#include "ipgui_types.h"
#include "ipgui_defs.h"

IPGUI_HEADER_BEGIN  /* This is a header guard. It prevents the file from being included more than once. */ _______________MARKER_______________
#define USE_BINARY_HEAP_STATIC 0

typedef struct binary_heap_st
{
    unsigned short usItemSize;
    unsigned short usItemMax;
    unsigned short usItemCnt;
    int (* pfCompare)(void *, void *);
    unsigned char pbPool[];
}bhp_t;

typedef unsigned char ipgui_prio_t;

typedef struct
{
    unsigned short usFreeSlot;
}for_alloc_t;

typedef struct 
{
    ipgui_prio_t prio;
    unsigned int usSlotIdx;
}prio_map_slot_t;

typedef struct ipgui_queue_t {
    unsigned short usFirstFreeSlot;
    for_alloc_t * pstFreeSlot;
    bhp_t * stBhp;
    unsigned short usItemSize;
    unsigned short usItemCnt;
    unsigned int short usItemUsed;
    unsigned char pbDpool[];
} ipgui_queue_t;

/* binary heap */
extern __IPGUI_API__ int binary_heap_init(bhp_t *, unsigned short, unsigned short, int (*)(void *, void *));

#if USE_BINARY_HEAP_STATIC == 0
extern __IPGUI_API__ bhp_t * binary_heap_create(unsigned short, unsigned short, int (*)(void *, void *));
extern __IPGUI_API__ void binary_heap_destroy(bhp_t *);
#endif

extern __IPGUI_API__ int binary_heap_insert(bhp_t *, void *, unsigned short);

extern __IPGUI_API__ int binary_heap_fetch(bhp_t *, void *, unsigned short);

/* queue v1 */
extern __IPGUI_API__ __IPGUI_INIT__ ipgui_err_t ipgui_queue_create(ipgui_queue_t **, unsigned short, unsigned short);

extern __IPGUI_API__ __IPGUI_DEINIT__ ipgui_err_t ipgui_queue_destroy(ipgui_queue_t **);

extern __IPGUI_API__ ipgui_err_t ipgui_queue_publish(ipgui_queue_t *, void *, unsigned short, ipgui_prio_t);

extern __IPGUI_API__ ipgui_err_t ipgui_queue_subscribe(ipgui_queue_t *, void *, unsigned short);

IPGUI_HEADER_END  /* This is the end of the header guard. */ _______________MARKER_______________