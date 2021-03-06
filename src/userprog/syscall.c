#include "userprog/syscall.h"
#include <stdio.h>
#include <string.h>
#include <syscall-nr.h>
#include "devices/shutdown.h"
#include "devices/input.h"
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "process.h"
#include "threads/synch.h"
#include "filesys/file.h"
#include "filesys/filesys.h"

struct thread* t;
struct file* fp;
struct lock filesys_lock;

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  lock_init(&filesys_lock);
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f) 
{
  uint32_t sys_num = *(uint32_t*)(f->esp);
  /*
  printf("hex_dump in syscall_handler\n");
  hex_dump(f->esp, f->esp, 320, 1);
  printf("\n\n");
*/
  switch(sys_num) {
    /* Project 1 User Program */
    case SYS_HALT:
      my_halt();
    
      break;
    case SYS_EXIT:
      if(!is_user_vaddr(f->esp + 4))
        my_exit(-1);
      
      my_exit(*(uint32_t*)(f->esp + 4));
      break;
    
    case SYS_EXEC:
      if(!is_user_vaddr(f->esp + 4))
        my_exit(-1);

      f->eax = my_exec((const char*)*(uint32_t*)(f->esp + 4));
      break;
    
    case SYS_WAIT:
      f->eax = my_wait((pid_t)*(uint32_t*)(f->esp + 4));
      break;

    /* Proj1 STDIN, Proj2 FILE INPUT */
    case SYS_READ:
      if(!is_user_vaddr(f->esp + 4) || !is_user_vaddr(f->esp + 8) || !is_user_vaddr(f->esp + 12))
        my_exit(-1);
      
      //printf("SYS_READ 주소 검사 완료 %p\n", f->esp + 8);
      //hex_dump(f->esp + 8, f->esp + 8, 320, 1);
      f->eax = my_read((int)*(uint32_t*)(f->esp + 4), (void*)*(uint32_t*)(f->esp + 8), 
              (unsigned)*((uint32_t*)(f->esp + 12)));
      break;
    
    /* Proj1 STDOUT, Proj2 FILE OUTPUT */
    case SYS_WRITE:
      if(!is_user_vaddr(f->esp + 4) || !is_user_vaddr(f->esp + 8) || !is_user_vaddr(f->esp + 12))
        my_exit(-1);

      f->eax = my_write((int)*(uint32_t*)(f->esp + 4), (void*)*(uint32_t*)(f->esp + 8), 
        (unsigned)*((uint32_t*)(f->esp + 12)));
      break;

    /* Proj1 User Program additional implementation */
    case SYS_FIBO:
      if(!is_user_vaddr(f->esp + 4))
        my_exit(-1);

      f->eax = my_fibonacci((int)*(uint32_t*)(f->esp + 4));
      break;

    case SYS_MAX4:
      if(!is_user_vaddr(f->esp + 4) || !is_user_vaddr(f->esp + 8) ||
         !is_user_vaddr(f->esp + 12) || !is_user_vaddr(f->esp + 16))
        my_exit(-1);

      f->eax = my_max_of_four_int((int)*(uint32_t*)(f->esp + 4), (int)*(uint32_t*)(f->esp + 8), 
                      (int)*(uint32_t*)(f->esp + 12), (int)*(uint32_t*)(f->esp + 16)); 
      break;   
    
    /* Proj2 User Program */
    case SYS_CREATE:
      if(!is_user_vaddr(f->esp + 4) || !is_user_vaddr(f->esp + 8))
        my_exit(-1);
      f->eax = my_create((const char*)*(uint32_t*)(f->esp + 4), (unsigned)*(uint32_t*)(f->esp + 8));
      break;

    case SYS_REMOVE:
      if(!is_user_vaddr(f->esp + 4))
        my_exit(-1);
      f->eax = my_remove((const char*)*(uint32_t*)(f->esp + 4));
      break;

    case SYS_OPEN:
      if(!is_user_vaddr(f->esp + 4))
        my_exit(-1);
      f->eax = my_open((const char*)*(uint32_t*)(f->esp + 4));
      break;

    case SYS_FILESIZE:
      if(!is_user_vaddr(f->esp + 4))
        my_exit(-1);
      f->eax = my_filesize((int)*(uint32_t*)(f->esp + 4));
      break;

    case SYS_SEEK:
      if(!is_user_vaddr(f->esp + 4) || !is_user_vaddr(f->esp + 8))
        my_exit(-1);
      my_seek((int)*(uint32_t*)(f->esp + 4), (unsigned)*(uint32_t*)(f->esp + 8));
      break;

    case SYS_TELL:
      if(!is_user_vaddr(f->esp + 4))
        my_exit(-1);
      f->eax = my_tell((int)*(uint32_t*)(f->esp + 4));
      break;

    case SYS_CLOSE:
      if(!is_user_vaddr(f->esp + 4))
        my_exit(-1);
      my_close((int)*(uint32_t*)(f->esp + 4));
      break;

  }

  //  thread_exit ();
}

