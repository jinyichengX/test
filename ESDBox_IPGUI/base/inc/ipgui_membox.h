#ifndef IPGUI_MEMBOX_H
#define IPGUI_MEMBOX_H

#include "ipgui_utils.h"

typedef struct blknode_ctx blk_node_t;

typedef struct blknode_ctx
{
	blk_node_t * next;
}blk_node_t;

typedef struct
{
	u16_t blk_sz;
    s32_t blk_cnt;
    s32_t blk_used;
	blk_node_t block;
}ipgui_membox_t IPGUI_ST_ALIGN(IPGUI_MEM_ALIGN_SIZE);

extern __IPGUI_API__ ipgui_err_t ipgui_membox_init(ipgui_membox_t *, void *, u16_t, s32_t);
extern __IPGUI_API__ ipgui_membox_t * ipgui_membox_create(u16_t, s32_t);
extern __IPGUI_API__ void ipgui_membox_destroy(ipgui_membox_t *);
extern __IPGUI_API__ void * ipgui_membox_alloc(ipgui_membox_t *);
extern __IPGUI_API__ void ipgui_membox_free(ipgui_membox_t *, void *);
extern __IPGUI_API__ void ipgui_membox_expand(ipgui_membox_t * mb, void * mem, u16_t blk_sz, s32_t blk_cnt);

#endif