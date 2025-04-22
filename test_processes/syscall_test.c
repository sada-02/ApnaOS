#include "../process/process.h"
#include "../process/syscall.h"
#include "../memory/memory.h"
extern void debug_print(const char* messe);
extern void print_to_screen(const char* message);
extern void debug_int(int val);
extern void serial_print(const char* message);
void test_simple_fork(void) {
    debug_print("DEBUG: Testing simple fork");
    int child_pid = fork_syscall();
    debug_print("DEBUG: Fork returned with PID:");
    debug_int(child_pid);

    if (child_pid == 0) {
        debug_print("DEBUG: Child process running");
        debug_print("DEBUG: Child process exiting");
        exit_syscall(42); 
    } else if (child_pid > 0) {
        int original_child_pid = child_pid;
        int status = 0;
        int wait_result = wait_syscall(&status);
        debug_print("DEBUG: Wait returned with PID:");
        debug_int(wait_result);
        debug_print("DEBUG: Child exit status:");
        debug_int(status);
        
        if (wait_result == original_child_pid && status == 42) {
            debug_print("DEBUG: Simple fork test PASSED");
        } else {
            debug_print("DEBUG: Simple fork test FAILED");
        }
    } else {
        debug_print("DEBUG: Fork failed");
    }
    current_process->state = STATE_EXIT;
    yield_syscall();
}

void test_multiple_forks(void) {
    debug_print("DEBUG: Testing multiple forks");
    
    int child_pids[3];
    int fork_count = 0;
    child_pids[0] = fork_syscall();
    if (child_pids[0] == 0) {
        debug_print("DEBUG: First child process running");
        exit_syscall(10);
    } else if (child_pids[0] > 0) {
        fork_count++;
        child_pids[1] = fork_syscall();
        if (child_pids[1] == 0) {
            debug_print("DEBUG: Second child process running");
            exit_syscall(20);
        } else if (child_pids[1] > 0) {
            fork_count++;
            
            // Create third child
            child_pids[2] = fork_syscall();
            if (child_pids[2] == 0) {
                // Third child process
                debug_print("DEBUG: Third child process running");
                exit_syscall(30);
            } else if (child_pids[2] > 0) {
                fork_count++;
                
                // Parent process - wait for all children
                debug_print("DEBUG: Parent created 3 children");
                
                int status = 0;
                int children_collected = 0;
                int total_status = 0;
                
                // Wait for all children to terminate
                for (int i = 0; i < fork_count; i++) {
                    int wait_result = wait_syscall(&status);
                    
                    debug_print("DEBUG: Wait returned with PID:");
                    debug_int(wait_result);
                    debug_print("DEBUG: Child exit status:");
                    debug_int(status);
                    
                    // Check if we got a valid PID
                    if (wait_result > 0) {
                        children_collected++;
                        total_status += status;
                    }
                }
                
                // Verify all children were collected with correct statuses
                if (children_collected == 3 && total_status == 60) {
                    debug_print("DEBUG: Multiple forks test PASSED");
                } else {
                    debug_print("DEBUG: Multiple forks test FAILED");
                    debug_print("DEBUG: Children collected:");
                    debug_int(children_collected);
                    debug_print("DEBUG: Total status:");
                    debug_int(total_status);
                }
            } else {
                debug_print("DEBUG: Third fork failed");
            }
        } else {
            debug_print("DEBUG: Second fork failed");
        }
    } else {
        debug_print("DEBUG: First fork failed");
    }
    current_process->state = STATE_EXIT;
    yield_syscall();
}

void syscall_test(void) {
    debug_print("DEBUG: Starting fork and wait syscall tests");

    uint32_t* parent_stack = (uint32_t*)kmalloc(4096);
    uint32_t* parent_stack_top = parent_stack + (4096/sizeof(uint32_t) - 16);
    
    PCB* parent_process = create_process(get_new_pid(), (uint32_t*)test_simple_fork, parent_stack_top ,1, 2, 3);
    if (parent_process == NULL) {
        debug_print("DEBUG: Failed to create parent process");
        return;
    }
    
    debug_print("DEBUG: Fork and wait syscall tests complete");
    exit_syscall(0); 
}
