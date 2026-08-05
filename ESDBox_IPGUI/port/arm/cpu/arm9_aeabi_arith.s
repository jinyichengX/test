@ =====================================================================
@  aeabi-arith.s
@  Software arithmetic helpers for the Allwinner F1C100s (ARM926EJ-S,
@  ARMv5TE) which has no hardware integer divide instruction.
@
@  When GCC emits 32/64-bit integer multiply and divide on such a
@  target it expects a small set of "AEABI" helper routines that
@  normally live in libgcc.  This file provides all of them in a single
@  self-contained assembly source so the program can be linked without
@  pulling in libgcc.
@
@  Source of the implementations
@    * 32-bit divide, 32-bit divmod, 64-bit multiply, 64-bit shifts and
@      __clzsi2 are transcribed verbatim from the libgcc.a shipped with
@      the YAGARTO toolchain (GCC 4.7.2) -- these come from GCC's
@      gcc/config/arm/lib1funcs.asm.
@    * The 64-bit divide (__aeabi_uldivmod / __aeabi_ldivmod) is, in that
@      toolchain, compiled C code and not available as readable assembly,
@      so it is implemented here as a clean shift-subtract long division.
@
@  ARM EABI calling convention (register pairs are lo:hi):
@    __aeabi_uidiv(u32 n, u32 d)        r0       = n / d
@    __aeabi_idiv(s32 n, s32 d)        r0       = n / d
@    __aeabi_uidivmod(u32 n, u32 d)    r0=n/d , r1 = n%d
@    __aeabi_idivmod(s32 n, s32 d)     r0=n/d , r1 = n%d
@    __aeabi_lmul (u64 a, u64 b)        r0:r1    = (a*b)[63:0]
@    __aeabi_uldivmod(u64 n, u64 d)    r0:r1=n/d, r2:r3 = n%d
@    __aeabi_ldivmod(s64 n, s64 d)     r0:r1=n/d, r2:r3 = n%d
@    __aeabi_llsl(u64 x, u32 s)        r0:r1    = x << s
@    __aeabi_llsr(u64 x, u32 s)        r0:r1    = x >> s  (logical)
@    __aeabi_lasr(u64 x, u32 s)        r0:r1    = x >> s  (arithmetic)
@    __clzsi2(u32 x)                    r0       = count leading zeros
@    __aeabi_idiv0 / __aeabi_ldiv0     divide-by-zero stubs (return)
@
@  Build:  arm-none-eabi-as -mcpu=arm926ej-s aeabi-arith.s -o aeabi-arith.o
@  (or just add aeabi-arith.s to your project sources / Makefile).
@ =====================================================================

    .syntax unified
    .arm
    .text


@ ---------------------------------------------------------------------
@ Divide-by-zero stubs (from _dvmd_tls.o).
@ The caller has already placed a saturated result in r0[-:r1]; we
@ simply return.  Override these if you prefer to trap.
@ ---------------------------------------------------------------------
    .align 2
    .global __aeabi_idiv0
    .global __aeabi_ldiv0
    .type   __aeabi_idiv0, %function
    .type   __aeabi_ldiv0, %function
__aeabi_idiv0:
__aeabi_ldiv0:
    bx      lr
    .size   __aeabi_idiv0, . - __aeabi_idiv0
    .size   __aeabi_ldiv0, . - __aeabi_ldiv0


@ ---------------------------------------------------------------------
@ __aeabi_uidiv / __udivsi3  --  unsigned 32-bit divide: r0 = r0 / r1
@ (verbatim from _udivsi3.o / lib1funcs.asm)
@ ---------------------------------------------------------------------
    .align 2
    .global __aeabi_uidiv
    .global __udivsi3
    .type   __aeabi_uidiv, %function
    .type   __udivsi3, %function
__aeabi_uidiv:
__udivsi3:
    subs    r2, r1, #1              @ d-1
    bxeq    lr                      @ d == 1 -> return dividend
    blo     .Luidiv_div0            @ carry clear -> d == 0
    cmp     r0, r1
    bls     .Luidiv_le              @ n <= d
    tst     r1, r2                  @ power of two?  d & (d-1)
    beq     .Luidiv_pow2
    tst     r1, #0xe0000000
    lsleq   r1, r1, #3
    moveq   r3, #8
    movne   r3, #1
.Luidiv_n1:
    cmp     r1, #0x10000000
    cmplo   r1, r0
    lsllo   r1, r1, #4
    lsllo   r3, r3, #4
    blo     .Luidiv_n1
