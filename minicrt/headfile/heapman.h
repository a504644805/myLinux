#ifndef _HEAP_MANAGEMENT
#define _HEAP_MANAGEMENT
void* brk(void* end);
void* malloc(int sz);
void free(void* FreeAddress);
int init_heap();

#endif