.section .text.boot,"ax",@progbits
.global _start
.type _start,@function
/* Future handoff: RDI=boot_info, RSP=aligned kernel stack, linked image mapped. */
_start:
    cli
    cld
    call kernel_main
.Lhalt:
    cli
    hlt
    jmp .Lhalt
.size _start, . - _start
