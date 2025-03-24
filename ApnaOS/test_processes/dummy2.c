// Dummy process 2 for your custom OS.
extern void print_to_screen(const char*);

void dummy_process_2(void) {
    print_to_screen("Dummy Process 2 is running.\n");
}
