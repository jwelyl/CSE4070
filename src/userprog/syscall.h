#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H
#include "../lib/user/syscall.h"

void syscall_init (void);

void my_halt(void);
void my_exit(int status);
pid_t my_exec(const char* file);
int my_wait(pid_t pid);
bool my_create(const char* file, unsigned initial_size);
bool my_remove(const char* file);
int my_open(const char* file);
int my_filesize(int fd);
int my_read(int fd, void* buffer, unsigned size);
int my_write(int fd, const void* buffer, unsigned size);
int my_fibonacci(int n);
int my_max_of_four_int(int num1, int num2, int num3, int num4);

#endif /* userprog/syscall.h */
