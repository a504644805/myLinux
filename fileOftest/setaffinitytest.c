#define _GNU_SOURCE
#include<stdio.h>
#include<unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sched.h>

//gcc -g -o test test.c -lpthread

struct args_t{
	int a;
	int b;
};

struct ret_t{
	int a;
};

void* f(void* args){
	int i=1;
	cpu_set_t mask;
	CPU_ZERO(&mask);
	CPU_SET(i, &mask);
	
	if (pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask) < 0) {
            printf("set thread affinity failed\n");
        }
	
	while(1){
		sleep(1);
		printf("c");
	}
}


int main(){
	pthread_t pt;
//	cout<<unitbuf;
	setbuf(stdout,NULL);
	int ret=pthread_create(&pt, NULL, (void *)f,NULL);
	if(ret){
		printf("create thread fail\n");
		return 1;
	}
	while(1){
		sleep(1);
		printf("f");
	}

	return 0;

}
