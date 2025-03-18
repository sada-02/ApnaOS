#include "process.h"
#include "memory.h"  // if needed
#include "string.h"  // if needed
#include <stdint.h>
#include <stddef.h>
#include "keyboard.h"

void *memcpy(void *dest, const void *src, size_t n) { char *d = dest; const char *s = src; while (n--) { *d++ = *s++; } return dest; }
extern void print_to_screen(const char*);
extern void switch_context(uint32_t **old_sp, uint32_t *new_sp);

#define MAX_PROCESSES 10
PCB pcb_pool[MAX_PROCESSES];
int pcb_used[MAX_PROCESSES] = {0};
// Make these global so other files can reference them.
PCB* process_list = NULL;
PCB* current_process = NULL;
static uint32_t next_pid = 1;

void init_process_management(void) {
    process_list = NULL;
    current_process = NULL;
    next_pid = 1;
}

PCB* get_current_process(void) {
    return current_process;
}

int processes_remain(void) {
    return (process_list != NULL);
}

static PCB* allocate_pcb(void) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (!pcb_used[i]) {
            pcb_used[i] = 1;
            pcb_pool[i].pid = 0;
            pcb_pool[i].state = PROCESS_NEW;
            pcb_pool[i].entry_point = 0;
            pcb_pool[i].next = 0;
            pcb_pool[i].parent = 0;
            pcb_pool[i].fork_ret = 0;
            pcb_pool[i].exit_status = 0;
            pcb_pool[i].kernel_sp = 0;
            return &pcb_pool[i];
        }
    }
    return 0;
}

static void free_pcb(PCB* pcb) {
    int index = pcb - pcb_pool;
    if (index >= 0 && index < MAX_PROCESSES)
        pcb_used[index] = 0;
}

PCB* create_process(void (*entry_point)(void)) {
    PCB* proc = allocate_pcb();
    if (!proc) return 0;
    proc->pid = next_pid++;
    proc->state = PROCESS_READY;
    proc->entry_point = entry_point;
    proc->parent = 0;
    proc->fork_ret = 0;
    proc->exit_status = 0;
    // Initialize the kernel stack pointer to the top of the dedicated stack.
    proc->kernel_sp = (uint32_t*)(proc->stack + STACK_SIZE);
    // Push a return address: if the process function returns, call sys_exit.
    *(--proc->kernel_sp) = (uint32_t)sys_exit;
    // Also push the process's entry point as a fake return address.
    *(--proc->kernel_sp) = (uint32_t)entry_point;
    // Insert at the head of the process list.
    *(--proc->kernel_sp) = 0;    // (4) Dummy for %ebp.
    *(--proc->kernel_sp) = 0;    // (3) Dummy for %ebx.
    *(--proc->kernel_sp) = 0;    // (2) Dummy for %esi.
    *(--proc->kernel_sp) = 0;    // (1) Dummy for %edi.

    proc->next = process_list;
    process_list = proc;
    return proc;
}

void terminate_process(PCB* process) {
    if (!process) return;
    process->state = PROCESS_TERMINATED;
    PCB** curr = &process_list;
    while (*curr) {
        if (*curr == process) {
            *curr = process->next;
            break;
        }
        curr = &((*curr)->next);
    }
    free_pcb(process);
}

// Advanced scheduler that calls our assembly routine switch_context.
void schedule(void) {
    debug_print("Scheduling...\n");
    PCB* prev = current_process;
    PCB* next = NULL;
    PCB* temp = process_list;
    // debug_print("Finding next process...\n");
    // Find the first READY process.
    while (temp) {
        if (temp->state == PROCESS_READY) {
            next = temp;
            break;
        }
        temp = temp->next;
    }
    // debug_print("Next process found.\n");
    if (!next) {
        while (1)
            asm volatile ("hlt");
    }
    debug_print("Switching context...\n");
    current_process = next;
    next->state = PROCESS_RUNNING;
    debug_print("Still no bugs 1\n");
    if (prev == NULL) {
        debug_print("check 1\n");
        switch_context(0, next->kernel_sp);
        debug_print("check 2\n");
    } else {
    debug_print("Scheduling... 12541564371635671    \n");
    switch_context((prev->kernel_sp), next->kernel_sp);
    debug_print("Scheduling... 12541564371635671    \n");
    }
    debug_print("Scheduling... ended\n");
}

