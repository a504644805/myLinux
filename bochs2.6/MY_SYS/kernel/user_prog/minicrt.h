#ifndef USER_PROG_H
#define USER_PROG_H

typedef unsigned int uint32_t;
typedef uint32_t size_t;

/*
^   args
|   argv
|   argc
|   旧ebp
|        <---ebp
*/
void _start();

#define SYS_GETPID 0
#define SYS_WRITE 1
#define SYS_EXIT 13

uint32_t getpid();
int write(int fd, const void *buf, size_t count);
void exit(int status);

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

#endif