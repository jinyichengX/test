#ifndef IPGUI_DEFS_H
#define IPGUI_DEFS_H
#include "ipgui_utils.h"
#include "ipgui_types.h"

IPGUI_HEADER_BEGIN  _______________MARKER_______________

#define IPGUI_CAST(t, exp) ((t)(exp))

/* bitmap begin */
#if defined(IPGUI_BASETYPE_64BIT)
#define IPGUI_BITS_TO_BTYPE(x) (((x) + 8 * sizeof (unsigned long) - 1) / (8 * sizeof (unsigned long)))
#else
#define IPGUI_BITS_TO_BTYPE(x) (((x) + 8 * sizeof (unsigned int) - 1) / (8 * sizeof (unsigned int)))
#endif

#if defined(IPGUI_BASETYPE_64BIT)
#define BITS_PER_WORD (sizeof(unsigned long) * 8)
#else
#define BITS_PER_WORD (sizeof(unsigned int) * 8)
#endif

/* bit mask */
#define BIT_MASK(nr) (1UL << ((nr) % BITS_PER_WORD))
/* bit word offset */
#define BIT_WORD(nr) ((nr) / BITS_PER_WORD)


/* bitmap last word mask */
#define BITMAP_LAST_WORD_MASK(nbits) (~0UL << ((nbits + 1) % BITS_PER_WORD))

static int test_bit(int nr, const void * addr)
{
	return (1UL & (((const unsigned int *) addr)[nr >> 5] >> (nr & 31))) != 0UL;
}

static inline void __set_bit(unsigned long nr, volatile void * addr)
{
	int *m = ((int *) addr) + (nr >> 5);

	*m |= 1 << (nr & 31);
}

static inline void __change_bit(unsigned long nr, volatile void * addr)
{
	int *m = ((int *) addr) + (nr >> 5);

	*m ^= 1 << (nr & 31);
}
/* bitmap end */

#define IPGUI_TIME_FOREVER ((ipgui_tick_t)(-1))

/* MAX tick */
#define IPGUI_TIME_TICK_MAX (ipgui_tick_t)(IPGUI_TIME_FOREVER - 1)

/* widget per level capacity */
#define IPGUI_WIDGET_PER_LEVEL_CAPACITY 30

IPGUI_HEADER_END    _______________MARKER_______________
#endif