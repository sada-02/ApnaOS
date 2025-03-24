#include "process.h"
#include "memory.h"

extern void debug_print(const char *);
extern void int_to_hex(uint32_t, char *);
extern void int_to_dec(uint32_t, char *);
extern void print_to_screen(const char *);
extern void debug_int(uint32_t);

/* Global variables */
PCB *current_process = NULL;
ProcessQueue ready_queue = {NULL, NULL};
static uint32_t next_pid = 1;

/* Queue management implementations */
void initialize_queue(ProcessQueue *queue)
{
    queue->front = NULL;
    queue->rear = NULL;
}

bool is_queue_empty(ProcessQueue *queue)
{
    return queue->front == NULL;
}

void enqueue_process(ProcessQueue *queue, PCB *process)
{
    process->next = NULL;

    if (is_queue_empty(queue))
    {
        queue->front = process;
        queue->rear = process;
        return;
    }

    queue->rear->next = process;
    queue->rear = process;
}

PCB *dequeue_process(ProcessQueue *queue)
{
    if (is_queue_empty(queue))
    {
        return NULL;
    }

    PCB *process = queue->front;
    queue->front = queue->front->next;

    if (queue->front == NULL)
    {
        queue->rear = NULL;
    }

    process->next = NULL;
    debug_print("DEBUG: Dequeued process has pid:");
    debug_int(process->pid);
    return process;
}

/* Context switching implementations */
void save_context(PCB *process, uint32_t *stack_ptr, uint32_t *return_addr)
{
    process->stack_pointer = stack_ptr;
    process->program_counter = return_addr;

    if (process->state == STATE_RUNNING)
    {
        process->state = STATE_READY;
    }
}

void restore_context(PCB *process)
{
    __asm__ volatile("movl %0, %%cr3" : : "r"(process->cr3));
    __asm__ volatile("movl %0, %%esp" : : "r"(process->stack_pointer));
    process->state = STATE_RUNNING;
    debug_print("Restored context.");
}

void context_switch(uint32_t *current_stack_ptr, uint32_t *return_addr)
{
    debug_print("DEBUG: Context switch");

    if (current_process != NULL)
    {
        char buffer[32];
        int_to_dec(current_process->state, buffer);
        debug_print("DEBUG: Current process state: ");
        debug_print(buffer);

        save_context(current_process, current_stack_ptr, return_addr);
        if (current_process->state == STATE_READY)
        {
            enqueue_process(&ready_queue, current_process);
        }
    }

    current_process = dequeue_process(&ready_queue);
    if (current_process == NULL)
    {
        debug_print("DEBUG: No more processes in ready queue");
        return;
    }

    restore_context(current_process);
    debug_print("DEBUG: Switched to process:");
    debug_int(current_process->pid);

    // Jump to the process's program counter
    __asm__ volatile("jmp *%0" : : "r"(current_process->program_counter));
}

void schedule()
{
    uint32_t *current_stack_ptr;
    uint32_t *return_addr;

    // Get the return address directly
    __asm__ volatile(
        "movl 4(%%ebp), %0" : "=r"(return_addr));

    // Get the current stack pointer
    __asm__ volatile("movl %%esp, %0" : "=r"(current_stack_ptr));

    debug_print("DEBUG: Return address in schedule:");
    debug_int((uint32_t)return_addr);

    // Check if current process has exited
    if (current_process != NULL && current_process->state == STATE_EXIT)
    {
        debug_print("DEBUG: Process terminated");
        // Don't re-enqueue terminated processes
        PCB *old_process = current_process;
        current_process = NULL;
        context_switch(current_stack_ptr, return_addr);
    }
    else
    {
        // No current process, just switch to next
        context_switch(current_stack_ptr, return_addr);
    }
}

/* Process creation implementation */
PCB *create_process(uint32_t pid, uint32_t *entry_point, uint32_t *stack_base)
{
    PCB *new_process = (PCB *)kmalloc(sizeof(PCB));
    if (new_process == NULL)
    {
        return NULL;
    }

    new_process->pid = pid;
    new_process->state = STATE_NEW;
    new_process->stack_pointer = stack_base;
    new_process->program_counter = entry_point;

    for (int i = 0; i < 8; i++)
    {
        new_process->registers[i] = 0;
    }

    new_process->cr3 = 0;
    new_process->next = NULL;

    new_process->state = STATE_READY;
    enqueue_process(&ready_queue, new_process);

    return new_process;
}

