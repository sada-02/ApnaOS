; gdt.asm
[bits 32]
global gdt_flush

gdt_flush:
    lgdt [eax]         ; Load GDT pointed to by EAX.
    
    ; Update segment registers:
    mov ax, 0x10       ; Data segment selector (2nd descriptor: index 2, so selector = 2*8 = 0x10)
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Jump to flush the code segment register.
    jmp 0x08:.flush   ; Code segment selector (1st descriptor after null: 1*8 = 0x08)
.flush:
    ret
