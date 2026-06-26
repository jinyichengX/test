#ifndef IPGUI_UTILS_H
#define IPGUI_UTILS_H

#include "ipgui_types.h"

#define _INTERNEL_
#define __IPGUI_TEST__
#define __IPGUI_NOT_FOR_USER__
#define __IPGUI_INIT__
#define __IPGUI_DEINIT__
#define __IPGUI_API__
#define __IPGUI_STATIC__  static
#define __IPGUI_INLINE__  inline
#define _______________MARKER_______________

#define __IPGUI_MACRO_START         do {
#define __IPGUI_MACRO_END           } while (0)

#define IPGUI_TERNARY_EXEC(cond, true_expr, false_expr) __IPGUI_MACRO_START \
            if (cond) { \
                (true_expr); \
            } else { \
                (false_expr); \
            } \
        __IPGUI_MACRO_END

#ifndef __PERIOD_CALL__
#define __PERIOD_CALL__
#endif

#ifdef __cplusplus
#define IPGUI_HEADER_BEGIN extern "C" {
#define IPGUI_HEADER_END }
#else
#define IPGUI_HEADER_BEGIN
#define IPGUI_HEADER_END
#endif

/*
 * Compiler attribute macros — multi-compiler support
 *
 * GCC / Clang   : full support (__attribute__, __builtin_expect, weak, etc.)
 * MSVC          : best-effort subset (align, pack, weak not supported)
 * IAR           : uses __weak, __packed, __noreturn; no likely/unlikely
 * ARMCC v5      : uses __attribute__ (GCC-compatible subset)
 * ARMCC v6      : uses __attribute__ (GCC-compatible subset)
 * Unsupported   : fall back to empty macros (degrade gracefully)
 */

#if defined(__GNUC__) || defined(__clang__) || \
    (defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050))
/* ── GCC, Clang, ARM Compiler 6 ─────────────────────────────────────── */
#define IPGUI_ST_ALIGN(n)   __attribute__((aligned(n)))
#define likely(x)           __builtin_expect(!!(x), 1)
#define unlikely(x)         __builtin_expect(!!(x), 0)
#define __UNUSED__          __attribute__((__unused__))
#define __USED__            __attribute__((__used__))
#define __PACKED__          __attribute__((packed))
#define __PURE__            __attribute__((__pure__))
#define __CONST__           __attribute__((__const__))
#define __NO_RETURN__       __attribute__((__noreturn__))
#define __NAKED__           __attribute__((naked))
#define __WEAK__            __attribute__((weak))

#elif defined(__CC_ARM)
/* ── ARM Compiler 5 (armcc) ────────────────────────────────────────── */
#define IPGUI_ST_ALIGN(n)   __attribute__((aligned(n)))
#define likely(x)           (x)
#define unlikely(x)         (x)
#define __UNUSED__          __attribute__((__unused__))
#define __USED__            __attribute__((__used__))
#define __PACKED__          __attribute__((packed))
#define __PURE__            __attribute__((__pure__))
#define __CONST__           __attribute__((__const__))
#define __NO_RETURN__       __attribute__((__noreturn__))
#define __NAKED__           __attribute__((naked))
#define __WEAK__            __attribute__((weak))

#elif defined(__ICCARM__) || defined(__ICC430__) || defined(__IAR_SYSTEMS_ICC__)
/* ── IAR (ARM / MSP430 / STM8 / AVR) ───────────────────────────────── */
#define IPGUI_ST_ALIGN(n)   _Pragma("data_alignment=n")
#define likely(x)           (x)
#define unlikely(x)         (x)
#define __UNUSED__
#define __USED__            __root
#define __PACKED__          __packed
#define __PURE__
#define __CONST__
#define __NO_RETURN__       __noreturn
#define __NAKED__           __naked
#define __WEAK__            __weak

#elif defined(_MSC_VER)
/* ── MSVC ──────────────────────────────────────────────────────────── */
#define IPGUI_ST_ALIGN(n)   __declspec(align(n))
#define likely(x)           (x)
#define unlikely(x)         (x)
#define __UNUSED__          __pragma(warning(suppress:4100))
#define __USED__
#define __PACKED__          __declspec(align(1))
#define __PURE__
#define __CONST__
#define __NO_RETURN__       __declspec(noreturn)
#define __NAKED__           __declspec(naked)
#define __WEAK__            /* weak not supported on MSVC */

