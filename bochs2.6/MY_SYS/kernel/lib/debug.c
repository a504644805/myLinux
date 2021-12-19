#include "debug.h"
void panic_spin(const char* filename,int line,const char* msg){
    disable_intr();
    put_str("ASSERT fail in:");put_str(filename);put_str(" at line ");put_int(line);put_str("\n");
    put_str(msg);put_str("\n");
    while(1);
}

void brk1(){
    return;   
}
void brk2(){
    return;   
}
void brk3(){
    return;   
}
void brk4(){
    return;   
}
void brk5(){
    return;   
}
void brk6(){
    return;   
}
void brk7(){
    return;   
}
void brk8(){
    return;   
}