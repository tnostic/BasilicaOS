[BITS 64]

section .text
global _start
extern kmain
extern _bss_start
extern _bss_end

_start:
    mov rbx, rdi

    mov rsp, kernel_stack_top

    mov rdi, _bss_start
    mov rcx, _bss_end
    sub rcx, _bss_start
    xor eax, eax
    rep stosb

    mov rdi, rbx

    mov word [0xB8008], 0x0F4B

    call kmain

    cli
.hang:
    hlt
    jmp .hang

section .bss
align 16
    resb 0x10000
kernel_stack_top:
