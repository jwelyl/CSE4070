#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

static void syscall_handler (struct intr_frame *);

        void
syscall_init (void) 
{
        intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

        static void
syscall_handler (struct intr_frame *f) 
{
        uint32_t sys_num = *(uint32_t*)(f->esp);
        //  printf("에헴(syscall.c) sys_num : %d\n", sys_num);
        //  printf("에헴(syscall.c) esp : %p\n\n", f->esp);

        //  printf("hex_dump in syscall_handler\n");
        //  hex_dump(f->esp, f->esp, 100, 1);
        //  printf("\n\n");

  switch(sys_num) {
    /* Project 1~2 User Program */
    case SYS_HALT:
//      printf("SYS_HALT\n");
      break;
    case SYS_EXIT:
      //printf("SYS_EXIT\n");
      exit(*(uint32_t*)(f->esp + 4));
      break;
    case SYS_EXEC:
//      printf("SYS_EXEC\n");
      break;
    case SYS_WAIT:
//      printf("SYS_WAIT\n");
      break;
    case SYS_CREATE:
//      printf("SYS_CREATE\n");
      break;
    case SYS_REMOVE:
//      printf("SYS_REMOVE\n");
      break;
    case SYS_OPEN:
//      printf("SYS_OPEN\n");
      break;
    case SYS_FILESIZE:
//      printf("SYS_FILESIZE\n");
      break;
    case SYS_READ:
//      printf("SYS_READ\n");
      break;
    case SYS_WRITE:
      //printf("SYS_WRITE\n");
      write((int)*(uint32_t*)(f->esp + 4), (void*)*(uint32_t*)(f->esp + 8), 
        (unsigned)*((uint32_t*)(f->esp + 12)));
      break;
    case SYS_SEEK:
//      printf("SYS_SEEK\n");
      break;
    case SYS_TELL:
//      printf("SYS_TELL");
      break;
    case SYS_CLOSE:
//      printf("SYS_CLOSE");
      break;
  }

  //  thread_exit ();
}

void halt(void) {
  shutdown_power_off();
}

void exit(int status) {
  printf("%s: exit(%d)\n", thread_name(), status);
  thread_exit();
}

pid_t exec(const char* file) {
  return process_execute(file);
}

int wait(pid_t pid) {
  return process_wait(pid);
}

/*
bool create(const char* file, unsigned initial_size) {
  return false;
}

bool remove(const char* file) {
  return false;
}

int open(const char* file) {
  return 0;
}

int filesize(int fd) {
  return 0;
}
*/

int read(int fd, void* buffer, unsigned size) {
  int i;
  if(fd == 0) {
    for(i = 0; i < size; i++) {
      if(((char*)buffer)[i] == '\0') {
        break;
      }
    }
  }

  return 0;
}

int write(int fd, const void* buffer, unsigned size) {
  //  STDOUT
  //printf("출력 위치 : %p\n", buffer);

  if(fd == 1) {
    putbuf(buffer, size);
    return size;
  }
  else return -1;
}
