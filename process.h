#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>
#include <stddef.h>

#define STACK_SIZE 4096

// Process states.
typedef enum {
    PROCESS_NEW,
    PROCESS_READY,
    PROCESS_RUNNING,
    PROCESS_WAITING,
    PROCESS_TERMINATED
} ProcessState;

typedef struct PCB {
    uint32_t pid;                  // Process identifier.
    ProcessState state;            // Current state.
    void (*entry_point)(void);     // Process function pointer.
    struct PCB* next;              // Next process in the list.

    // For system calls.
    struct PCB* parent;            // Parent process.
    int fork_ret;                  // fork() return: 0 in child, child's PID in parent.
    int exit_status;               // Exit status.

    // Kernel stack for context switching.
    uint32_t *kernel_sp;           // Saved kernel stack pointer.
    uint8_t stack[STACK_SIZE];     // Dedicated kernel stack.
} PCB;

// Process management function prototypes.
void init_process_management(void);
PCB* create_process(void (*entry_point)(void));
void terminate_process(PCB* process);
PCB* get_current_process(void);
void schedule(void);
void yield(void);
int processes_remain(void);

// System call prototypes.
int fork_syscall(void);
int sys_exec(void (*new_entry)(void));
int sys_wait(int* status);
void sys_exit(int status);

// Advanced context-switch function implemented in assembly.
extern void switch_context(uint32_t **old_sp, uint32_t *new_sp);

#endif // PROCESS_H