#include "process.h"
#include <stdint.h>

// Declaration for print_to_screen (external function)
extern void print_to_screen(const char*);

// We assume that process_list and current_process are defined in process.c.
// Here, declare them as extern so we can use them.
extern PCB* process_list;
extern PCB* current_process;

uint32_t* context_switch_handler(void) {
    // Save the current process’s context by capturing the current ESP.
    uint32_t current_sp;
    asm volatile ("movl %%esp, %0" : "=r"(current_sp));
    
    // Save current ESP in the current process's PCB.
    if (current_process) {
        current_process->kernel_sp = (uint32_t*) current_sp;
    }
    
    // Simple round-robin scheduling:
    PCB* next = current_process->next;
    if (!next) {
        // If there is no next process, start from the beginning.
        next = process_list;
    }
    current_process = next;
    
    // Optionally, print debug info.
    print_to_screen("Switching context...\n");
    
    // Return the saved kernel stack pointer of the next process.
    return next->kernel_sp;
}
