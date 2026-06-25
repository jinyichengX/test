/*
 * MIT License
 *
 * Copyright (c) 2024~2025 JinYiCheng
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* this module is stolen from tlsf implementation 
 * ^_^
 */

#include "ipgui_mempool.h"
#include "ipgui_debug.h"

#define ALIGN_UP(x, align)          ((((u32_t)(x)) + (align) - 1) & (~((align) - 1)))

#define BLK_FREE_FLAG_SHIFT          0
#define BLK_PREV_PHY_FREE_FLAG_SHIFT 1

/* mask of block free/prev phy free */
#define BLK_FREE_FLAG_MASK          (1U << BLK_FREE_FLAG_SHIFT)
#define BLK_PREV_PHY_FREE_FLAG_MASK (1U << BLK_PREV_PHY_FREE_FLAG_SHIFT)
#define BLK_FLAG_MASK               (BLK_FREE_FLAG_MASK | BLK_PREV_PHY_FREE_FLAG_MASK)

#define BLOCK_SIZE(x)               (((x)->size) & (~(BLK_FLAG_MASK)))

#define BLK_FIELD_SIZE_SIZE         (ipgui_offset_of(ipgui_mem_blk_t, next) - ipgui_offset_of(ipgui_mem_blk_t, size))

static inline void block_mark_used(ipgui_mem_blk_t * blk)
{
    blk->size &= ~BLK_FREE_FLAG_MASK;
}

static inline void block_prev_phy_mark_used(ipgui_mem_blk_t * blk)
{
    blk->size &= ~BLK_PREV_PHY_FREE_FLAG_MASK;
}

static inline void block_mark_free(ipgui_mem_blk_t * blk)
{
    blk->size |= BLK_FREE_FLAG_MASK;
}

static inline void block_prev_phy_mark_free(ipgui_mem_blk_t * blk)
{
    blk->size |= BLK_PREV_PHY_FREE_FLAG_MASK;
}

static inline s32_t block_is_free(ipgui_mem_blk_t * blk)
{
    return (!!(blk->size & BLK_FREE_FLAG_MASK));
}

static inline s32_t block_prev_phy_is_free(ipgui_mem_blk_t * blk)
{
    return (!!(blk->size & BLK_PREV_PHY_FREE_FLAG_MASK));
}

static inline ipgui_mem_blk_t * ptr_to_blk(void * p)
{
    return (ipgui_mem_blk_t *)((u32_t)p - ipgui_offset_of(ipgui_mem_blk_t, next));
}

static inline void * blk_to_ptr(ipgui_mem_blk_t * blk)
{
    return (void *)((u32_t)blk + ipgui_offset_of(ipgui_mem_blk_t, next));
}

static inline void * blk_to_size(ipgui_mem_blk_t * blk)
{
    return (void *)((u32_t)blk + ipgui_offset_of(ipgui_mem_blk_t, size));
}

static ipgui_mem_blk_t * block_next_phy(ipgui_mem_blk_t * blk)
{
    return (ipgui_mem_blk_t *)((u32_t)blk + BLOCK_SIZE(blk));
}

static inline void set_block_size(ipgui_mem_blk_t * blk, u32_t size)
{
    blk->size = size | (blk->size & BLK_FLAG_MASK);
}

/* find highest bit set(32 ~ 1) */
static s32_t generic_fls(u32_t v)
{
    s32_t ret = 32;
    if (!v) return 0;
    if (!(v & 0xffff0000u)) { v <<= 16; ret -= 16; }
    if (!(v & 0xff000000u)) { v <<= 8;  ret -= 8;  }
    if (!(v & 0xf0000000u)) { v <<= 4;  ret -= 4;  }
    if (!(v & 0xc0000000u)) { v <<= 2;  ret -= 2;  }
    if (!(v & 0x80000000u)) { v <<= 1;  ret -= 1;  }
    return ret;
}

/* find lowest bit set(1 ~ 32) */
static s32_t generic_ffs(u32_t v)
{
    s32_t ret = 1;
    if (!v) return 0;
    if (!(v & 0x0000ffffu)) { ret += 16; v >>= 16; }
    if (!(v & 0x000000ffu)) { ret += 8;  v >>= 8;  }
    if (!(v & 0x0000000fu)) { ret += 4;  v >>= 4;  }
    if (!(v & 0x00000003u)) { ret += 2;  v >>= 2;  }
    if (!(v & 0x00000001u)) { ret += 1;  v >>= 1;  }
    return ret;
}