#else
/* ── Unknown compiler — fallback (no attributes) ───────────────────── */
#define IPGUI_ST_ALIGN(n)
#define likely(x)           (x)
#define unlikely(x)         (x)
#define __UNUSED__
#define __USED__
#define __PACKED__
#define __PURE__
#define __CONST__
#define __NO_RETURN__
#define __NAKED__
#define __WEAK__
#endif

/* 对齐大小由 uintptr_t 宽度自动决定：
 * 用户只需在 ipgui_types.h 中正确定义 uintptr_t（4 字节或 8 字节） */
#define IPGUI_MEM_ALIGN_SIZE      sizeof(uintptr_t)
#define IPGUI_MEM_ALIGN_SIZE_MASK (IPGUI_MEM_ALIGN_SIZE - 1)

#define IPGUI_ROUND(x)                  ((int)((x) + 0.5))

#define IPGUI_MAX(x, y)                 (((x) > (y))? (x) : (y))

#define IPGUI_MIN(x, y)                 (((x) < (y))? (x) : (y))

#define IPGUI_ABS(x)                   (((x) < 0)? (-(x)) : (x))

/// max2
#define IPGUI_MAX2(x, y)                 (((x) > (y))? (x) : (y))

/// min2
#define IPGUI_MIN2(x, y)                 (((x) < (y))? (x) : (y))

/// max3
#define IPGUI_MAX3(x, y, z)                 (((x) > (y))? (((x) > (z))? (x) : (z)) : (((y) > (z))? (y) : (z)))

/// min3
#define IPGUI_MIN3(x, y, z)                 (((x) < (y))? (((x) < (z))? (x) : (z)) : (((y) < (z))? (y) : (z)))

/// the number of entries in the array
#define IPGUI_ARRAY_LEN(x)                  ((sizeof((x)) / sizeof((x)[0])))

/// ispow2: 1, 2, 4, 8, 16, 32, ...
#define IPGUI_IS_POW2(x)                    (!((x) & ((x) - 1)) && (x))

/// align2
#define IPGUI_ALIGN2(x)                     (((x) + 1) >> 1 << 1)

/// align4
#define IPGUI_ALIGN4(x)                     (((x) + 3) >> 2 << 2)

/// align8
#define IPGUI_ALIGN8(x)                     (((x) + 7) >> 3 << 3)

/// align
#define IPGUI_ALIGN(x, b)                   (((uintptr_t)(x) + ((uintptr_t)(b) - 1)) & ~((uintptr_t)(b) - 1))

/// align u32
#define IPGUI_ALIGN_U32(x)               (((u32_t)(x) + (u32_t)3U) & ~((u32_t)3U))

/// align u64
#define IPGUI_ALIGN_U64(x, b)               (((u64_t)(x) + ((u64_t)(b) - 1)) & ~((u64_t)(b) - 1))

/// align by pow2
#define IPGUI_ALIGN_POW2(x)                 (((x) > 1)? (IPGUI_IS_POW2(x)? (x) : ((uintptr_t)1 << (32 - tb_bits_cl0_u32_be((tb_uint32_t)(x))))) : 1)

/*
 * align by cpu bytes, derived from uintptr_t width
 */
#define IPGUI_ALIGN_CPU(x)                  (((x) + IPGUI_MEM_ALIGN_SIZE_MASK) & ~((uintptr_t)IPGUI_MEM_ALIGN_SIZE_MASK))

/// offsetof
#define IPGUI_OFFSETOF(t, m)                ((uintptr_t) &((t *)0)->m)

/// container of 
#define ipgui_offsetof(type, mem)           ((uintptr_t) &((type *)0)->mem)
#define ipgui_container_of(ptr, type, mem)  (type *)((s8_t *)ptr -ipgui_offsetof(type, mem))

// /// memsizeof
// #define tb_memsizeof(s, m)              sizeof(((s const*)0)->m)

// /// memtailof
// #define tb_memtailof(s, m)              (tb_offsetof(s, m) + tb_memsizeof(s, m))

// /// memdiffof: lm - rm
// #define tb_memdiffof(s, lm, rm)         (tb_memtailof(s, lm) - tb_memtailof(s, rm))

// /// check the offset and size of member for struct or union
// #define tb_memberof_eq(ls, lm, rs, rm)  ((tb_offsetof(ls, lm) == tb_offsetof(rs, rm)) && (tb_memsizeof(ls, lm) == tb_memsizeof(rs, rm)))

/// swap
#define IPGUI_SWAP(t, l, r)                 __IPGUI_MACRO_START t __p = (r); (r) = (l); (l) = __p; __IPGUI_MACRO_END

#endif