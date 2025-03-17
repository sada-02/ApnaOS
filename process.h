#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>
#include <stddef.h>

// Process states.
typedef enum {
    PROCESS_NEW,
    PROCESS_READY,
    PROCESS_RUNNING,
    PROCESS_WAITING,
    PROCESS_TERMINATED
} ProcessState;

// Process Control Block (PCB) structure.
typedef struct PCB {
    uint32_t pid;                  // Process identifier.
    ProcessState state;            // Current state of the process.
    void (*entry_point)(void);     // Pointer to the process’ entry function.
    void* stack_pointer;           // Pointer to the process stack (for context switching).
    struct PCB* next;              // Pointer to the next PCB (for linking).
} PCB;

// Initialize process management.
void init_process_management(void);

// Create a new process with the given entry point.
PCB* create_process(void (*entry_point)(void));

// Terminate a process and free its resources.
void terminate_process(PCB* process);

// Retrieve the currently running process.
PCB* get_current_process(void);

// A simple scheduler that finds a READY process, runs it, and terminates it.
void schedule(void);

#endif // PROCESS_H