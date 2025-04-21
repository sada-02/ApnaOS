#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>
#include <stdbool.h>

#define KERNEL_STACK_SIZE 4096

#define STATE_READY    0
#define STATE_RUNNING  1
#define STATE_BLOCKED  2
#define STATE_NEW      3
#define STATE_EXIT     4
#define STATE_ZOMBIE   5

typedef struct PCB {
    uint32_t pid;
    uint32_t state;
    uint32_t* user_stack_ptr;
    uint32_t* program_counter;
    uint32_t cr3;
    int priority;  // Process priority (lower number = higher priority)

    struct PCB* next;           // Next in the ready queue
    struct PCB* next_in_table;  // Next in the process table
    struct PCB* parent;

    int exit_status;
    bool is_new_child;
    
    uint32_t* user_stack_base;    // User stack base
    uint32_t* kernel_stack_base;  // Kernel stack base
    uint32_t* kernel_stack_ptr;   // Current kernel stack pointer
} PCB;

extern PCB* process_table_head;  // Global linked list of all processes

typedef struct {
    PCB* front;
    PCB* rear;
} ProcessQueue;

extern PCB* current_process;
extern ProcessQueue ready_queue;

void initialize_queue(ProcessQueue* queue);
bool is_queue_empty(ProcessQueue* queue);
void enqueue_process(ProcessQueue* queue, PCB* process);
PCB* dequeue_process(ProcessQueue* queue);
void allocate_kernel_stack(PCB* process);

void schedule(void);
PCB* create_process(uint32_t pid, uint32_t* entry_point, uint32_t* stack_base, int priority);
uint32_t get_new_pid(void);

void init_process_management(void);

void process_test(void);

#endif // PROCESS_H
