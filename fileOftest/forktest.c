#include<unistd.h>
#include<stdlib.h>
#include<stdio.h>
#include <sys/types.h>
#include <sys/wait.h>


int main(){
	int i=fork();
	int c_status=0;
	if(i==-1){
		printf("fork fail\n");
	}
	else if(i==0){
		printf("I am child\n");
	//	return 1;
		exit(2);
	}
	else{
		int c_pid=wait(&c_status);
		printf("fa: child %d exit,exit_status is %d\n",c_pid,c_status);
		return 0;
	}

}

