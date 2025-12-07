#include "../process/process.h"
#include "../process/syscall.h"
#include "../memory/memory.h"

extern void print_to_screen(const char* message);

void simplesys(void) {
    int child_pid = fork_syscall();

    if (child_pid == 0) {
        print_to_screen("[CHILD] I am the child process!\n");
        print_to_screen("[CHILD] Doing some work...\n");
        for (int i = 0; i < 3; i++) {
            yield_syscall();
        }
        print_to_screen("[CHILD] Exiting with status 42\n");
        exit_syscall(42); 
    } else if (child_pid > 0) {
        print_to_screen("[PARENT] Created child with PID ");
        char buf[10];
        int temp = child_pid;
        int i = 0;
        if (temp == 0) buf[i++] = '0';
        while (temp > 0) {
            buf[i++] = '0' + (temp % 10);
            temp /= 10;
        }
        buf[i] = '\0';
        for (int j = 0; j < i/2; j++) {
            char tmp = buf[j];
            buf[j] = buf[i-1-j];
            buf[i-1-j] = tmp;
        }
        print_to_screen(buf);
        print_to_screen("\n[PARENT] Waiting for child...\n");
        
        int original_child_pid = child_pid;
        int status = 0;
        int wait_result = wait_syscall(&status);
        
        print_to_screen("[PARENT] Child exited with status: ");
        buf[0] = '0' + (status / 10);
        buf[1] = '0' + (status % 10);
        buf[2] = '\0';
        print_to_screen(buf);
        print_to_screen("\n");
        
        if (wait_result == original_child_pid && status == 42) {
            print_to_screen("\n[RESULT] Simple fork test PASSED ✓\n");
        } else {
            print_to_screen("\n[RESULT] Simple fork test FAILED ✗\n");
        }
    } else {
        print_to_screen("[ERROR] Fork failed!\n");
    }
    
    current_process->state = STATE_EXIT;
    yield_syscall();
}