/* insert to first */
static void block_insert(ipgui_mem_blk_t * blk, ipgui_mem_blk_t ** start)
{
    ipgui_mem_blk_t * first = * start;
    * start = blk;
    blk->next = first;
    blk->prev = (ipgui_mem_blk_t *)0;
    if (first) {
        first->next = (ipgui_mem_blk_t *)0;
        first->prev = blk;
    }
}

/* remove block */
static void block_remove(ipgui_mem_blk_t * blk, ipgui_mem_blk_t ** start)
{
    // DBG_ASSERT(* start, "remove first err, blk is null");

    ipgui_mem_blk_t * first = * start;
    
    if (blk == first) {
        * start = first->next;
        if (* start)
            (* start)->prev = (ipgui_mem_blk_t *)0;
    } else {
        blk->prev->next = blk->next;
        if (blk->next)
            blk->next->prev = blk->prev;
    }
}

/* mapping size to fl and sl */
static void mapping_index(u32_t size, s32_t * fl, s32_t * sl)
{
    s32_t fl_idx, sl_idx;

    fl_idx = generic_fls(size) - 1;
    sl_idx = (size - (1 << fl_idx)) * SL_BITMAP_WIDTH / (1 << fl_idx);
    fl_idx -= FL_BITMAP_VALID_SHIFT;

    *fl = fl_idx;
    *sl = sl_idx;
}

static void insert_free_block(ipgui_mem_mng_t * mem, ipgui_mem_blk_t * blk)
{
    // DBG_ASSERT(block_is_free(blk), "insert free err, blk is used");

    s32_t fl, sl;
    mapping_index(BLOCK_SIZE(blk), &fl, &sl);
    block_insert(blk, &mem->free[fl][sl]);

    /* update fl_bmp and sl_bmp */
    mem->fl_bmp |= (1 << fl);
    mem->sl_bmp[fl] |= (1 << sl);
}

static void remove_free_block(ipgui_mem_mng_t * mem, ipgui_mem_blk_t * blk)
{
    s32_t fl, sl;
    mapping_index(BLOCK_SIZE(blk), &fl, &sl);
    block_remove(blk, &mem->free[fl][sl]);

    /* update fl_bmp and sl_bmp */
    if (mem->free[fl][sl] == (ipgui_mem_blk_t *)0) {
        mem->sl_bmp[fl] &= (~(1 << sl));
    }
    if (0 == mem->sl_bmp[fl]) {
        mem->fl_bmp &= (~(1 << fl));
    }
}

/* at most 4GB memory can be managed */
s32_t ipgui_mempool_init(ipgui_mem_mng_t * mem, void * start, u32_t size)
{
    // DBG_ASSERT(mem && start, "invalid parameter, mem and start must be valid");
	if (size == 0) return 0;
    s32_t valid_size;
    s32_t fl, sl;
    ipgui_mem_blk_t * blk;
    u32_t pool_start =\
    ALIGN_UP((u32_t)start, MEM_ALIGN_SIZE);
    pool_start += ipgui_offset_of(ipgui_mem_blk_t, size);

    /* check pool size */
    valid_size = size - (pool_start - (u32_t)start);
    if (valid_size < BLK_SIZE_MIN) {
        return 1;
    }

    /* init management struct */
    mem->fl_bmp = 0;
    for (s32_t iter = 0; iter < FL_BITMAP_VALID_WIDTH; ++ iter) {
        mem->sl_bmp[iter] = 0;
        for (s32_t idx = 0; idx < SL_BITMAP_WIDTH; ++ idx) {
            mem->free[iter][idx] = (ipgui_mem_blk_t *)0;
        }
    }

    /* insert first whole free block */
    blk = (ipgui_mem_blk_t *)((u32_t)pool_start - ipgui_offset_of(ipgui_mem_blk_t, size));
    
    set_block_size(blk, valid_size);
    block_prev_phy_mark_used(blk);
    block_mark_free(blk);
    insert_free_block(mem, blk);

    return 0;
}

