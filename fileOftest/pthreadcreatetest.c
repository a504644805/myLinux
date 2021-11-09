#include<iostream>
#include<unistd.h>
#include<pthread.h>
using namespace std;

struct args_t{
	int a;
	int b;
};

struct ret_t{
	int a;
};

void* f(void* args){
	while(1){
		sleep(1);
		cout<<"c";
	}
}


int main(){
	pthread_t pt;
	cout<<unitbuf;
	int ret=pthread_create(&pt, NULL, (void *)f,NULL);
	if(ret){
		cout<<"create thread fail"<<endl;
		return 1;
	}
	while(1){
		sleep(1);
		cout<<"f";
	}

	return 0;

}
