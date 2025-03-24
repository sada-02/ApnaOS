/* process_test.c - Comprehensive process management test */
#include "process.h"
#include "memory.h"

extern void debug_print(const char *);
extern void int_to_hex(uint32_t, char *);
extern void int_to_dec(uint32_t, char *);
extern void print_to_screen(const char *);
extern void debug_int(uint32_t);

/* Test process function prototypes */
void test_process_1(void);
void test_process_2(void);
void test_process_3(void);

/* Global test variables */
static int test_counter = 0;
static PCB *test_processes[10];

/* Process test entry points */
void test_process_1(void)
{
    debug_print("DEBUG: Test process 1 running");

    // First iteration
    debug_print("DEBUG: Test process 1 iteration 0");
    schedule();

    // Second iteration
    debug_print("DEBUG: Test process 1 iteration 1");
    schedule();

    // Third iteration
    debug_print("DEBUG: Test process 1 iteration 2");

    debug_print("DEBUG: Test process 1 complete");
    current_process->state = STATE_EXIT;
    schedule();
}

void test_process_2(void)
{
    debug_print("DEBUG: Test process 2 running");
    // First iteration
    debug_print("DEBUG: Test process 2 iteration 0");
    schedule();

    // Second iteration
    debug_print("DEBUG: Test process 2 iteration 1");
    schedule();

    debug_print("DEBUG: Test process 2 complete");
    current_process->state = STATE_EXIT;
    schedule();
}

void test_process_3(void)
{
    debug_print("DEBUG: Test process 3 running");
    debug_print("DEBUG: Test process 3 complete");
    current_process->state = STATE_EXIT;
    schedule();
}

/* Test queue operations */
void test_queue_operations(void)
{
    debug_print("DEBUG: Testing queue operations");

    ProcessQueue test_queue;
    initialize_queue(&test_queue);

    if (is_queue_empty(&test_queue))
    {
        debug_print("DEBUG: Queue is empty - PASS");
    }
    else
    {
        debug_print("DEBUG: Queue should be empty - FAIL");
    }

    PCB *p1 = create_process(get_new_pid(), (uint32_t *)test_process_1, (uint32_t *)kmalloc(4096));
    PCB *p2 = create_process(get_new_pid(), (uint32_t *)test_process_2, (uint32_t *)kmalloc(4096));

    enqueue_process(&test_queue, p1);
    if (!is_queue_empty(&test_queue))
    {
        debug_print("DEBUG: Queue has one process - PASS");
    }
    else
    {
        debug_print("DEBUG: Queue should have one process - FAIL");
    }

    enqueue_process(&test_queue, p2);

    PCB *dequeued = dequeue_process(&test_queue);
    if (dequeued == p1)
    {
        debug_print("DEBUG: Dequeued first process - PASS");
    }
    else
    {
        debug_print("DEBUG: Dequeued wrong process - FAIL");
    }

    dequeued = dequeue_process(&test_queue);
    if (dequeued == p2)
    {
        debug_print("DEBUG: Dequeued second process - PASS");
    }
    else
    {
        debug_print("DEBUG: Dequeued wrong process - FAIL");
    }

    if (is_queue_empty(&test_queue))
    {
        debug_print("DEBUG: Queue is empty after dequeuing - PASS");
    }
    else
    {
        debug_print("DEBUG: Queue should be empty after dequeuing - FAIL");
    }

    debug_print("DEBUG: Queue operations test complete");
}

/* Test process creation */
void test_process_creation(void)
{
    debug_print("DEBUG: Testing process creation");

    uint32_t initial_pid = get_new_pid();
    PCB *p1 = create_process(get_new_pid(), (uint32_t *)test_process_1, (uint32_t *)kmalloc(4096));

    if (p1 != NULL)
    {
        debug_print("DEBUG: Process created successfully - PASS");
    }
    else
    {
        debug_print("DEBUG: Process creation failed - FAIL");
        return;
    }

    if (p1->pid == initial_pid + 1)
    {
        debug_print("DEBUG: PID assigned correctly - PASS");
    }
    else
    {
        debug_print("DEBUG: PID assignment incorrect - FAIL");
    }

    if (p1->state == STATE_READY)
    {
        debug_print("DEBUG: Process state is READY - PASS");
    }
    else
    {
        debug_print("DEBUG: Process state should be READY - FAIL");
    }

    debug_print("DEBUG: Process creation test complete");
}

/* Test scheduler */
void test_scheduler(void)
{
    debug_print("DEBUG: Testing scheduler");

    // Clear the ready queue
    while (!is_queue_empty(&ready_queue))
    {
        dequeue_process(&ready_queue);
    }

    // Create test processes with proper stack allocation
    uint32_t *stack1 = (uint32_t *)kmalloc(4096);
    uint32_t *stack2 = (uint32_t *)kmalloc(4096);
    uint32_t *stack3 = (uint32_t *)kmalloc(4096);

    // Set stack pointers to the top of the stack (stacks grow downward)
    stack1 += 1024; // 4096 bytes / 4 bytes per uint32_t = 1024 elements
    stack2 += 1024;
    stack3 += 1024;

    test_processes[0] = create_process(get_new_pid(), (uint32_t *)test_process_1, stack1);
    test_processes[1] = create_process(get_new_pid(), (uint32_t *)test_process_2, stack2);
    test_processes[2] = create_process(get_new_pid(), (uint32_t *)test_process_3, stack3);

    debug_print("DEBUG: Created test processes");

    // Run the scheduler multiple times
    current_process = NULL;
    for (int i = 0; i < 10; i++)
    {
        debug_print("DEBUG: Scheduler iteration");
        schedule();
        if (current_process != NULL)
        {
            char buffer[32];
            int_to_dec(current_process->pid, buffer);
            debug_print("DEBUG: Current process PID: ");
            debug_print(buffer);
        }
    }

    debug_print("DEBUG: Scheduler test complete");
}

/* Main test function */
void process_test(void)
{
    debug_print("DEBUG: Starting comprehensive process management test");

    test_queue_operations();
    test_process_creation();
    test_scheduler();

    debug_print("DEBUG: Comprehensive process management test complete");
}
