#include<stdio.h>

int main(){
        register int i asm("rdx")=2;
   
        i=i+1;
	asm("movl $1,%edx");
	return 0;  	 
}

