    .section .text
    .global switch_context
    .type switch_context, @function
# void switch_context(uint32_t **old_sp, uint32_t *new_sp);
switch_context:
    pushl %ebp
    pushl %ebx
    pushl %esi
    pushl %edi
    # After pushing 4 registers, the stack pointer is decreased by 16 bytes.
    # According to cdecl:
    #   - The return address is originally at 0(%esp) before our pushes.
    #   - After pushes, parameters are at offsets: 
    #       old_sp is at 20(%esp) and new_sp at 24(%esp).
    movl 20(%esp), %eax       # Load pointer to old_sp into EAX.
    cmpl $0, %eax
    je L1
    movl %esp, (%eax)         # Save current ESP into *old_sp.
L1:
    movl 24(%esp), %eax       # Load new_sp pointer into EAX.
    movl %eax, %esp           # Set ESP to new_sp pointer.
    popl %edi                 # Restore registers.
    popl %esi
    popl %ebx
    popl %ebp
    ret
