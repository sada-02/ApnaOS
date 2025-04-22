#include "../process/process.h"
#include "../memory/memory.h"
#include "../process/syscall.h"

extern void debug_print(const char* messe);
extern void print_to_screen(const char* message);
extern void debug_int(int val);
extern void serial_print(const char* message);

void test_process_1(void);
void test_process_2(void);
void test_process_3(void);

static int test_counter = 0;
static PCB* test_processes[10];

void test_process_1(void) {
    debug_print("DEBUG: Test process 1 running");                                                   
    int x=4;

    for(int i=0; i<3; i++) {
        serial_print("DEBUG: Test process 1 iteration ");
        debug_int(i);
        serial_print("x is "); debug_int(x);
        serial_print("and it is stored at location: "); debug_int((uint32_t)&x);
        yield_syscall();
    }
    
    debug_print("DEBUG: Test process 1 complete");
    current_process->state = STATE_EXIT;
    yield_syscall();
}

void test_process_2(void) {
    debug_print("DEBUG: Test process 2 running");
    int y=5;
    for(int i=0; i<2; i++) {
        serial_print("DEBUG: Test process 2 iteration ");
        debug_int(i);
        serial_print("y is "); debug_int(y);
        serial_print("and it is stored at location: "); debug_int((uint32_t)&y);
        yield_syscall();
    }
    
    debug_print("DEBUG: Test process 2 complete");
    current_process->state = STATE_EXIT;
    yield_syscall();
}

void test_process_3(void) {
    debug_print("DEBUG: Test process 3 running");
    debug_print("DEBUG: Test process 3 complete");
    current_process->state = STATE_EXIT;
    yield_syscall();
}

void test_queue_operations(void) {
    debug_print("DEBUG: Testing queue operations");
    
    ProcessQueue test_queue;
    initialize_queue(&test_queue);
    
    if (is_queue_empty(&test_queue)) {
        debug_print("DEBUG: Queue is empty - PASS");
    } else {
        debug_print("DEBUG: Queue should be empty - FAIL");
    }
    
    PCB* p1 = create_process(get_new_pid(), (uint32_t*)test_process_1, (uint32_t*)kmalloc(4096) , 1, 2 , 3);
    PCB* p2 = create_process(get_new_pid(), (uint32_t*)test_process_2, (uint32_t*)kmalloc(4096) , 1, 4, 5);
    
    enqueue_process(&test_queue, p1);
    if (!is_queue_empty(&test_queue)) {
        debug_print("DEBUG: Queue has one process - PASS");
    } else {
        debug_print("DEBUG: Queue should have one process - FAIL");
    }
    
    enqueue_process(&test_queue, p2);
    
    PCB* dequeued = dequeue_process(&test_queue);
    if (dequeued == p1) {
        debug_print("DEBUG: Dequeued first process - PASS");
    } else {
        debug_print("DEBUG: Dequeued wrong process - FAIL");
    }
    
    dequeued = dequeue_process(&test_queue);
    if (dequeued == p2) {
        debug_print("DEBUG: Dequeued second process - PASS");
    } else {
        debug_print("DEBUG: Dequeued wrong process - FAIL");
    }
    
    if (is_queue_empty(&test_queue)) {
        debug_print("DEBUG: Queue is empty after dequeuing - PASS");
    } else {
        debug_print("DEBUG: Queue should be empty after dequeuing - FAIL");
    }
    
    debug_print("DEBUG: Queue operations test complete");
}

void test_process_creation(void) {
    debug_print("DEBUG: Testing process creation");
    
    uint32_t initial_pid = get_new_pid();
    PCB* p1 = create_process(get_new_pid(), (uint32_t*)test_process_1, (uint32_t*)kmalloc(4096) , 1, 2 , 3);
    
    if (p1 != NULL) {
        debug_print("DEBUG: Process created successfully - PASS");
    } else {
        debug_print("DEBUG: Process creation failed - FAIL");
        return;
    }
    
    if (p1->pid == initial_pid + 1) {
        debug_print("DEBUG: PID assigned correctly - PASS");
    } else {
        debug_print("DEBUG: PID assignment incorrect - FAIL");
    }
    
    if (p1->state == STATE_READY) {
        debug_print("DEBUG: Process state is READY - PASS");
    } else {
        debug_print("DEBUG: Process state should be READY - FAIL");
    }
    
    debug_print("DEBUG: Process creation test complete");
}

void test_scheduler(void) {
    debug_print("DEBUG: Testing scheduler started.");
    
    // Clear the ready queue
    while (!is_queue_empty(&ready_queue)) {
        dequeue_process(&ready_queue);
    }
    
    // Create test processes with proper stack allocation
    uint32_t* stack1 = (uint32_t*)kmalloc(4096);
    uint32_t* stack2 = (uint32_t*)kmalloc(4096);
    uint32_t* stack3 = (uint32_t*)kmalloc(4096);

    // Set stack pointers to the top of the stack (stacks grow downward)
    stack1 += 1024;  // 4096 bytes / 4 bytes per uint32_t = 1024 elements
    stack2 += 1024;
    stack3 += 1024;

    test_processes[0] = create_process(get_new_pid(), (uint32_t*)test_process_1, stack1 , 1, 2 , 3);
    test_processes[1] = create_process(get_new_pid(), (uint32_t*)test_process_2, stack2 , 1, 4, 5);
    test_processes[2] = create_process(get_new_pid(), (uint32_t*)test_process_3, stack3 , 1, 6, 7);
    
    debug_print("DEBUG: Created test processes");

    for(int i=0;i<5;i++){
        debug_print("DEBUG: Scheduler iteration");
        yield_syscall();
    }
    
    debug_print("DEBUG: Scheduler test complete");
    current_process->state = STATE_EXIT;
    yield_syscall();
}

void process_test(void) {
    debug_print("DEBUG: Starting comprehensive process management test");
    
    // test_queue_operations();
    // test_process_creation();
    create_process(get_new_pid(), (uint32_t*)test_scheduler, (uint32_t*)kmalloc(4096)+1024 , 1, 2 , 3);
    schedule();
    
    debug_print("DEBUG: Comprehensive process management test complete");
}
