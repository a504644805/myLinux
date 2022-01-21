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
#include "shell.h"
uint8_t a='a',b='e',c='i';
void f(void* f_arg);
void f2(void* f_arg);
void uf();
void uf2();
void idle(void* f_arg);
void init();
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
	start_thread(idle,NULL,31);
	/*
	start_thread(f,"1",31);
	start_thread(f2,"2",31);

	create_process(uf,31);
	create_process(uf2,31);
	*/
	
	init_kbd();
	init_lock(&console_lock);
	
	enable_intr();

	init_fs();

	extern struct channel ata0_channel;
	uint32_t user_prog_file_size=4996;
	uint32_t user_prog_file_sect_cnt=DIVUP(user_prog_file_size,SECT_SIZE);
	ASSERT(user_prog_file_size<=6*KB);
	void* user_prog_file_buf=malloc(user_prog_file_sect_cnt*SECT_SIZE);
	ASSERT(user_prog_file_buf!=NULL);
	ata_read(user_prog_file_sect_cnt,300,&(ata0_channel.disks[0]),user_prog_file_buf);
	ASSERT(memcmp(user_prog_file_buf,"\177ELF\1\1\1",7)==0);
	uint32_t u_file_fd=sys_open("/user_prog",O_CREAT|O_WRONLY);//write into fs
	sys_write(u_file_fd,user_prog_file_buf,user_prog_file_size);
	sys_close(u_file_fd);
	free(user_prog_file_buf);

	create_process(init,31);
	
	while(1);

	/*
	//fs test
	extern struct partition* default_parti;
	struct dir_entry dir_entry={"normal1",1,NORMAL};
	char str_buf[MAX_PATH_LEN];

	brk6();
	sys_print_dir("/");
	brk6();
	sys_mkdir("/dir1");
	sys_print_dir("/");
	print_bm_in_parti(default_parti,INODE_BITMAP,16);
	print_bm_in_parti(default_parti,DATA_BITMAP,16);
	brk6();
	sys_mkdir("/dir1/dir2");
	brk6();
	printf("dir1 content:\n");sys_print_dir("/dir1");
	printf("dir2 content:\n");sys_print_dir("/dir1/dir2");
	print_bm_in_parti(default_parti,INODE_BITMAP,16);
	print_bm_in_parti(default_parti,DATA_BITMAP,16);
	brk6();

	sys_open("/dir1/fil1",O_CREAT|O_RDWR);
	printf("dir1 content:\n");sys_print_dir("/dir1");
	printf("/ content:\n");sys_print_dir("/");
	brk6();
	sys_rmdir("/dir1/dir2");
	printf("dir1 content:\n");sys_print_dir("/dir1");
	brk6();
	sys_getcwd(str_buf,MAX_PATH_LEN);
	printf("cwd: %s\n",str_buf);
	brk6();
	sys_chdir("/dir1");
	sys_getcwd(str_buf,MAX_PATH_LEN);
	printf("cwd: %s\n",str_buf);
	brk6();

	brk5();
	uint32_t fd1=sys_open("/file1",O_CREAT|O_RDWR);
	printf("fd1: %d\n",fd1);
	print_inode_list(default_parti);
	brk5();
	sys_write(fd1,"aabbccdd",8);
	print_inode_list(default_parti);
	brk5();
	sys_close(fd1);
	print_inode_list(default_parti);
	sys_unlink("/file1");
	print_inode_list(default_parti);
	brk5();

	brk5();
	uint32_t fd2=sys_open("/file1",O_CREAT|O_RDWR);
	printf("fd2: %d\n",fd2);
	print_inode_list(default_parti);
	brk5();	
	sys_close(fd2);
	print_inode_list(default_parti);
	brk5();
	*/

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
void idle(void* f_arg){
	while(1);
}
void init(){
	int i=fork();

	if(i==-1){
		printf("fail to fork\n");
	}
	else if(i==0){
		shell();
	}
	else{
		printf("Boot done\n");
		while(1){
			int child_status;
			int child_pid=wait(&child_status);
			if(child_pid==-1){
				continue;
			}
			else{
				printf("INIT PROCESS: my exit child process's pid: %d, exit_status: %d\n",child_pid,child_status);
			}
		}
	}
	while(1);


	/*
	test pipe(put code below under " void init{ "
	int pipefd[2];
	pipe(pipefd);
	printf("finish pipe, pipefd[0] is %d, pipefd[1] is %d\n",pipefd[0],pipefd[1]);brk1();
	int r=fork();
	if(r==-1){
		printf("fail to fork\n");
	}
	else if(r==0){
		close(pipefd[0]);
		write(pipefd[1],"Hi I am your father",19);
		while(1);
	}
	else{
		char buf[32]={0};
		close(pipefd[1]);
		read(pipefd[0],buf,19);
		printf("receive \"%s\"\n",buf);
		while(1);
	}
	*/
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