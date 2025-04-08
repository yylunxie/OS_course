#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

#include <devices/shutdown.h>

#include <string.h>
#include <filesys/file.h>
#include <devices/input.h>
#include <threads/malloc.h>
#include <threads/palloc.h>
#include "process.h"
#include "pagedir.h"
#include <threads/vaddr.h>
#include <filesys/filesys.h>

#define MAX_SYSCALL 20

// lab01 Hint - Here are the system calls you need to implement.

/* System call for process. */

void sys_halt(void);
void sys_exit(struct intr_frame* f);
void sys_exec(struct intr_frame* f);
void sys_wait(struct intr_frame* f);

/* System call for file. */
void sys_create(struct intr_frame* f);
void sys_remove(struct intr_frame* f);
void sys_open(struct intr_frame* f);
void sys_filesize(struct intr_frame* f);
void sys_read(struct intr_frame* f);
void sys_write(struct intr_frame* f);
void sys_seek(struct intr_frame* f);
void sys_tell(struct intr_frame* f);
void sys_close(struct intr_frame* f);


static void (*syscalls[MAX_SYSCALL])(struct intr_frame *) = {
  [SYS_HALT] = sys_halt,
  [SYS_EXIT] = sys_exit,
  // [SYS_EXEC] = sys_exec,
  // [SYS_WAIT] = sys_wait,
  // [SYS_CREATE] = sys_create,
  // [SYS_REMOVE] = sys_remove,
  // [SYS_OPEN] = sys_open,
  // [SYS_FILESIZE] = sys_filesize,
  // [SYS_READ] = sys_read,
  [SYS_WRITE] = sys_write,
  // [SYS_SEEK] = sys_seek,
  // [SYS_TELL] = sys_tell,
  // [SYS_CLOSE] = sys_close
};

static void syscall_handler (struct intr_frame *);

void syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}


/* System Call: void halt (void)
    Terminates Pintos by calling shutdown_power_off() (declared in devices/shutdown.h). 
*/
void sys_halt(void)
{
  shutdown_power_off();
}

static void syscall_handler (struct intr_frame *f UNUSED) 
{
  // printf ("system call!\n");
  int syscall_num = *(int *) f->esp;

  switch (syscall_num)
  {
  case SYS_WRITE:
    // printf("DEBUG: syscall_number = %d\n", syscall_num);
    sys_write(f);
    break;
  
  default:
    // printf("DEBUG: Unknown system call: %d\n", syscall_num);
    thread_exit();
  }
}

void sys_exit(struct intr_frame* f)
{
  struct thread *cur = thread_current();

  int status = *(int *)(f->esp + 4);
  cur->exit_status = status;  // 設定 exit status
  printf("%s: exit(%d)\n", cur->name, status);
  thread_exit();  // 呼叫 thread_exit() 讓 process 退出
}

struct write_args {
  int fd;
  const void *buffer;
  unsigned size;
};

void sys_write(struct intr_frame* f){
  int fd = *((int *) f->esp + 1);
  const void *buffer = (void *) *((int *) f->esp + 2);
  unsigned size = *((unsigned *) f->esp + 3);
  // printf("DEBUG: sys_write called with fd=%d, buffer=%p, size=%u\n", 
    // fd, buffer, size);

  if (!is_user_vaddr(buffer)) {
    f->eax = -1;
    return;
  }

  if (fd == 1){
    putbuf(buffer, size);  // 輸出到 Pintos console
    f->eax = size;         // 回傳成功寫入的位元組數量
    return;
  }

  // printf("DEBUG: invalid fd %d\n", fd);
  f->eax = -1;
  return;
}