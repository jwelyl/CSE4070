#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H
#include "../lib/user/syscall.h"
#include "filesys/off_t.h"

struct file
{
  struct inode* inode;
  off_t pos;
  bool deny_write;
};

void syscall_init (void);

/* Proj1 */
void my_halt(void);
void my_exit(int status);
pid_t my_exec(const char* file);
int my_wait(pid_t pid);

/* Proj1(STDIN, STDOUT), Proj2(FILE) */
int my_read(int fd, void* buffer, unsigned size);
int my_write(int fd, const void* buffer, unsigned size);

/* Proj1 additional implementation */
int my_fibonacci(int n);
int my_max_of_four_int(int num1, int num2, int num3, int num4);

/* Proj2 */
bool my_create(const char* file, unsigned initial_size);
bool my_remove(const char* file);
int my_open(const char* file);
int my_filesize(int fd);
void my_seek(int fd, unsigned position);
unsigned my_tell(int fd);
void my_close(int fd);

#endif /* userprog/syscall.h */
