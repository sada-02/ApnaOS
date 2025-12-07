#include "../process/process.h"
#include "../process/syscall.h"
#include "../memory/memory.h"

extern void print_to_screen(const char* message);

void multisys(void) {   
    int child_pids[3];
    int fork_count = 0;
    
    child_pids[0] = fork_syscall();
    if (child_pids[0] == 0) {
        print_to_screen("[CHILD 1] I'm child 1, doing work...\n");
        for (int i = 0; i < 2; i++) {
            yield_syscall();
        }
        print_to_screen("[CHILD 1] Exiting with status 10\n");
        exit_syscall(10);
    } else if (child_pids[0] > 0) {
        fork_count++;
        
        child_pids[1] = fork_syscall();
        if (child_pids[1] == 0) {
            print_to_screen("[CHILD 2] I'm child 2, doing work...\n");
            for (int i = 0; i < 2; i++) {
                yield_syscall();
            }
            print_to_screen("[CHILD 2] Exiting with status 20\n");
            exit_syscall(20);
        } else if (child_pids[1] > 0) {
            fork_count++;
            
            child_pids[2] = fork_syscall();
            if (child_pids[2] == 0) {
                print_to_screen("[CHILD 3] I'm child 3, doing work...\n");
                for (int i = 0; i < 2; i++) {
                    yield_syscall();
                }
                print_to_screen("[CHILD 3] Exiting with status 30\n");
                exit_syscall(30);
            } else if (child_pids[2] > 0) {
                fork_count++;
                
                print_to_screen("[PARENT] Successfully created 3 children\n");
                print_to_screen("[PARENT] Child PIDs: ");
                for (int i = 0; i < 3; i++) {
                    char buf[10];
                    int temp = child_pids[i];
                    int j = 0;
                    if (temp == 0) buf[j++] = '0';
                    while (temp > 0) {
                        buf[j++] = '0' + (temp % 10);
                        temp /= 10;
                    }
                    buf[j] = '\0';
                    for (int k = 0; k < j/2; k++) {
                        char tmp = buf[k];
                        buf[k] = buf[j-1-k];
                        buf[j-1-k] = tmp;
                    }
                    print_to_screen(buf);
                    if (i < 2) print_to_screen(", ");
                }
                print_to_screen("\n[PARENT] Waiting for all children...\n\n");
                
                int status = 0;
                int children_collected = 0;
                int total_status = 0;
                
                for (int i = 0; i < fork_count; i++) {
                    int wait_result = wait_syscall(&status);
                    
                    if (wait_result > 0) {
                        print_to_screen("[PARENT] Child PID ");
                        char buf[10];
                        int temp = wait_result;
                        int j = 0;
                        if (temp == 0) buf[j++] = '0';
                        while (temp > 0) {
                            buf[j++] = '0' + (temp % 10);
                            temp /= 10;
                        }
                        buf[j] = '\0';
                        for (int k = 0; k < j/2; k++) {
                            char tmp = buf[k];
                            buf[k] = buf[j-1-k];
                            buf[j-1-k] = tmp;
                        }
                        print_to_screen(buf);
                        print_to_screen(" exited with status: ");
                        buf[0] = '0' + (status / 10);
                        buf[1] = '0' + (status % 10);
                        buf[2] = '\0';
                        print_to_screen(buf);
                        print_to_screen("\n");
                        children_collected++;
                        total_status += status;
                    }
                }
                
                print_to_screen("\n[PARENT] All children collected!\n");
                print_to_screen("[PARENT] Total exit status: ");
                char buf[10];
                int temp = total_status;
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
                print_to_screen("\n");
                
                if (children_collected == 3 && total_status == 60) {
                    print_to_screen("\n[RESULT] Multiple forks test PASSED ✓\n");
                } else {
                    print_to_screen("\n[RESULT] Multiple forks test FAILED ✗\n");
                }
            } else {
                print_to_screen("[ERROR] Third fork failed\n");
            }
        } else {
            print_to_screen("[ERROR] Second fork failed\n");
        }
    } else {
        print_to_screen("[ERROR] First fork failed\n");
    }
    
    print_to_screen("\n========================================\n");
    current_process->state = STATE_EXIT;
    yield_syscall();
}
