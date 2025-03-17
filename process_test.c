#include "process.h"

// Declare the dummy process functions.
extern void dummy_process_1(void);
extern void dummy_process_2(void);
extern void dummy_process_3(void);

// Use the OS-specific print function.
extern void print_to_screen(const char*);

// Function to test process management.
void process_test(void) {
    // Create three dummy processes.
    create_process(dummy_process_1);
    create_process(dummy_process_2);
    create_process(dummy_process_3);

    // Run the scheduler three times to execute each process.
    schedule();
    schedule();
    schedule();

    print_to_screen("All dummy processes have been executed.\n");
}