uint32_t get_new_pid()
{
    return next_pid++;
}

/* Initialization */
void init_process_management()
{
    initialize_queue(&ready_queue);
    debug_print("DEBUG: Process queues initialized.");

    // Create initial process (usually idle process)
    // This would be implemented based on your specific requirements

    debug_print("DEBUG: Process management system initialized.");
}

// --- System Call Implementations ---

// int fork_syscall(void) {
//     debug_print("oggy fork check 1.\n");
//     PCB* parent = get_current_process();
//     if (!parent) return -1;
//     debug_print("fork check 2.\n");
//     PCB* child = allocate_pcb();

//     if (!child) return -1;

//     child->pid = next_pid++;
//     child->state = PROCESS_READY;
//     child->entry_point = parent->entry_point;  // duplicate parent's code pointer.
//     child->parent = parent;
//     child->fork_ret = 0;  // Child sees fork() return 0.
//     child->exit_status = 0;

//     debug_print("fork check 3.\n");
//     size_t used_bytes = (parent->stack + STACK_SIZE) - (uint8_t*)parent->kernel_sp;
//     child->kernel_sp = (uint32_t*)(child->stack + STACK_SIZE - used_bytes);

//     memcpy(child->kernel_sp, parent->kernel_sp, used_bytes);
//     child->next = process_list;
//     process_list = child;
//     debug_print("fork check 4.\n");

//     return child->pid;  // Parent returns child's PID.
// }

// int sys_exec(void (*new_entry)(void)) {
//     PCB* curr = get_current_process();
//     if (!curr) return -1;
//     curr->entry_point = new_entry;
//     // Reinitialize kernel stack for the new image.
//     curr->kernel_sp = (uint32_t*)(curr->stack + STACK_SIZE);
//     *(--curr->kernel_sp) = (uint32_t)sys_exit;
//     *(--curr->kernel_sp) = (uint32_t)new_entry;
//     return 0;
// }

// static PCB* find_child(PCB* parent) {
//     PCB* temp = process_list;
//     while (temp) {
//         if (temp->parent == parent)
//             return temp;
//         temp = temp->next;
//     }
//     return 0;
// }

// int sys_wait(int* status) {
//     // Get the parent process (the one that called wait)
//     debug_print("oggy wait check 1.\n");
//     PCB* parent = get_current_process();
//     if (!parent) return -1;
//     debug_print("oggy wait check 2.\n");

//     // Mark the parent as WAITING so the scheduler will not pick it until a child completes.
//     parent->state = PROCESS_WAITING;

//     // Wait until all children of this parent have terminated.
//     while (1) {
//         debug_print("oggy wait check 3.\n");
//         PCB* child = NULL;
//         PCB* temp = process_list;
//         // Look for any child of the parent.
//         while (temp) {
//             debug_print("oggy wait check 4.\n");
//             if (temp->parent == parent) {
//                 child = temp;
//                 break;
//             }
//             temp = temp->next;
//         }
//         // If there is no child at all, we're done waiting.
//         if (child == NULL)
//             break;
//         // If the found child is terminated, then reap it.
//         if (child->state == PROCESS_TERMINATED) {
//             debug_print("oggy wait check 5.\n");
//             if (status)
//                 *status = child->exit_status;
//             int child_pid = child->pid;
//             terminate_process(child);
//             // Continue checking for any remaining children.
//             continue;
//         }
//         // Otherwise, yield to allow child processes to run.

//         debug_print("oggy wait check 6.\n");
//         yield();
//     }

//     // All children are done; restore the parent's state so it can run.
//     parent->state = PROCESS_READY;
//     return 0;  // Optionally, return the last child's PID or similar.
// }

// void sys_exit(int status) {
//     PCB* curr = get_current_process();
//     if (!curr) return;
//     curr->exit_status = status;
//     curr->state = PROCESS_TERMINATED;
//     schedule();
//     while (1);
// }

// int sys_read(int fd, void* buffer, size_t size) {
//     if (!buffer || size == 0)
//         return -1;

//     // For now, only handle standard input (fd == 0).
//     if (fd == 0) {
//         return read_line((char*)buffer, size);
//     }

//     return -1; // Unsupported file descriptor.
// }
