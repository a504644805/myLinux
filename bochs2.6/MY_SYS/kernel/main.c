#include "lib/print.h"
#include "lib/debug.h"
#include "lib/string.h"
#include "include/global.h"
#include "include/interrupt.h"
#include "include/memory.h"

int main(void){
	put_str("Hi, I am kernel\n");

	void* p=malloc_page(K,3);
	memset(p,0,3*PG_SIZE);

	init_interrupt(); //1.init_8259A
			 		  //2.init_idt
			 		  //3.lidt	
	asm volatile("sti");
	while(1){
	}
	return 0;
}
