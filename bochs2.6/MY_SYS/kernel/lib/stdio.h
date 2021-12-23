#ifndef STDIO_H
#define STDIO_H
#include "string.h"
#include "syscall.h"

#define va_list char*
#define va_start(ap,last) (ap=(va_list)&last+sizeof(last))
#define va_arg(ap,type) (*(type*)((ap+=sizeof(type))-sizeof(type)))
#define va_end(ap) (ap=0)

int printf(const char *format, ...);
int vsprintf(char *str, const char *format, va_list ap);

#endif