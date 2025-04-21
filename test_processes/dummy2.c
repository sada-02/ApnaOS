
extern void print_to_screen(const char*);
extern void exit_syscall(int status);

void dummy_process_2(void) {
    print_to_screen("Dummy Process 2 is running.\n");
    exit_syscall(0);  
}

