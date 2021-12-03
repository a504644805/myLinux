#include "headfile/minicrt.h"
int main(int argc,char* argv[]){
	int a=argc;
	char* s1=argv[0];
	char* s2=argv[2];

	int* p=(int*)malloc(sizeof(int));
	*p=3;
	free(p);

	asm volatile("xor %rax,%rax");
	asm volatile("xor %rbx,%rbx");
	asm volatile("xor %rcx,%rcx");

	printf("1w21%d%%%s",123,s1);
	FILE* file=fopen("/home/xy/t","w");
	fprintf(file,"1w21%d%%%s",123,s1);
	fclose(file);

	int c=strlen("1234");
	c=strlen("");

	char* buffer=(char*)malloc(100);
	char* s=itoa(1234,buffer,16);
	s=itoa(1234,buffer,10);
	free(buffer);


	//while(1);
    return 0;
} 
