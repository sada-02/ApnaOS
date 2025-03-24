[bits 32]
global isr_stub

isr_stub:
    pusha           ; Save registers
    cli             ; Clear interrupts

    hlt             ; Halt CPU
    jmp 1b          ; Jump back to label 1
