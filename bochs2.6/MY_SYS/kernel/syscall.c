#include "syscall.h"
#include "memory.h"
void* syscall_table[SYSCALL_NUM];
void init_syscall_table(){
    syscall_table[SYS_GETPID]=sys_getpid;
    syscall_table[SYS_WRITE]=sys_write;
    syscall_table[SYS_MALLOC]=sys_malloc;
    syscall_table[SYS_FREE]=sys_free;
}

uint32_t sys_getpid(){
    return get_cur_running()->pid;
}

uint32_t sys_write(const char* s){
    put_str(s);
    return strlen(s);
}

//这一部分应该是用户的库而不是内核代码
uint32_t getpid(){
    return _syscall0(SYS_GETPID);
}

uint32_t write(const char* s){
    return _syscall1(SYS_WRITE,s);
}

void* malloc(size_t sz){
    return (void*)_syscall1(SYS_MALLOC,sz);
}

void free(void* vaddr){
    _syscall1(SYS_FREE,vaddr);
}