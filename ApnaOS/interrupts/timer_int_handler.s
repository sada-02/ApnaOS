    .section .text
    .global timer_interrupt_handler
    .type timer_interrupt_handler, @function

timer_interrupt_handler:
    pusha                       # Save all general-purpose registers
    pushl %ds
    pushl %es
    pushl %fs
    pushl %gs

    # Set data segments (assuming 0x10 is the kernel data segment)
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    # Call the C function that performs scheduling.
    call context_switch_handler

    # Set ESP to the new process's saved kernel stack pointer (returned in EAX).
    mov %eax, %esp

    # Restore registers from the new process's kernel stack.
    popl %gs
    popl %fs
    popl %es
    popl %ds
    popa
    iret
