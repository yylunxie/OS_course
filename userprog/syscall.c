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
#include "threads/synch.h"
struct lock filesys_lock;

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
  [SYS_CREATE] = sys_create,
  // [SYS_REMOVE] = sys_remove,
  [SYS_OPEN] = sys_open,
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
  lock_init(&filesys_lock);
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
  case SYS_EXIT:
    // printf("DEBUG: syscall_number = SYS_EXIT\n");
    sys_exit(f);
    break;
  case SYS_CREATE:
    sys_create(f);
    break;
  case SYS_OPEN:
    sys_open(f);
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
  cur->exit_status = status;
  printf("%s: exit(%d)\n", cur->name, status);
  thread_exit();
}

void sys_write(struct intr_frame* f){
  int fd = *(int *)(f->esp + 4);
  const char *buffer = *(char **)(f->esp + 8);
  unsigned size = *(unsigned *)(f->esp + 12);

  // printf("sys_write: fd=%d, buffer=%p, size=%u\n", fd, buffer, size);

  if (!is_user_vaddr(buffer) || !is_user_vaddr(buffer + size - 1)) {
    f->eax = -1;
    thread_exit();  // 如果訪問無效記憶體，應終止進程
    return;
  }

  if (fd == 1) { // stdout
    putbuf(buffer, size);
    f->eax = size;
  } 

  if (fd == 0) {
    f->eax = -1;
    return;
  }

  if (fd >= 2 && fd < MAX_FD) {
    struct thread *cur_thread = thread_current();
    struct file *file = cur_thread->fd_table[fd];

    if (file == NULL) {
      f->eax = -1;  // 如果找不到對應的檔案，返回錯誤
      return;
    }
    
    // 使用 file_write 函數寫入檔案
    int bytes_written = file_write(file, buffer, size);
    if (bytes_written < 0) {
      f->eax = -1;  // 寫入失敗，返回錯誤
    } else {
      f->eax = bytes_written;  // 返回成功寫入的字節數
    }
    return;
  }


  // printf("DEBUG: invalid fd %d\n", fd);
  f->eax = -1;
  return;
}

void sys_create(struct intr_frame *f) {
  const char *user_filename_ptr = *(char **)(f->esp + 4);
  unsigned initial_size = *(unsigned *)(f->esp + 8);

  // 1. 驗證 filename 的 user pointer 合法性
  if (user_filename_ptr == NULL || !is_user_vaddr(user_filename_ptr) ||
      pagedir_get_page(thread_current()->pagedir, user_filename_ptr) == NULL) {
    printf("ERROR: Invalid filename user pointer\n");
    f->eax = false;
    return;
  }

  // 2. 安全地從 user 空間複製 filename 字串到 kernel buffer
  char kernel_filename[128];
  size_t i;
  for (i = 0; i < sizeof(kernel_filename) - 1; i++) {
    if (!is_user_vaddr(user_filename_ptr + i) ||
        pagedir_get_page(thread_current()->pagedir, user_filename_ptr + i) == NULL) {
      printf("ERROR: Invalid memory when copying filename\n");
      f->eax = false;
      return;
    }
    kernel_filename[i] = user_filename_ptr[i];
    if (kernel_filename[i] == '\0') break;
  }
  kernel_filename[sizeof(kernel_filename) - 1] = '\0';  // 保險 null terminator
  if (i == sizeof(kernel_filename) - 1) {
    printf("ERROR: filename too long\n");
    f->eax = false;
    return;
  }

  // 3. Debug：顯示確認已成功取得 kernel 字串
  // printf("DEBUG: sys_create called with filename='%s', size=%u\n", kernel_filename, initial_size);

  // 4. 使用 lock 保護 filesys_create()
  lock_acquire(&filesys_lock);
  bool success = filesys_create(kernel_filename, initial_size);
  lock_release(&filesys_lock);

  // 5. Debug 成功與否
  // printf("DEBUG: filesys_create returned %d\n", success);

  f->eax = success;
}

void sys_open(struct intr_frame* f) {
  const char *user_filename = *(char **)(f->esp + 4);

  // 1. 驗證 user space 指標是否合法
  if (user_filename == NULL ||
      !is_user_vaddr(user_filename) ||
      pagedir_get_page(thread_current()->pagedir, user_filename) == NULL) {
    f->eax = -1;
    return;
  }

  // 2. 複製 filename 到 kernel buffer（最多複製 127 字元）
  char kernel_filename[128];
  size_t i;
  for (i = 0; i < sizeof(kernel_filename) - 1; i++) {
    if (!is_user_vaddr(user_filename + i) ||
        pagedir_get_page(thread_current()->pagedir, user_filename + i) == NULL) {
      f->eax = -1;
      return;
    }
    kernel_filename[i] = user_filename[i];
    if (kernel_filename[i] == '\0') break;
  }
  kernel_filename[sizeof(kernel_filename) - 1] = '\0';

  if (i == sizeof(kernel_filename) - 1) {
    f->eax = -1;
    return;
  }

  // 3. 開啟檔案
  lock_acquire(&filesys_lock);
  struct file *file = filesys_open(kernel_filename);
  lock_release(&filesys_lock);

  if (file == NULL) {
    f->eax = -1;
    return;
  }

  // 4. 找空的 fd slot，從 2 開始（0: stdin, 1: stdout）
  struct thread *t = thread_current();
  for (int fd = 2; fd < 128; fd++) {
    if (t->fd_table[fd] == NULL) {
      t->fd_table[fd] = file;
      f->eax = fd;
      return;
    }
  }

  // 5. 找不到 slot，關掉 file
  file_close(file);
  f->eax = -1;
}