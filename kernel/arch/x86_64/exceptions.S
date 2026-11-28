.section .text
.global exception_handler_common

/* Common dispatch: RDI = vector, RSI = error_code */
exception_dispatch:
    cli
    cld
    call exception_handler_common
    hlt
    jmp exception_dispatch

/* ------------------------------------------------------------------ */
/* Exceptions without error code                                      */
/* ------------------------------------------------------------------ */

.macro exc_no_err vec
    .global exception_stub_\vec
    .type exception_stub_\vec, @function
exception_stub_\vec:
    movq $0x\vec, %rdi
    xorq %rsi, %rsi
    jmp exception_dispatch
.size exception_stub_\vec, . - exception_stub_\vec
.endm

/* ------------------------------------------------------------------ */
/* Exceptions with error code                                         */
/* ------------------------------------------------------------------ */

.macro exc_with_err vec
    .global exception_stub_\vec
    .type exception_stub_\vec, @function
exception_stub_\vec:
    movq $0x\vec, %rdi
    movq (%rsp), %rsi
    jmp exception_dispatch
.size exception_stub_\vec, . - exception_stub_\vec
.endm

/* Vector 0: Divide Error                                            */
exc_no_err 0
/* Vector 1: Debug                                                   */
exc_no_err 1
/* Vector 2: NMI                                                     */
exc_no_err 2
/* Vector 3: Breakpoint                                              */
exc_no_err 3
/* Vector 4: Overflow                                                */
exc_no_err 4
/* Vector 5: Bound Range Exceeded                                    */
exc_no_err 5
/* Vector 6: Invalid Opcode                                          */
exc_no_err 6
/* Vector 7: Device Not Available                                    */
exc_no_err 7
/* Vector 8: Double Fault (error code)                               */
exc_with_err 8
/* Vector 9: Coprocessor Segment Overrun                             */
exc_no_err 9
/* Vector 10: Invalid TSS (error code)                               */
exc_with_err 10
/* Vector 11: Segment Not Present (error code)                       */
exc_with_err 11
/* Vector 12: Stack Segment Fault (error code)                       */
exc_with_err 12
/* Vector 13: General Protection Fault (error code)                  */
exc_with_err 13
/* Vector 14: Page Fault (error code)                                */
exc_with_err 14
/* Vector 15: Reserved                                               */
exc_no_err 15
/* Vector 16: x87 Floating-Point Exception                           */
exc_no_err 16
/* Vector 17: Alignment Check (error code)                           */
exc_with_err 17
/* Vector 18: Machine Check                                          */
exc_no_err 18
/* Vector 19: SIMD Floating-Point Exception                          */
exc_no_err 19
/* Vector 20: Virtualization Exception (error code)                  */
exc_with_err 20
/* Vector 21-29: Reserved                                            */
exc_no_err 21
exc_no_err 22
exc_no_err 23
exc_no_err 24
exc_no_err 25
exc_no_err 26
exc_no_err 27
exc_no_err 28
exc_no_err 29
/* Vector 30: Security Exception (error code)                        */
exc_with_err 30
/* Vector 31: Reserved                                               */
exc_no_err 31
