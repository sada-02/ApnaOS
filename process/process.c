#include "process.h"
#include "../memory/memory.h"
#include "syscall.h"

PCB* current_process = NULL;
ProcessQueue ready_queue = {NULL, NULL};
PCB* process_table_head = NULL;
static uint32_t next_pid = 1;

extern void debug_print(const char* messe);
extern void print_to_screen(const char* message);
extern void debug_int(int val);
extern void cli_loop(void);

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
void enqueue_process(ProcessQueue* queue, PCB* process) {
    process->next = NULL;
    if (is_queue_empty(queue)) {
        queue->front = process;
        queue->rear = process;
        return;
    }
    if (process->priority < queue->front->priority) {
        process->next = queue->front;
        queue->front = process;
        return;
    }

    PCB* current = queue->front;
    while (current->next && current->next->priority <= process->priority) {
        current = current->next;
    }
    process->next = current->next;
    current->next = process;
    if (process->next == NULL) {
        queue->rear = process;
    }
}
void enqueue_process_edf(ProcessQueue* queue, PCB* process) {
    process->next = NULL;
    if (is_queue_empty(queue)) {
        queue->front = process;
        queue->rear = process;
        return;
    }
    if (process->deadline < queue->front->deadline) {
        process->next = queue->front;
        queue->front = process;
        return;
    }

    PCB* current = queue->front;
    while (current->next && current->next->deadline <= process->deadline) {
        current = current->next;
    }
    process->next = current->next;
    current->next = process;
    if (process->next == NULL) {
        queue->rear = process;
    }
}
void enqueue_process_sjf(ProcessQueue* queue, PCB* process) {
    process->next = NULL;
    if (is_queue_empty(queue)) {
        queue->front = process;
        queue->rear = process;
        return;
    }
    if (process->time_to_run < queue->front->time_to_run) {
        process->next = queue->front;
        queue->front = process;
        return;
    }

    PCB* current = queue->front;
    while (current->next && current->next->time_to_run <= process->time_to_run) {
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
void schedule() {
    if (current_process != NULL) {
        if (current_process->state == STATE_RUNNING) {
            current_process->state = STATE_READY;
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
    if (next_process->is_new_child) {
        next_process->is_new_child = false;
        
        current_process = next_process;
        __asm__ volatile (
            "xorl %%eax, %%eax\n\t"  
            "movl %0, %%esp\n\t"     
            "popl %%ebp\n\t"        
            "ret\n\t"              
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
        "movl %0, %%esp\n\t"     
        "popl %%ebp\n\t"         
        "ret\n\t"                
        : : "r" (next_process->user_stack_ptr)
    );
}


PCB* create_process(uint32_t pid, uint32_t* entry_point, int priority, int deadline, int time_to_run) {
    PCB* new_process = (PCB*) kmalloc(sizeof(PCB));
    if (new_process == NULL) {
        return NULL;
    }

    uint32_t *stack_top = (uint32_t *) kmalloc(4096);
    if (!stack_top) {
        return NULL;
    }
    stack_top += 4096 / sizeof(uint32_t);

    new_process->pid = pid;
    new_process->state = STATE_NEW;
    new_process->priority = priority;  
    new_process->deadline = deadline;
    new_process->time_to_run = time_to_run;
    new_process->user_stack_base = stack_top - 1024;
    *(--stack_top) = (uint32_t)entry_point;
    *(--stack_top) = 0x0;
    
    new_process->user_stack_ptr = stack_top;
    new_process->program_counter = entry_point;

    new_process->cr3 = 0;
    new_process->next = NULL;

    allocate_kernel_stack(new_process);
    new_process->next_in_table = process_table_head;
    process_table_head = new_process;

    new_process->state = STATE_READY;
    enqueue_process(&ready_queue, new_process);
    return new_process;
}

void init_process_management() {
    initialize_queue(&ready_queue);
    debug_print("DEBUG: Process queues initialized.");
    debug_print("DEBUG: Process management system initialized.");
}
