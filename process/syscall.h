#ifndef SYSCALL_H
#define SYSCALL_H

#include <stdint.h>

// System call numbers
#define SYS_FORK 1
#define SYS_WAIT 2
#define SYS_EXIT 3
#define SYS_YIELD 4

// System call function prototypes
int fork_syscall(void);
int wait_syscall(int* status);
void exit_syscall(int status);
void yield_syscall(void);

// Initialize system call handling
void init_syscalls(void);

#endif // SYSCALL_H