.Luidiv_n2:
    cmp     r1, #0x80000000
    cmplo   r1, r0
    lsllo   r1, r1, #1
    lsllo   r3, r3, #1
    blo     .Luidiv_n2
    mov     r2, #0
.Luidiv_loop:
    cmp     r0, r1
    subhs   r0, r0, r1
    orrhs   r2, r2, r3
    cmp     r0, r1, lsr #1
    subhs   r0, r0, r1, lsr #1
    orrhs   r2, r2, r3, lsr #1
    cmp     r0, r1, lsr #2
    subhs   r0, r0, r1, lsr #2
    orrhs   r2, r2, r3, lsr #2
    cmp     r0, r1, lsr #3
    subhs   r0, r0, r1, lsr #3
    orrhs   r2, r2, r3, lsr #3
    cmp     r0, #0
    lsrsne  r3, r3, #4
    lsrne   r1, r1, #4
    bne     .Luidiv_loop
    mov     r0, r2
    bx      lr
.Luidiv_le:                         @ n <= d
    moveq   r0, #1                  @ n == d -> 1
    movne   r0, #0                  @ n <  d -> 0
    bx      lr
.Luidiv_pow2:                       @ divisor is a power of two
    cmp     r1, #0x10000
    lsrhs   r1, r1, #16
    movhs   r2, #16
    movlo   r2, #0
    cmp     r1, #0x100
    lsrhs   r1, r1, #8
    addhs   r2, r2, #8
    cmp     r1, #0x10
    lsrhs   r1, r1, #4
    addhs   r2, r2, #4
    cmp     r1, #4
    addhi   r2, r2, #3
    addls   r2, r2, r1, lsr #1
    lsr     r0, r0, r2
    bx      lr
.Luidiv_div0:                       @ divide by zero
    cmp     r0, #0
    mvnne   r0, #0                  @ n != 0 -> 0xffffffff
    b       __aeabi_idiv0
    .size   __aeabi_uidiv, . - __aeabi_uidiv


@ ---------------------------------------------------------------------
@ __aeabi_uidivmod  --  unsigned 32-bit divide+mod: r0 = n/d, r1 = n%d
@ (verbatim from _udivsi3.o)
@ ---------------------------------------------------------------------
    .align 2
    .global __aeabi_uidivmod
    .type   __aeabi_uidivmod, %function
__aeabi_uidivmod:
    cmp     r1, #0
    beq     .Luidiv_div0            @ shares the div-by-zero path above
    push    {r0, r1, lr}
    bl      __udivsi3
    pop     {r1, r2, lr}
    mul     r3, r2, r0              @ divisor * quotient
    sub     r1, r1, r3              @ remainder = dividend - that
    bx      lr
    .size   __aeabi_uidivmod, . - __aeabi_uidivmod


@ ---------------------------------------------------------------------
@ __aeabi_idiv / __divsi3  --  signed 32-bit divide: r0 = r0 / r1
@ (verbatim from _divsi3.o / lib1funcs.asm)
@ ---------------------------------------------------------------------
    .align 2
    .global __aeabi_idiv
    .global __divsi3
    .type   __aeabi_idiv, %function
    .type   __divsi3, %function
__aeabi_idiv:
__divsi3:
    cmp     r1, #0
    beq     .Lidiv_div0
.Ldivsi3_body:                      @ entry used by idivmod (div0 already checked)
    eor     ip, r0, r1             @ result sign in N
    rsbmi   r1, r1, #0             @ make divisor positive
    subs    r2, r1, #1
    beq     .Lidiv_d1              @ |d| == 1
    movs    r3, r0
    rsbmi   r3, r0, #0             @ make dividend positive (r3 = |n|)
    cmp     r3, r1
    bls     .Lidiv_le
    tst     r1, r2                 @ power of two?
    beq     .Lidiv_pow2
    tst     r1, #0xe0000000
    lsleq   r1, r1, #3
    moveq   r2, #8
    movne   r2, #1
.Lidiv_n1:
    cmp     r1, #0x10000000
    cmplo   r1, r3
    lsllo   r1, r1, #4
    lsllo   r2, r2, #4
    blo     .Lidiv_n1
.Lidiv_n2:
    cmp     r1, #0x80000000
    cmplo   r1, r3
    lsllo   r1, r1, #1
    lsllo   r2, r2, #1
    blo     .Lidiv_n2
    mov     r0, #0
