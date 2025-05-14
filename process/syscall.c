#include "syscall.h"
#include "process.h"
#include "../memory/memory.h"

extern void debug_print(const char* messe);
extern void print_to_screen(const char* message);
extern void debug_int(int val);
extern void kfree(void* ptr);

PCB* get_current_process(void) {
    return current_process;
}

static PCB* find_zombie_child(PCB* parent) {
    debug_print("DEBUG: Searching for zombie child of parent with pid:");
    debug_int(parent->pid);
    PCB* current = process_table_head;
    PCB* prev = NULL;
    while (current != NULL) {
        if (current->parent->pid == parent->pid && current->state == STATE_ZOMBIE) {
            debug_print("DEBUG: Found zombie child");
            if (prev == NULL) {
                process_table_head = current->next_in_table;
            } else {
                prev->next_in_table = current->next_in_table;
            }
            
            return current;
        }
        
        prev = current;
        current = current->next_in_table;
    }
    
    return NULL;
}

int fork_syscall(void) {
    uint32_t* return_addr;
    uint32_t* stack_ptr;

    __asm__ volatile(
        "movl 4(%%ebp), %0\n\t"
        "lea (%%ebp), %1\n\t" 
        : "=r" (return_addr), "=r" (stack_ptr)
    );

    debug_print("DEBUG: Fork syscall started");
    PCB* parent = get_current_process();
    if (parent == NULL) {
        debug_print("DEBUG: Fork failed - no current process");
        return -1;
    }

    parent->user_stack_ptr = stack_ptr;
    PCB* child = (PCB*)kmalloc(sizeof(PCB));
    if (child == NULL) {
        debug_print("DEBUG: Fork failed - memory allocation error");
        return -1;
    }
    
    child->pid = get_new_pid();
    child->parent = parent;
    child->priority = parent->priority;
    uint32_t* child_stack_base = (uint32_t*)kmalloc(4096);
    if (child_stack_base == NULL) {
        debug_print("DEBUG: Fork failed - stack allocation error");
        return -1;
    }

    uint32_t stack_offset = (uint32_t)parent->user_stack_ptr - (uint32_t)parent->user_stack_base;
    copy_memory(child_stack_base, parent->user_stack_base, 4096);
    child->user_stack_ptr = (uint32_t*)((uint32_t)child_stack_base + stack_offset);
    if (parent->cr3 != 0) {
        child->cr3 = (uint32_t)kmalloc(4096);
        if (child->cr3 == 0) {
            debug_print("DEBUG: Fork failed - page table allocation error");
            return -1;  
        }
        copy_page_tables(parent->cr3, child->cr3);
    }

    child->kernel_stack_base = (uint32_t*)kmalloc(KERNEL_STACK_SIZE);
    child->kernel_stack_ptr = child->kernel_stack_base + (KERNEL_STACK_SIZE/sizeof(uint32_t));
    
    child->state = STATE_READY;
    child->is_new_child = true;
    child->next_in_table = process_table_head;
    process_table_head = child;
    enqueue_process(&ready_queue, child);
    
    debug_print("DEBUG: Fork created new process with PID:");
    debug_int(child->pid);
    return child->pid;
}

int wait_syscall(int* status) {
    debug_print("DEBUG: Wait syscall started");

    PCB* parent = get_current_process();
    if (parent == NULL) {
        debug_print("DEBUG: Wait failed - no current process");
        return -1;
    }
    PCB* zombie_child = find_zombie_child(parent);
    if (zombie_child != NULL) {
        if (status != NULL) {
            *status = zombie_child->exit_status;
        }
        
        int child_pid = zombie_child->pid;
        kfree(zombie_child);

        return child_pid;
    }

    debug_print("DEBUG: No existing zombie children found. Blocking parent.");
    parent->state = STATE_BLOCKED;
    
    debug_print("DEBUG: Wait syscall going to schedule a process after blocking process with PID:");
    debug_int(parent->pid);
    yield_syscall();

    debug_print("DEBUG: Wait syscall resumed after blocking for the process with PID:");
    debug_int(parent->pid);
    
    zombie_child = find_zombie_child(parent);
    if (zombie_child != NULL) {
        if (status != NULL) {
            *status = zombie_child->exit_status;
        }
        
        int child_pid = zombie_child->pid;
        
        kfree(zombie_child);

        return child_pid;
    }
    
    debug_print("DEBUG: Wait failed - no children found after blocking");

    return -1;
}

void exit_syscall(int status) {
    debug_print("DEBUG: Exit syscall started with status:");
    debug_int(status);
    
    // Get the current process
    PCB* proc = get_current_process();
    if (proc == NULL) {
        debug_print("DEBUG: Exit failed - no current process");
        return;
    }
    debug_print("DEBUG: Exiting process with PID:");
    debug_int(proc->pid);

    proc->exit_status = status;
    proc->state = STATE_ZOMBIE;
    if (proc->parent != NULL && proc->parent->state == STATE_BLOCKED) {
        proc->parent->state = STATE_READY;
        enqueue_process(&ready_queue, proc->parent);
    }
    schedule();
    
    debug_print("DEBUG: Exit syscall - should not reach here");
    while(1); 
}

void yield_syscall(void) {
    uint32_t* stack_ptr;
    
    __asm__ volatile(
        "lea (%%ebp), %0\n\t"
        : "=r" (stack_ptr)
    );

    PCB* proc = get_current_process();
    if (proc == NULL) {
        debug_print("DEBUG: Yield failed - no current process");
        return;
    }
    
    debug_print("DEBUG: Process yielding CPU has PID:");
    debug_int(proc->pid);

    proc->user_stack_ptr = stack_ptr;

    asm volatile (
        "movl %0, %%esp\n\t"  
        :
        : "r" (proc->kernel_stack_ptr)
    );
    schedule();
    debug_print("DEBUG: Yield syscall - should not reach here");
}

void init_syscalls(void) {
    debug_print("DEBUG: System calls initialized");
}
