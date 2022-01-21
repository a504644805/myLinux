#include "syscall.h"
#include "memory.h"
#include "fork_exec_wait_exit.h"
#include "fs.h"
#include "print.h"
#include "pipe.h"
void* syscall_table[SYSCALL_NUM];
void init_syscall_table(){
    syscall_table[SYS_GETPID]=sys_getpid;
    syscall_table[SYS_WRITE]=sys_write;
    syscall_table[SYS_MALLOC]=sys_malloc;
    syscall_table[SYS_FREE]=sys_free;
    syscall_table[SYS_FORK]=sys_fork;
    syscall_table[SYS_READ]=sys_read;
    syscall_table[SYS_PUTCHAR]=put_char;
    syscall_table[SYS_CLEAR]=cls_screen;
    syscall_table[SYS_PRINT_DIR]=sys_print_dir;
    syscall_table[SYS_OPEN]=sys_open;
    syscall_table[SYS_CLOSE]=sys_close;
    syscall_table[SYS_EXEC]=sys_execv;
    syscall_table[SYS_WAIT]=sys_wait;
    syscall_table[SYS_EXIT]=sys_exit;
    syscall_table[SYS_PIPE]=sys_pipe;
}

uint32_t sys_getpid(){
    return get_cur_running()->pid;
}

//这一部分应该是用户的库而不是内核代码
int fork(){
    return _syscall0(SYS_FORK);
}

uint32_t getpid(){
    return _syscall0(SYS_GETPID);
}

int write(int fd, const void *buf, size_t count){
    return _syscall3(SYS_WRITE,fd,buf,count);
}

int read(int fd, void *buf, size_t count){
    return _syscall3(SYS_READ,fd,buf,count);
}

void* malloc(size_t sz){
    return (void*)_syscall1(SYS_MALLOC,sz);
}

void free(void* vaddr){
    _syscall1(SYS_FREE,vaddr);
}

void putchar(char c){
    _syscall1(SYS_PUTCHAR,c);
}

void clear(){
    _syscall0(SYS_CLEAR);
}

void print_dir(char* path){
    _syscall1(SYS_PRINT_DIR,path);
}

uint32_t open(const char *path, int flags){
    return _syscall2(SYS_OPEN,path,flags);
}

void close(uint32_t fd){
    _syscall1(SYS_CLOSE,fd);
}

void execv(const char *path, char *const argv[]){
    _syscall2(SYS_EXEC,path,argv);
}

void exit(int status){
    _syscall1(SYS_EXIT,status);
}

int wait(int *wstatus){
    return _syscall1(SYS_WAIT,wstatus);
}

void pipe(int pipefd[2]){
    _syscall1(SYS_PIPE,pipefd);
}
