#include "process.h"
extern void print_to_screen(const char*);
extern void fork_exec_test(void);
extern void dummy_process_1(void);
extern void dummy_process_2(void);
extern void dummy_process_3(void);

void process_test(void) {
    debug_print("Starting system call test process.\n");
    debug_print("Hello world \n");
    create_process(fork_exec_test);
    debug_print("Goodbye world \n");
    while (processes_remain()) {
        debug_print("Hello again bruh \n");
        schedule();
        debug_print("Goodbye again bruh \n");
    }
    debug_print("System call test process completed.\n");

    debug_print("Creating Dummy Processes.\n");
    create_process(dummy_process_1);
    create_process(dummy_process_2);
    create_process(dummy_process_3);
    while (processes_remain()) {
        schedule();
    }
    debug_print("All dummy processes have been executed.\n");
}
