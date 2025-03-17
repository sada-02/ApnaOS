#include "process.h"

// Maximum number of processes supported.
#define MAX_PROCESSES 10

// Static PCB pool and usage flags.
static PCB pcb_pool[MAX_PROCESSES];
static int pcb_used[MAX_PROCESSES] = {0}; // 0 = free, 1 = used

// Pointer to the head of the process list.
static PCB* process_list = NULL;
// Pointer to the current running process.
static PCB* current_process = NULL;
// Global PID generator.
static uint32_t next_pid = 1;

// Allocate a free PCB from the pool.
static PCB* allocate_pcb() {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (!pcb_used[i]) {
            pcb_used[i] = 1;
            pcb_pool[i].pid = 0;
            pcb_pool[i].state = PROCESS_NEW;
            pcb_pool[i].entry_point = 0;
            pcb_pool[i].stack_pointer = 0;
            pcb_pool[i].next = 0;
            return &pcb_pool[i];
        }
    }
    return 0; // No free PCB available.
}

// Free a PCB and return it to the pool.
static void free_pcb(PCB* pcb) {
    int index = pcb - pcb_pool; // Pointer arithmetic to get index.
    if (index >= 0 && index < MAX_PROCESSES) {
        pcb_used[index] = 0;
    }
}

void init_process_management(void) {
    process_list = 0;
    current_process = 0;
    next_pid = 1;
}

PCB* create_process(void (*entry_point)(void)) {
    PCB* new_process = allocate_pcb();
    if (!new_process) {
        // Out of PCBs.
        return 0;
    }
    new_process->pid = next_pid++;
    new_process->state = PROCESS_READY;
    new_process->entry_point = entry_point;
    // For a real OS, allocate a proper stack; here we set it to 0.
    new_process->stack_pointer = 0;

    // Insert the new process into the list.
    new_process->next = process_list;
    process_list = new_process;

    return new_process;
}

void terminate_process(PCB* process) {
    if (!process) return;
    process->state = PROCESS_TERMINATED;

    // Remove process from the list.
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

PCB* get_current_process(void) {
    return current_process;
}

void schedule(void) {
    PCB* proc = process_list;
    while (proc) {
        if (proc->state == PROCESS_READY) {
            current_process = proc;
            proc->state = PROCESS_RUNNING;
            // In a real OS, a context switch would be performed.
            if (proc->entry_point) {
                proc->entry_point();
            }
            proc->state = PROCESS_TERMINATED;
            terminate_process(proc);
            break;
        }
        proc = proc->next;
    }
}