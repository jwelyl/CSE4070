#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "devices/shutdown.h"
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "process.h"

static void syscall_handler (struct intr_frame *);
//void check_user_vaddr(const void* vaddr);

void
syscall_init (void) 
{
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
    /* Project 1~2 User Program */
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
    
    case SYS_READ:
      if(!is_user_vaddr(f->esp + 4) || !is_user_vaddr(f->esp + 8) || !is_user_vaddr(f->esp + 12))
        my_exit(-1);
      
      f->eax = my_read((int)*(uint32_t*)(f->esp + 4), (void*)*(uint32_t*)(f->esp + 8), 
              (unsigned)*((uint32_t*)(f->esp + 16)));
      break;
    
    case SYS_WRITE:
      if(!is_user_vaddr(f->esp + 4) || !is_user_vaddr(f->esp + 8) || !is_user_vaddr(f->esp + 12))
        my_exit(-1);

      f->eax = my_write((int)*(uint32_t*)(f->esp + 4), (void*)*(uint32_t*)(f->esp + 8), 
        (unsigned)*((uint32_t*)(f->esp + 12)));
      break;
  
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
  }

  //  thread_exit ();
}

void my_halt(void) {
  shutdown_power_off();
}

void my_exit(int status) {
  printf("%s: exit(%d)\n", thread_name(), status);
  thread_current() -> exit_status = status;
  thread_exit();
}

pid_t my_exec(const char* file) {
  return process_execute(file);
}

int my_wait(pid_t pid) {
  return process_wait(pid);
}

int my_read(int fd, void* buffer, unsigned size) {
  int i, ret = 0;

  //  STDIN
  if(fd == 0) {
    for(i = 0; i < size; i++) {
      if((((char*)buffer)[i] = input_getc()) == '\0') 
        break;
      ret += 1;
    }
  }
  else ret = -1;

  return ret;
}

int my_write(int fd, const void* buffer, unsigned size) {
  //  STDOUT
  if(fd == 1) {
    putbuf((const char*)buffer, size);
    return size;
  }
  else return -1;
}

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
