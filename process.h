#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>
#include <stdbool.h>

/* Process states */
#define STATE_READY    0
#define STATE_RUNNING  1
#define STATE_BLOCKED  2
#define STATE_NEW      3
#define STATE_EXIT     4
#define STATE_ZOMBIE   5

/* Process Control Block structure */
typedef struct PCB {
    uint32_t pid;
    uint32_t state;
    uint32_t* stack_pointer;
    uint32_t* program_counter;
    uint32_t registers[8];
    uint32_t cr3;
    struct PCB* next;
} PCB;

/* Process queue structure */
typedef struct {
    PCB* front;
    PCB* rear;
} ProcessQueue;

/* Global variables */
extern PCB* current_process;
extern ProcessQueue ready_queue;

/* Queue management */
void initialize_queue(ProcessQueue* queue);
bool is_queue_empty(ProcessQueue* queue);
void enqueue_process(ProcessQueue* queue, PCB* process);
PCB* dequeue_process(ProcessQueue* queue);

/* Context switching */
void save_context(PCB* process, uint32_t* stack_ptr, uint32_t* program_counter);
void restore_context(PCB* process);
void context_switch(uint32_t* current_stack_ptr, uint32_t* current_pc);
void schedule(void);

/* Process creation/management */
PCB* create_process(uint32_t pid, uint32_t* entry_point, uint32_t* stack_base);
uint32_t get_new_pid(void);

/* Initialization */
void init_process_management(void);

/* Test function */
void process_test(void);

// System call prototypes.
// int fork_syscall(void);
// int sys_exec(void (*new_entry)(void));
// int sys_wait(int* status);
// void sys_exit(int status);

#endif // PROCESS_H