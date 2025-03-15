section .multiboot
align 4
multiboot_header:
    dd 0x1BADB002              ; Multiboot magic number
    dd 0x0                     ; Flags (no extra features)
    dd -(0x1BADB002 + 0x0)     ; Checksum (magic + flags + checksum = 0)

section .text
global _start
extern kernel_main
_start:
    call kernel_main
    cli                        ; Disable interrupts
    hlt                        ; Halt CPU
