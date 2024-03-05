    .section .init
    .align  2
    .globl  __start
    .type   __start, %function

__start:

/* push */
.option push

/* norelax */
.option norelax
    la      gp, __global_pointer$

/* pop */
.option pop
    csrci   mstatus, 8

    lui a0, 0x20056
    lw  a1, 0xb0(a0)
    lui a0, 0x40281
    beq a0, a1, 1f
    j .
1:

    /* entry */
    la      a0, default_trap_handler
    ori     a0, a0, 3
    csrw    mtvec, a0

#ifdef CONFIG_IRQ_USE_VECTOR
    /* entry */
    la      a0, __Vectors
    csrw    mtvt, a0
#endif

#if defined(__riscv_flen)
    /* set mstatus */
    csrr    t0, mstatus
    li      t1, ~0x6000
    and     t0, t0, t1
    li      t1, 0x2000
    or      t0, t0, t1
    csrw    mstatus, t0
#endif

    .weak __StackTop
    la      sp, __StackTop
    csrw    mscratch, sp

    call     SystemInit

    call     start_load

    call     System_Post_Init

    call     main

    .size   __start, . - __start

__exit:
    j      __exit

