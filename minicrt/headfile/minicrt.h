#ifndef _MINICRT_HEADER
#define _MINICRT_HEADER

#define va_list char*
#define va_start(ap,last) (ap=(va_list)&last+sizeof(last))
#define va_arg(ap,type) (*(type*)((ap+=sizeof(type))-sizeof(type)))
#define va_end(ap) (ap=0)

#define NULL 0

#define asmlinkage __attribute__((regparm(0)))

#include "list.h"
#include "heapman.h"
#include "str.h"
#include "ioman.h"

#endif