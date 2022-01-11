#ifndef MY_IO_H
#define MY_IO_H
#include "global.h"

//AT&A
//in dx,al
static inline uint8_t inb(uint16_t dx){
	uint8_t data;
	asm volatile ("inb %w1,%b0":"=a"(data):"d"(dx));
	return data;
}
//out al,dx
static inline void outb(uint8_t al,uint16_t dx){
	asm volatile("outb %%al,%%dx"::"a"(al),"d"(dx));
}

//rep dx --2B--> es:edi, edi+=2
//cnt port			dst
static inline void rep_insw(uint16_t port,char* buf,size_t cnt){
	asm volatile("cld; rep insw":"+D"(buf),"+c"(cnt):"d"(port):"memory");
}

//rep ds:esi --2B--> dx, esi+=2
static inline void rep_outsw(uint16_t port,char* buf,size_t cnt){
	asm volatile("cld; rep outsw":"+S"(buf),"+c"(cnt):"d"(port):"memory");
}

#endif
