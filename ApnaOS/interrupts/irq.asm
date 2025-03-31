; irq.asm
[bits 32]

global irq0_stub, irq1_stub, irq2_stub, irq3_stub, irq4_stub
global irq5_stub, irq6_stub, irq7_stub, irq8_stub, irq9_stub
global irq10_stub, irq11_stub, irq12_stub, irq13_stub, irq14_stub, irq15_stub
extern common_irq_handler

%macro IRQ_STUB 1
irq%1_stub:
    pusha                  ; Save registers
    push dword %1        ; <-- Push the IRQ number as a full 32-bit value
    call common_irq_handler
    add esp, 4           ; Clean up the pushed 32-bit argument
    popa                 ; Restore registers
    iretd                ; Return from interrupt
%endmacro

IRQ_STUB 0
IRQ_STUB 1
IRQ_STUB 2
IRQ_STUB 3
IRQ_STUB 4
IRQ_STUB 5
IRQ_STUB 6
IRQ_STUB 7
IRQ_STUB 8
IRQ_STUB 9
IRQ_STUB 10
IRQ_STUB 11
IRQ_STUB 12
IRQ_STUB 13
IRQ_STUB 14
IRQ_STUB 15

section .note.GNU-stack
