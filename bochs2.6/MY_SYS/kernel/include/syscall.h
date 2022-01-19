#ifndef SYSCALL_H
#define SYSCALL_H
#include "global.h"
#include "thread.h"

#define SYSCALL_NUM 16
void init_syscall_table();

#define SYS_GETPID 0
#define SYS_WRITE 1
#define SYS_MALLOC 2
#define SYS_FREE 3
#define SYS_FORK 4
#define SYS_READ 5
#define SYS_PUTCHAR 6
#define SYS_CLEAR 7
#define SYS_PRINT_DIR 8
#define SYS_OPEN 9
#define SYS_CLOSE 10
#define SYS_EXEC 11
uint32_t sys_getpid();

//这一部分应该是用户的库而不是内核代码
#define _syscall0(num) ({\
    int retval;\
    asm volatile("int $0x80":"=a"(retval):"a"(num));\
    retval;\
})
#define _syscall1(num,arg1) ({\
    int retval;\
    asm volatile("int $0x80":"=a"(retval):"a"(num),"b"(arg1));\
    retval;\
})
#define _syscall2(num,arg1,arg2) ({\
    int retval;\
    asm volatile("int $0x80":"=a"(retval):"a"(num),"b"(arg1),"c"(arg2));\
    retval;\
})
#define _syscall3(num,arg1,arg2,arg3) ({\
    int retval;\
    asm volatile("int $0x80":"=a"(retval):"a"(num),"b"(arg1),"c"(arg2),"d"(arg3));\
    retval;\
})
int fork();
uint32_t getpid();
int write(int fd, const void *buf, size_t count);
int read(int fd, void *buf, size_t count);
void* malloc(size_t sz);
void free(void* vaddr);
void putchar(char c);
void clear();
void print_dir(char* path);
uint32_t open(const char *path, int flags);
void close(uint32_t fd);
void execv(const char *path, char *const argv[]);
#endif