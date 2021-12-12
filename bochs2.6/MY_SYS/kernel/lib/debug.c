#include "debug.h"
void panic_spin(const char* filename,int line,const char* msg){
    disable_intr();
    put_str("ASSERT fail in:");put_str(filename);put_str(" at line ");put_int(line);put_str("\n");
    put_str(msg);put_str("\n");
    while(1);
}