.Lidiv_loop:
    cmp     r3, r1
    subhs   r3, r3, r1
    orrhs   r0, r0, r2
    cmp     r3, r1, lsr #1
    subhs   r3, r3, r1, lsr #1
    orrhs   r0, r0, r2, lsr #1
    cmp     r3, r1, lsr #2
    subhs   r3, r3, r1, lsr #2
    orrhs   r0, r0, r2, lsr #2
    cmp     r3, r1, lsr #3
    subhs   r3, r3, r1, lsr #3
    orrhs   r0, r0, r2, lsr #3
    cmp     r3, #0
    lsrsne  r2, r2, #4
    lsrne   r1, r1, #4
    bne     .Lidiv_loop
    cmp     ip, #0
    rsbmi   r0, r0, #0             @ negate quotient if signs differed
    bx      lr
.Lidiv_d1:                         @ |d| == 1
    teq     ip, r0                 @ sign of original divisor
    rsbmi   r0, r0, #0
    bx      lr
.Lidiv_le:                         @ |n| <= |d|
    movlo   r0, #0                 @ |n| < |d| -> 0
    asreq   r0, ip, #31            @ |n| == |d| -> +/-1 (sign from ip)
    orreq   r0, r0, #1
    bx      lr
.Lidiv_pow2:                       @ |d| is a power of two
    cmp     r1, #0x10000
    lsrhs   r1, r1, #16
    movhs   r2, #16
    movlo   r2, #0
    cmp     r1, #0x100
    lsrhs   r1, r1, #8
    addhs   r2, r2, #8
    cmp     r1, #0x10
    lsrhs   r1, r1, #4
    addhs   r2, r2, #4
    cmp     r1, #4
    addhi   r2, r2, #3
    addls   r2, r2, r1, lsr #1
    cmp     ip, #0
    lsr     r0, r3, r2
    rsbmi   r0, r0, #0
    bx      lr
.Lidiv_div0:                       @ divide by zero (signed)
    cmp     r0, #0
    mvngt   r0, #0x80000000        @ n > 0 -> 0x7fffffff (INT_MAX)
    movlt   r0, #0x80000000        @ n < 0 -> 0x80000000 (INT_MIN)
    b       __aeabi_idiv0
    .size   __aeabi_idiv, . - __aeabi_idiv


@ ---------------------------------------------------------------------
@ __aeabi_idivmod  --  signed 32-bit divide+mod: r0 = n/d, r1 = n%d
@ (verbatim from _divsi3.o)
@ ---------------------------------------------------------------------
    .align 2
    .global __aeabi_idivmod
    .type   __aeabi_idivmod, %function
__aeabi_idivmod:
    cmp     r1, #0
    beq     .Lidiv_div0
    push    {r0, r1, lr}
    bl      .Ldivsi3_body
    pop     {r1, r2, lr}
    mul     r3, r2, r0
    sub     r1, r1, r3
    bx      lr
    .size   __aeabi_idivmod, . - __aeabi_idivmod


@ ---------------------------------------------------------------------
@ __aeabi_lmul / __muldi3  --  64-bit multiply, low 64 bits
@   in : r0:r1 = a (lo:hi), r2:r3 = b (lo:hi)
@   out: r0:r1 = (a * b)[63:0]   (same result for signed & unsigned)
@ (verbatim from _muldi3.o / lib1funcs.asm)
@ ---------------------------------------------------------------------
    .align 2
    .global __aeabi_lmul
    .global __muldi3
    .type   __aeabi_lmul, %function
    .type   __muldi3, %function
