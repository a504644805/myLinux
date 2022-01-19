#include "minicrt.h"
uint32_t getpid(){
    return _syscall0(SYS_GETPID);
}
int write(int fd, const void *buf, size_t count){
    return _syscall3(SYS_WRITE,fd,buf,count);
}

int main(int argc,char** argv);
void _start(){
    uint32_t ebp;
    asm volatile("mov %%ebp,%%ebx":"=b"(ebp));
    int argc=*((int*)(ebp+4));
    char** argv=(char**)(ebp+8);
    main(argc,argv);

    while(1);//exit
}

