#ifndef DEBUG_H
#define DEBUG_H
#include "print.h"
#include "interrupt.h"
#define ASSERT(condition) \
if(condition){} else{panic_spin(__FILE__,__LINE__,#condition);}

void panic_spin(const char* filename,int line,const char* msg);

#endif