__aeabi_lmul:
__muldi3:
    mul     r1, r2, r1             @ b_lo * a_hi        (cross term, low)
    mla     r1, r0, r3, r1         @ + a_lo * b_hi       (high-word cross sum)
    str     r4, [sp, #-4]!         @ save r4
    lsr     r3, r0, #16            @ a_hi16
    lsr     ip, r2, #16            @ b_hi16
    bic     r4, r0, r3, lsl #16    @ a_lo16
    bic     r2, r2, ip, lsl #16    @ b_lo16
    mul     r0, r4, r2             @ a_lo16 * b_lo16  -> new low word
    mul     r2, r3, r2             @ a_hi16 * b_lo16
    mul     r4, ip, r4             @ b_hi16 * a_lo16
    mul     ip, r3, ip             @ a_hi16 * b_hi16
    adds    r4, r2, r4             @ mid = a_hi16*b_lo16 + b_hi16*a_lo16
    addhs   ip, ip, #0x10000       @ carry of mid into high partial
    adds    r0, r0, r4, lsl #16    @ low += mid << 16
    adc     ip, ip, r4, lsr #16    @ high partial += mid >> 16 + carry
    add     r1, r1, ip             @ high word += low-product high partial
    ldm     sp!, {r4}
    bx      lr
    .size   __aeabi_lmul, . - __aeabi_lmul


@ ---------------------------------------------------------------------
@ 64-bit shift helpers (verbatim from _ashldi3/_ashrdi3/_lshrdi3.o)
@   in : r0:r1 = value (lo:hi), r2 = shift count
@   out: r0:r1 = shifted value
@ ---------------------------------------------------------------------
    .align 2
    .global __aeabi_llsl
    .global __ashldi3
    .type   __aeabi_llsl, %function
    .type   __ashldi3, %function
__aeabi_llsl:
__ashldi3:
    subs    r3, r2, #32
    rsb     ip, r2, #32
    lslmi   r1, r1, r2
    lslpl   r1, r0, r3
    orrmi   r1, r1, r0, lsr ip
    lsl     r0, r0, r2
    bx      lr
    .size   __aeabi_llsl, . - __aeabi_llsl

    .align 2
    .global __aeabi_llsr
    .global __lshrdi3
    .type   __aeabi_llsr, %function
    .type   __lshrdi3, %function
__aeabi_llsr:
__lshrdi3:
    subs    r3, r2, #32
    rsb     ip, r2, #32
    lsrmi   r0, r0, r2
    lsrpl   r0, r1, r3
    orrmi   r0, r0, r1, lsl ip
    lsr     r1, r1, r2
    bx      lr
    .size   __aeabi_llsr, . - __aeabi_llsr

    .align 2
    .global __aeabi_lasr
    .global __ashrdi3
    .type   __aeabi_lasr, %function
    .type   __ashrdi3, %function
__aeabi_lasr:
__ashrdi3:
    subs    r3, r2, #32
    rsb     ip, r2, #32
    lsrmi   r0, r0, r2
    asrpl   r0, r1, r3
    orrmi   r0, r0, r1, lsl ip
    asr     r1, r1, r2
    bx      lr
    .size   __aeabi_lasr, . - __aeabi_lasr


@ ---------------------------------------------------------------------
@ __clzsi2  --  count leading zeros of a 32-bit value (r0 in, r0 out)
@ (verbatim from _clzsi2.o / lib1funcs.asm)
@ ---------------------------------------------------------------------
    .align 2
    .global __clzsi2
    .type   __clzsi2, %function
__clzsi2:
    mov     r1, #28
    cmp     r0, #0x10000
    lsrhs   r0, r0, #16
    subhs   r1, r1, #16
    cmp     r0, #0x100
    lsrhs   r0, r0, #8
    subhs   r1, r1, #8
    cmp     r0, #0x10
    lsrhs   r0, r0, #4
    subhs   r1, r1, #4
    adr     r2, .Lclz_table
    ldrb    r0, [r2, r0]
    add     r0, r0, r1
    bx      lr
    .align 2
.Lclz_table:
    .byte   4, 3, 2, 2, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0
    .size   __clzsi2, . - __clzsi2


@ =====================================================================
@  64-bit division (implemented here -- see file header)
@ =====================================================================

@ ---------------------------------------------------------------------
@ __uldivmod64_core  --  unsigned 64 / 64 -> quotient + remainder
@   in : r0:r1 = n,  r2:r3 = d   (d must be non-zero)
@   out: r0:r1 = n/d, r2:r3 = n%d
@ Classic bit-by-bit long division, MSB first.
@ ---------------------------------------------------------------------
    .align 2
    .type   __uldivmod64_core, %function
__uldivmod64_core:
    push    {r4, r5, r6, r7, lr}
    mov     r6, r0                  @ n_lo  (consumed as we shift)
    mov     r7, r1                  @ n_hi
    mov     r4, #0                  @ remainder_lo
    mov     r5, #0                  @ remainder_hi
    mov     r0, #0                  @ quotient_lo
    mov     r1, #0                  @ quotient_hi
    mov     ip, #64                 @ bit counter
.Luldiv_loop:
    mov     lr, r7, lsr #31         @ bit = MSB of n
    movs    r6, r6, lsl #1          @ n <<= 1 (lo), C = bit shifted out
    adc     r7, r7, r7              @ n <<= 1 (hi)
    movs    r4, r4, lsl #1          @ rem <<= 1 (lo), C = bit shifted out
    adc     r5, r5, r5              @ rem <<= 1 (hi)
    orr     r4, r4, lr              @ rem |= bit
    cmp     r5, r3                  @ rem >= d ?  (high words)
    cmpeq   r4, r2                  @   if equal, decide on low words
    bcc     .Luldiv_nosub
    subs    r4, r4, r2              @ rem -= d
    sbc     r5, r5, r3
    movs    r0, r0, lsl #1          @ q = (q << 1) | 1
    adc     r1, r1, r1
    orr     r0, r0, #1
    b       .Luldiv_next
.Luldiv_nosub:
    movs    r0, r0, lsl #1          @ q = q << 1
    adc     r1, r1, r1
.Luldiv_next:
    subs    ip, ip, #1
    bne     .Luldiv_loop
    mov     r2, r4                  @ remainder -> r2:r3
    mov     r3, r5
    pop     {r4, r5, r6, r7, pc}
    .size   __uldivmod64_core, . - __uldivmod64_core


@ ---------------------------------------------------------------------
@ __aeabi_uldivmod  --  unsigned 64-bit divide+mod
@   in : r0:r1 = n,  r2:r3 = d
@   out: r0:r1 = n/d, r2:r3 = n%d
@ ---------------------------------------------------------------------
    .align 2
    .global __aeabi_uldivmod
    .type   __aeabi_uldivmod, %function
__aeabi_uldivmod:
    orrs    ip, r2, r3              @ d == 0 ?
    bne     .Luldiv_ok
    @ divide by zero: quotient = (n==0) ? 0 : 0xffffffffffffffff, rem = 0
    orrs    ip, r0, r1
    mov     r2, #0
    mov     r3, #0
    mvnne   r0, #0
    mvnne   r1, #0
    moveq   r0, #0
    moveq   r1, #0
    bx      lr
.Luldiv_ok:
    b       __uldivmod64_core       @ tail call (d != 0 guaranteed)
    .size   __aeabi_uldivmod, . - __aeabi_uldivmod


@ ---------------------------------------------------------------------
@ __aeabi_ldivmod  --  signed 64-bit divide+mod
@   in : r0:r1 = n (signed),  r2:r3 = d (signed)
@   out: r0:r1 = n/d,  r2:r3 = n%d   (remainder takes sign of dividend)
@ ---------------------------------------------------------------------
    .align 2
    .global __aeabi_ldivmod
    .type   __aeabi_ldivmod, %function
__aeabi_ldivmod:
    orrs    ip, r2, r3              @ d == 0 ?
    bne     .Lldiv_ok
    @ divide by zero (signed): saturated quotient, remainder 0
    cmp     r1, #0                  @ sign of n in high word (bit 63)
    blt     .Lldiv_dz_neg           @ n < 0  -> INT64_MIN
    orrs    ip, r0, r1             @ n == 0 ?  (both words zero)
    beq     .Lldiv_dz_zero
.Lldiv_dz_pos:                      @ n > 0  -> INT64_MAX = 0x7fffffffffffffff
    mvn     r0, #0                  @ 0xffffffff
    mvn     r1, #0x80000000         @ 0x7fffffff
    mov     r2, #0
    mov     r3, #0
    bx      lr
.Lldiv_dz_zero:
    mov     r0, #0
    mov     r1, #0
    mov     r2, #0
    mov     r3, #0
    bx      lr
.Lldiv_dz_neg:                      @ n < 0  -> INT64_MIN = 0x8000000000000000
    mov     r0, #0
    mov     r1, #0x80000000
    mov     r2, #0
    mov     r3, #0
    bx      lr
.Lldiv_ok:
    push    {r4, r5, lr}
    @ |n| ; r5 = 1 if n was negative (remainder must be negated)
    mov     r5, #0
    cmp     r1, #0
    bpl     .Lldiv_absd
    rsbs    r0, r0, #0
    rsc     r1, r1, #0
    mov     r5, #1
.Lldiv_absd:
    @ |d| ; ip = 1 if d was negative
    mov     ip, #0
    cmp     r3, #0
    bpl     .Lldiv_call
    rsbs    r2, r2, #0
    rsc     r3, r3, #0
    mov     ip, #1
.Lldiv_call:
    eor     r4, r5, ip             @ r4 = 1 -> quotient must be negated
    bl      __uldivmod64_core
    @ fix quotient sign
    cmp     r4, #0
    beq     .Lldiv_fixr
    rsbs    r0, r0, #0
    rsc     r1, r1, #0
.Lldiv_fixr:
    @ fix remainder sign (sign of dividend)
    cmp     r5, #0
    beq     .Lldiv_done
    rsbs    r2, r2, #0
    rsc     r3, r3, #0
.Lldiv_done:
    pop     {r4, r5, pc}
    .size   __aeabi_ldivmod, . - __aeabi_ldivmod


    .end