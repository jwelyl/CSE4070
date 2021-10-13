#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

bool my_create(const char* file, unsigned initial_size);
bool my_remove(const char* file);
int my_open(const char* file);
int my_filesize(int fd);
void my_seek(int fd, unsigned position);
unsigned my_tell(int fd);
void my_close(int fd);

#endif /* userprog/syscall.h */
