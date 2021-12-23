#ifndef SYSCALL_H
#define SYSCALL_H
#include "global.h"
#include "thread.h"

#define SYSCALL_NUM 10
void init_syscall_table();

#define SYS_GETPID 0
#define SYS_WRITE 1
#define SYS_MALLOC 2
#define SYS_FREE 3
uint32_t sys_getpid();
//On success, the number of bytes written  is  returned
uint32_t sys_write(const char* s);


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
uint32_t getpid();
uint32_t write(const char* s);
void* malloc(size_t sz);
void free(void* vaddr);
#endif