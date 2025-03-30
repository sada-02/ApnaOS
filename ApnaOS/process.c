#include "process.h"
#include "memory.h"
#include "syscall.h"

PCB* current_process = NULL;
ProcessQueue ready_queue = {NULL, NULL};
PCB* process_table_head = NULL;
static uint32_t next_pid = 1;

uint32_t get_new_pid() {
    return next_pid++;
}

void initialize_queue(ProcessQueue* queue) {
    queue->front = NULL;
    queue->rear = NULL;
}

bool is_queue_empty(ProcessQueue* queue) {
    return queue->front == NULL;
}

/* New function: Enqueue process sorted by priority (lower number = higher priority)
   For processes with the same priority, insertion order (FCFS) is preserved. */
void enqueue_process(ProcessQueue* queue, PCB* process) {
    process->next = NULL;

    // If the queue is empty, simply insert.
    if (is_queue_empty(queue)) {
        queue->front = process;
        queue->rear = process;
        return;
    }

    // If the new process has higher priority than the front process, insert at the beginning.
    if (process->priority < queue->front->priority) {
        process->next = queue->front;
        queue->front = process;
        return;
    }

    PCB* current = queue->front;
    // Traverse until you find a process with greater priority value.
    // For same priority, preserve the order (i.e. insert after).
    while (current->next && current->next->priority <= process->priority) {
        current = current->next;
    }
    process->next = current->next;
    current->next = process;
    if (process->next == NULL) {
        queue->rear = process;
    }
}

PCB* dequeue_process(ProcessQueue* queue) {
    if (is_queue_empty(queue)) {
        return NULL;
    }
    
    PCB* process = queue->front;
    queue->front = queue->front->next;
    
    if (queue->front == NULL) {
        queue->rear = NULL;
    }
    
    process->next = NULL;
    debug_print("DEBUG: Dequeued process has pid:");
    debug_int(process->pid);
    return process;
}

void allocate_kernel_stack(PCB* process) {
    process->kernel_stack_base = (uint32_t*)kmalloc(KERNEL_STACK_SIZE);
    process->kernel_stack_ptr = process->kernel_stack_base + (KERNEL_STACK_SIZE/sizeof(uint32_t));
}

/* Modified schedule: Assumes that processes in the ready_queue are sorted by priority */
void schedule() {
    // Save current process state if it's running.
    if (current_process != NULL) {
        if (current_process->state == STATE_RUNNING) {
            current_process->state = STATE_READY;
            // Reinsert using our priority-queue function.
            enqueue_process(&ready_queue, current_process);
        }
    }
    
    PCB* next_process = dequeue_process(&ready_queue);
    if (next_process == NULL) {
        debug_print("DEBUG: No more processes in ready queue");
        cli_loop();
        while(1);
    }
    
    next_process->state = STATE_RUNNING;
    
    debug_print("DEBUG: Switching to process:");
    debug_int(next_process->pid);
    
    // If this is a newly forked process, adjust registers so fork returns 0.
    if (next_process->is_new_child) {
        next_process->is_new_child = false;
        
        current_process = next_process;
        __asm__ volatile (
            "xorl %%eax, %%eax\n\t"  // Child sees fork returning 0
            "movl %0, %%esp\n\t"     // Load process user stack pointer
            "popl %%ebp\n\t"         // Restore base pointer
            "ret\n\t"               // Jump to saved instruction pointer
            : : "r" (next_process->user_stack_ptr) : "eax"
        );
    }
    
    if (current_process != NULL) {
        __asm__ volatile (
            "movl %%esp, %0\n\t"
            : "=m" (current_process->kernel_stack_ptr)
        );
    }
    
    current_process = next_process;
    
    __asm__ volatile (
        "movl %0, %%esp\n\t"     // Load next process user stack pointer
        "popl %%ebp\n\t"         // Restore next process base pointer
        "ret\n\t"                // Return to the saved instruction pointer
        : : "r" (next_process->user_stack_ptr)
    );
}

/* Modified create_process to accept a priority parameter.
   The process's user stack is set up, the kernel stack allocated,
   and the process is enqueued in the ready queue sorted by priority. */
PCB* create_process(uint32_t pid, uint32_t* entry_point, uint32_t* stack_top, int priority) {
    PCB* new_process = (PCB*) kmalloc(sizeof(PCB));
    if (new_process == NULL) {
        return NULL;
    }

    new_process->pid = pid;
    new_process->state = STATE_NEW;
    new_process->priority = priority;  // Set process priority

    // Allocate user stack (for example, 1024 words below the stack_top)
    new_process->user_stack_base = stack_top - 1024;

    // Prepare user stack:
    // Push the entry point as the return address.
    *(--stack_top) = (uint32_t)entry_point;
    // Optionally, push a dummy value for EBP.
    *(--stack_top) = 0x0;
    
    new_process->user_stack_ptr = stack_top;
    new_process->program_counter = entry_point;

    new_process->cr3 = 0;
    new_process->next = NULL;

    allocate_kernel_stack(new_process);
    
    // Add process to global process table.
    new_process->next_in_table = process_table_head;
    process_table_head = new_process;

    new_process->state = STATE_READY;
    // Enqueue using the priority-based method.
    enqueue_process(&ready_queue, new_process);

    return new_process;
}

void init_process_management() {
    initialize_queue(&ready_queue);
    debug_print("DEBUG: Process queues initialized.");
    debug_print("DEBUG: Process management system initialized.");
}
