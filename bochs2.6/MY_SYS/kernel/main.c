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

	//fs test
	extern struct dir root_dir;
	extern struct partition* default_parti;
	struct dir_entry dir_entry={"normal1",1,NORMAL};

	brk5();
	uint32_t fd1=sys_open("/file1",O_CREAT|O_RDWR);
	printf("fd1: %d\n",fd1);
	print_inode_list(default_parti);
	brk5();
	uint32_t fd2=sys_open("/file1",O_CREAT|O_RDWR);
	printf("fd2: %d\n",fd2);
	print_inode_list(default_parti);
	brk5();	
	sys_close(fd1);
	print_inode_list(default_parti);
	brk5();
	sys_close(fd2);
	print_inode_list(default_parti);
	brk5();

	brk4();
	print_inode_list(default_parti);
	brk4();
	print_bm_in_parti(default_parti,INODE_BITMAP,64);
	brk4();
	print_bm_in_parti(default_parti,DATA_BITMAP,64);
	brk4();
	print_inode_in_disk(default_parti,7);
	brk4();
	print_dir(root_dir,default_parti,5);
	brk4();

	brk3();
	dir_add_dir_entry_and_sync(&root_dir,&dir_entry,default_parti);
	brk3();
	search_file_in_dir("normal1",&root_dir,default_parti,&dir_entry);
	brk3();
	print_dir_entry(dir_entry);
	brk3();


	brk2();
	if(search_file_with_path("/normal1",&dir_entry)){
		printf("success.");print_dir_entry(dir_entry);
	}
	else{
		printf("fail.");print_dir_entry(dir_entry);
	}
	brk2();

	if(search_file_with_path("/normal2",&dir_entry)){
		printf("success.");print_dir_entry(dir_entry);
	}
	else{
		printf("fail.");print_dir_entry(dir_entry);
	}
	brk2();

	brk1();
	search_file_in_dir("aaa",&root_dir,default_parti,&dir_entry);
	brk1();
	print_dir_entry(dir_entry);
	brk1();

	while (1);

	/*
	//硬盘驱动测试
	extern struct channel ata0_channel;
	uint8_t* p=(char*)malloc(512);
	ata_read(1,1,&(ata0_channel.disks[1]),p);
	for(int i=0;i<512;i++)
		printf("%x",p[i]);
	free(p);
	*/

	/*
	//内存管理测试
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

	/*
	//多线程测试
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
	*/
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