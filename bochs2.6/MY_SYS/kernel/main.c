#include "lib/print.h"
#include "lib/debug.h"
#include "lib/string.h"
#include "include/global.h"
#include "include/interrupt.h"
#include "include/memory.h"
#include "include/thread.h"
#include "include/lock.h"
void f(void* f_arg);
void f2(void* f_arg);
struct lock console_lock;
int main(void){
	put_str("Hi, I am kernel\n");

	init_pool();
	init_interrupt(); //1.init_8259A
			 		  //2.init_idt
			 		  //3.lidt	

	make_main_thread();
	start_thread(f,"1",31);
	start_thread(f2,"2",31);

	init_lock(&console_lock);
	enable_intr();

	while(1){
		lock(&console_lock);
		//asm volatile("cli");
		//disable_intr();
		put_str("m");
		//enable_intr();
		//asm volatile("sti");
		unlock(&console_lock);
	}
	return 0;
}
void f(void* f_arg){
	while(1){
		lock(&console_lock);
		//asm volatile("cli");
		//disable_intr();
		put_str(f_arg);
		//enable_intr();
		//asm volatile("sti");
		unlock(&console_lock);

	}
}
void f2(void* f_arg){
	while(1){
		lock(&console_lock);
		//asm volatile("cli");
		//disable_intr();
		put_str(f_arg);
		//enable_intr();
		//asm volatile("sti");
		unlock(&console_lock);
	}
}

