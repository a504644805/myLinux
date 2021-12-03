#include "headfile/minicrt.h"
extern int main(int argc,char* argv[]);
void exit(int status){
	//__exit_funcs

	asm volatile("mov $1,%%eax;int $0x80"::"b"(status));
}
void entry(){
	//prepare stack environment for main
	char* rbp_val=0;
	asm volatile("mov %%rbp,%%rax":"=a"(rbp_val));
	int argc=*((int*)(rbp_val+8));
	char** argv=(char**)(rbp_val+16);
	//init heap and I/O
	if(init_heap()==-1){
		return;
	}
	if(init_io()==-1){
		return;
	}

	int status=main(argc,argv);

	//__exit_funcs and 0x80 exit
	exit(status);
	asm("hlt");
}
