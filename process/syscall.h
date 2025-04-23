#ifndef SYSCALL_H
#define SYSCALL_H

#include <stdint.h>
int fork_syscall(void);
int wait_syscall(int* status);
void exit_syscall(int status);
void yield_syscall(void);
void init_syscalls(void);

#endif