section .multiboot
align 4
multiboot_header:
    dd 0x1BADB002      ; Multiboot magic
    dd 0x0             ; Flags (set as needed)
    dd -(0x1BADB002 + 0x0)

section .bss
align 16
resb 16384
stack_top:

section .text
global _start
extern kernel_main

_start:
    mov esp, stack_top      ; Setup stack (32-bit register)
    push ebx                ; Pass multiboot info pointer (from ebx)
    call kernel_main
    add esp, 4              ; Clean up the stack
    cli
    hlt
    
section .note.GNU-stack