static ipgui_mem_blk_t * find_suitbale_block(ipgui_mem_mng_t * mem, s32_t size,
                                        s32_t * pfl, s32_t * psl)
{
    s32_t fl, sl;
    u32_t fl_bmp, sl_bmp;
    ipgui_mem_blk_t * blk = (ipgui_mem_blk_t *)0;
    mapping_index(size, &fl, &sl);

    sl_bmp = mem->sl_bmp[fl] & ((~0U) << sl);
    if (!sl_bmp)/* fl is 0 */
    {   
        fl_bmp = mem->fl_bmp & ((~0U) << fl);
        if (!fl_bmp) 
        {
            return (ipgui_mem_blk_t *)0;
        }
        fl = generic_ffs(fl_bmp) - 1;
    }

    /* find lowest bit set */
    sl = generic_ffs(mem->sl_bmp[fl]) - 1;
    blk = mem->free[fl][sl];
    * pfl = fl; * psl = sl;

    return blk;
}

static void split_block(ipgui_mem_blk_t * blk, s32_t size, ipgui_mem_blk_t ** pnext)
{
    * pnext = (ipgui_mem_blk_t *)((u32_t)blk_to_size(blk) + size);
    * pnext = (ipgui_mem_blk_t *)((u32_t)(* pnext) - ipgui_offset_of(ipgui_mem_blk_t, size));
}

void * ipgui_mempool_alloc(ipgui_mem_mng_t * mem, u32_t size)
{
    void * p;
    ipgui_mem_blk_t * blk;
    s32_t fl, sl;
    if (!size) return (void *)0;

    /* size needed */
    size = ALIGN_UP(size + BLK_FIELD_SIZE_SIZE, MEM_ALIGN_SIZE);
    if (size < BLK_SIZE_MIN) {
        size = BLK_SIZE_MIN;
    }

    /* find suitbale block */
    blk = find_suitbale_block(mem, size, &fl, &sl);
    if (blk == (ipgui_mem_blk_t *)0) {
        return (void *)0;
    }

    /* if remaining size larger than BLK_MIN, 
     * split block, else allocate all
     */
    if ((BLOCK_SIZE(blk) - size) >= BLK_SIZE_MIN) {
        ipgui_mem_blk_t * next_blk;
        split_block(blk, size, &next_blk);
        remove_free_block(mem, blk);

        /* set size for this allocated block and next phy block */
        set_block_size(next_blk, BLOCK_SIZE(blk) - size);
        set_block_size(blk, size);

        block_prev_phy_mark_used(blk);
        block_mark_used(blk);

        block_prev_phy_mark_used(next_blk);
        block_mark_free(next_blk);
        insert_free_block(mem, next_blk);
    } else {
        block_prev_phy_mark_used(blk);
        block_mark_used(blk);
        remove_free_block(mem, blk);
    }

    p = (void *)blk_to_ptr(blk);
    return p;
}

static ipgui_mem_blk_t * merge_prev_phy(ipgui_mem_mng_t * mem, ipgui_mem_blk_t * blk, ipgui_mem_blk_t * prev)
{
    remove_free_block(mem, prev);
    set_block_size(prev, BLOCK_SIZE(blk) + BLOCK_SIZE(prev));
    return prev;
}

static ipgui_mem_blk_t * merge_next_phy(ipgui_mem_mng_t * mem, ipgui_mem_blk_t * blk, ipgui_mem_blk_t * next)
{
    remove_free_block(mem, next);
    set_block_size(blk, BLOCK_SIZE(blk) + BLOCK_SIZE(next));
    return blk;
}

void ipgui_mempool_free(ipgui_mem_mng_t * mem, void * p)
{
    // DBG_ASSERT(mem && p, "invalid parameter, mem and p must be valid");
    
    ipgui_mem_blk_t * blk, * next_blk;

    blk = ptr_to_blk(p);
    next_blk = block_next_phy(blk);
    
    /* first, set this block free 
     * second, set next physical block's prev physical block free 
     */
    block_mark_free(blk);
    next_blk->prev_phy = blk;
    block_prev_phy_mark_free(next_blk);

    /* merge prev physical memory if possible */
    if (block_prev_phy_is_free(blk)) {
        /* merge prev phy */
        ipgui_mem_blk_t * prev_blk;
        prev_blk = blk->prev_phy;
        blk = merge_prev_phy(mem, blk, prev_blk);
    }

    /* merge next physical memory if possible */
    if (block_is_free(next_blk)) {
        /* merge next phy */
        blk = merge_next_phy(mem, blk, next_blk);
    }

    insert_free_block(mem, blk);
}
