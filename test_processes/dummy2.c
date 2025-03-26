// Dummy process 1 for your custom OS.
// Uses the OS-specific print function for output.
extern void print_to_screen(const char*);
extern void exit_syscall(int status);

void dummy_process_2(void) {
    print_to_screen("Dummy Process 2 is running.\n");
    // ... do whatever it needs to do ...
    exit_syscall(0);  // Once it’s done, call exit so it won't run again
}

