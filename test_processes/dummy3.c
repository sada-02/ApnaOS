// Dummy process 1 for my custom OS.
extern void print_to_screen(const char*);
extern void exit_syscall(int status);

void dummy_process_3(void) {
    print_to_screen("Dummy Process 3 is running.\n");
    exit_syscall(0);  
}

