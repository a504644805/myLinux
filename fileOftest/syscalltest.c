#include<unistd.h>
#include<stdlib.h>
#include<stdio.h>
#include <sys/types.h>
#include <sys/wait.h>


int main(){
	long l=syscall(333);
	return 0;

}

