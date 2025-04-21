#ifndef SYSCALL_H
#define SYSCALL_H

#include <stdint.h>
#define SYS_FORK 1
#define SYS_WAIT 2
#define SYS_EXIT 3
#define SYS_YIELD 4
int fork_syscall(void);
int wait_syscall(int* status);
void exit_syscall(int status);
void yield_syscall(void);
void init_syscalls(void);

#endif