/* Proj1 */
void my_halt(void) {
  shutdown_power_off();
}

void my_exit(int status) {
  int i;
  t = thread_current();
  printf("%s: exit(%d)\n", thread_name(), status);
  t->exit_status = status;

  for(i = 3; i < 128; i++) {
    fp = t->fd[i];
    if(fp) file_close(fp); 
  }
  thread_exit();
}

pid_t my_exec(const char* file) {
  return process_execute(file);
}

int my_wait(pid_t pid) {
  return process_wait(pid);
}

/* Proj1(STDIN), Proj2(FILE INPUT) */
int my_read(int fd, void* buffer, unsigned size) {
  int ret = 0;
  //t = thread_current();

  lock_acquire(&filesys_lock);
  t = thread_current();
  //printf("my_read 주소 %p\n", buffer);
  //hex_dump(buffer, buffer, 320, 1);
  if(!is_user_vaddr(buffer))
    my_exit(-1);

  //  Proj1 STDIN
  if(fd == 0) {
    for(; ret < (int)size; ret++) 
      if(input_getc() == '\0') break;
  }
  //  Proj2 FILE INPUT
  else if(fd >= 3) {
    if(!t->fd[fd]) {
      lock_release(&filesys_lock);      
      my_exit(-1);
    }
    ret = file_read(t->fd[fd], buffer, size);
  }
  else ret = -1;
  lock_release(&filesys_lock);

  return ret;
}

/* Proj2(STDOUT), Proj2(FILE OUPTPUT) */
int my_write(int fd, const void* buffer, unsigned size) {
  int ret = 0;
  
  //t = thread_current();
  lock_acquire(&filesys_lock);
  t = thread_current();
  //  Proj1 STDOUT
  if(fd == 1) {
    putbuf((const char*)buffer, size);
    ret = size;
  }
  //  Proj2 FILE OUTPUT
  else if(fd >= 3) {
    if(!t->fd[fd]) {
      lock_release(&filesys_lock);
      my_exit(-1);
    }

    if(t->fd[fd]->deny_write) file_deny_write(t->fd[fd]);
    ret = file_write(t->fd[fd], buffer, size);
  }
  else ret = -1;
  lock_release(&filesys_lock);

  return ret;
}

/* Proj1 additional implementation */
int my_fibonacci(int n) {
  int f1 = 1, f2 = 1;
  int i, ret;

  if(n == 0) ret = 0;
  else if(n < 3) ret = 1;
  else {
    for(i = 3; i <= n; i++) {
      ret = f1 + f2;
      f1 = f2;
      f2 = ret;
    }
  }

  return ret;
}

int my_max_of_four_int(int num1, int num2, int num3, int num4) {
  int ret = num1;

  if(ret < num2) ret = num2;
  if(ret < num3) ret = num3;
  if(ret < num4) ret = num4;

  return ret; 
}

/* Proj2 */
bool my_create(const char* file, unsigned initial_size) {
  if(!file) my_exit(-1);

  return filesys_create(file, initial_size);
}

bool my_remove(const char* file) {
  if(!file) my_exit(-1);
  return filesys_remove(file);
}

int my_open(const char* file) {
  int ret;
  //t = thread_current();
  
  if(!file) my_exit(-1);

  lock_acquire(&filesys_lock);
  t = thread_current();
  fp = filesys_open(file);

  if(!fp) 
    ret = -1;

  //  0 : STDIN, 1 : STDOUT, 2 : STDERR, 3 ~ 127 : FILE
  else {
    //printf("thread 이름 : %s\n", thread_name());
          
    for(ret = 3; ret < 128; ret++) {
      if(!t->fd[ret]) {
        if(!strcmp(t->name, file))
          file_deny_write(fp); 
        t->fd[ret] = fp;
        break;
      }
    }
  }
  lock_release(&filesys_lock);

  return ret; 
}

int my_filesize(int fd) {
  t = thread_current();
  if(!t->fd[fd]) my_exit(-1);

  return file_length(t->fd[fd]);
}

void my_seek(int fd, unsigned position) {
  t = thread_current();
  if(!t->fd[fd]) my_exit(-1);

  file_seek(t->fd[fd], position);
}

unsigned my_tell(int fd) {
  t = thread_current();
  if(!t->fd[fd]) my_exit(-1);

  return file_tell(t->fd[fd]);
}

void my_close(int fd) {
  t = thread_current();

  if(!t->fd[fd]) my_exit(-1);

  file_close(t->fd[fd]);
  t->fd[fd] = NULL; 
}