void yield(void) {
    schedule();
}

// --- System Call Implementations ---

int fork_syscall(void) {
    debug_print("oggy fork check 1.\n");
    PCB* parent = get_current_process();
    if (!parent) return -1;
    debug_print("fork check 2.\n");
    PCB* child = allocate_pcb();

    if (!child) return -1;
    
    child->pid = next_pid++;
    child->state = PROCESS_READY;
    child->entry_point = parent->entry_point;  // duplicate parent's code pointer.
    child->parent = parent;
    child->fork_ret = 0;  // Child sees fork() return 0.
    child->exit_status = 0;

    debug_print("fork check 3.\n");
    size_t used_bytes = (parent->stack + STACK_SIZE) - (uint8_t*)parent->kernel_sp;
    child->kernel_sp = (uint32_t*)(child->stack + STACK_SIZE - used_bytes);

    memcpy(child->kernel_sp, parent->kernel_sp, used_bytes);
    child->next = process_list;
    process_list = child;
    debug_print("fork check 4.\n");

    return child->pid;  // Parent returns child's PID.
}

int sys_exec(void (*new_entry)(void)) {
    PCB* curr = get_current_process();
    if (!curr) return -1;
    curr->entry_point = new_entry;
    // Reinitialize kernel stack for the new image.
    curr->kernel_sp = (uint32_t*)(curr->stack + STACK_SIZE);
    *(--curr->kernel_sp) = (uint32_t)sys_exit;
    *(--curr->kernel_sp) = (uint32_t)new_entry;
    return 0;
}

static PCB* find_child(PCB* parent) {
    PCB* temp = process_list;
    while (temp) {
        if (temp->parent == parent)
            return temp;
        temp = temp->next;
    }
    return 0;
}

int sys_wait(int* status) {
    // Get the parent process (the one that called wait)
    debug_print("oggy wait check 1.\n");
    PCB* parent = get_current_process();
    if (!parent) return -1;
    debug_print("oggy wait check 2.\n");

    // Mark the parent as WAITING so the scheduler will not pick it until a child completes.
    parent->state = PROCESS_WAITING;

    // Wait until all children of this parent have terminated.
    while (1) {
        debug_print("oggy wait check 3.\n");
        PCB* child = NULL;
        PCB* temp = process_list;
        // Look for any child of the parent.
        while (temp) {
            debug_print("oggy wait check 4.\n");
            if (temp->parent == parent) {
                child = temp;
                break;
            }
            temp = temp->next;
        }
        // If there is no child at all, we're done waiting.
        if (child == NULL)
            break;
        // If the found child is terminated, then reap it.
        if (child->state == PROCESS_TERMINATED) {
            debug_print("oggy wait check 5.\n");
            if (status)
                *status = child->exit_status;
            int child_pid = child->pid;
            terminate_process(child);
            // Continue checking for any remaining children.
            continue;
        }
        // Otherwise, yield to allow child processes to run.

        debug_print("oggy wait check 6.\n");
        yield();
    }

    // All children are done; restore the parent's state so it can run.
    parent->state = PROCESS_READY;
    return 0;  // Optionally, return the last child's PID or similar.
}


void sys_exit(int status) {
    PCB* curr = get_current_process();
    if (!curr) return;
    curr->exit_status = status;
    curr->state = PROCESS_TERMINATED;
    schedule();
    while (1);
}

int sys_read(int fd, void* buffer, size_t size) {
    if (!buffer || size == 0)
        return -1;

    // For now, only handle standard input (fd == 0).
    if (fd == 0) {
        return read_line((char*)buffer, size);
    }

    return -1; // Unsupported file descriptor.
}