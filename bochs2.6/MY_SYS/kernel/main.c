#include "list.h"
#include "stdio.h"
#include "lib/print.h"
#include "lib/debug.h"
#include "lib/string.h"
#include "include/global.h"
#include "include/interrupt.h"
#include "include/memory.h"
#include "include/thread.h"
#include "include/lock.h"
#include "include/syscall.h"
#include "ata.h"
#include "fs.h"
uint8_t a='a',b='e',c='i';
void f(void* f_arg);
void f2(void* f_arg);
void uf();
void uf2();
struct lock console_lock;
int main(void){
	put_str("Hi, I am kernel\n");

	init_pool();
	init_k_arena_cluster();
	init_interrupt(); //1.init_8259A
			 		  //2.init_idt
			 		  //3.lidt	
	init_syscall_table();

	update_gdt();
	
	make_main_thread();
	start_thread(f,"1",31);
	start_thread(f2,"2",31);

	create_process(uf,31);
	create_process(uf2,31);

	init_lock(&console_lock);
	
	enable_intr();

	init_fs();

	extern struct channel ata0_channel;
	uint8_t* p=(char*)malloc(512);
	ata_read(1,1,&(ata0_channel.disks[1]),p);
	for(int i=0;i<512;i++)
		printf("%x",p[i]);
	free(p);


	/*
	disable_intr();
	void* vaddr=malloc(9000);
	printf("vaddr: %x\n",vaddr);
	brk1();
	*(uint32_t*)vaddr=0x12345678;
	brk3();
	free(vaddr);
	brk2();
	*/
	/*
	disable_intr();
	void* vaddr=malloc(10);
	void* vaddr2=malloc(8);
	printf("vaddr: %x\n",vaddr);
	printf("vaddr2: %x\n",vaddr2);
	brk1();
	*(uint8_t*)vaddr=0x11;
	*(uint8_t*)vaddr2=0x22;
	brk3();
	free(vaddr);
	brk4();
	free(vaddr2);
	brk2();
	*/
	while (1);
	while(1){
		lock(&console_lock);
		//asm volatile("cli");
		//disable_intr();
		//put_str("m");
		//put_int(c);
		put_char(a);
		//enable_intr();
		//asm volatile("sti");
		unlock(&console_lock);
	}
	return 0;
}
void f(void* f_arg){
	while(1);
	while(1){
		lock(&console_lock);
		//asm volatile("cli");
		//disable_intr();
		//put_str(f_arg);
		//put_str("f");
		put_char(b);
		//enable_intr();
		//asm volatile("sti");
		unlock(&console_lock);

	}
}
void f2(void* f_arg){
	while(1);
	while(1){
		lock(&console_lock);
		//asm volatile("cli");
		//disable_intr();
		//put_str(f_arg);
		put_char(c);
		//enable_intr();
		//asm volatile("sti");
		unlock(&console_lock);
	}
}

void uf(){
	while(1);
	while(1){
		if(b=='h'){
			b='e';
		}
		else{
			b=b+1;
		}
	}
}
void uf2(){
	while(1);
	while(1){
		if(c=='n'){
			c='i';
		}
		else{
			c=c+1;
		}
	}
}