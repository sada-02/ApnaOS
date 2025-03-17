// Dummy process 1 for your custom OS.
// Uses the OS-specific print function for output.
extern void print_to_screen(const char*);

void dummy_process_1(void) {
    print_to_screen("Dummy Process 1 is running.\n");
}
