#ifndef IPGUI_MEMPOOL_H
#define IPGUI_MEMPOOL_H

#ifdef __cplusplus
extern "C" {
#endif

#define MEM_ALIGN_SIZE          4

#define SL_BITMAP_WIDTH         16
#define FL_BITMAP_VALID_SHIFT   7                               /* define minimum block size 128B */
#define FL_BITMAP_VALID_WIDTH   (32 - FL_BITMAP_VALID_SHIFT)    /* define first level valid width */
#define BLK_SIZE_MIN            (1 << FL_BITMAP_VALID_SHIFT)

#if SL_BITMAP_WIDTH > 32
#error "SL_BITMAP_WIDTH must be less than 32"
#endif

#define ipgui_offset_of(type, field) ((unsigned int)&(((type *)0)->field))

typedef struct ipgui_mem_blk_st ipgui_mem_blk_t;
typedef struct ipgui_mem_blk_st {
    ipgui_mem_blk_t * prev_phy;  
    unsigned int size;
    ipgui_mem_blk_t * next;           /* logical block */
    ipgui_mem_blk_t * prev;           /* logical block */
}ipgui_mem_blk_t;

typedef struct {
    unsigned int fl_bmp;
    unsigned int sl_bmp[FL_BITMAP_VALID_WIDTH];
    ipgui_mem_blk_t * free[FL_BITMAP_VALID_WIDTH][SL_BITMAP_WIDTH];
}ipgui_mem_mng_t;

extern int    ipgui_mempool_init  (ipgui_mem_mng_t *, void *, unsigned int);
extern void * ipgui_mempool_alloc (ipgui_mem_mng_t *, unsigned int);
extern void   ipgui_mempool_free  (ipgui_mem_mng_t *, void *);
extern void * ipgui_mempool_calloc(ipgui_mem_mng_t *, unsigned int);

#ifdef __cplusplus
}
#endif

#endif