#include<iostream>

#include <sys/wait.h>
#include<stdlib.h>
#include<stdio.h>
#include<pthread.h>
#include<unistd.h>
#include<sys/types.h>
#include<assert.h>
#include<string.h>
#include<time.h>
#include<math.h>

using namespace std;

int main(int argc,char* argv[]){
	assert(argc>1);
	for(int i=1;i<argc;i++){
		int j=fork();
		if(j==-1){
			cout<<"fork fail"<<endl;
			return -1;
		}
		else if(j==0){
			char* dst=strdup("/home/xy/my_trash/");
			strcat(dst,argv[i]);
			strcat(dst,"_");
			char t[50];
			//cout<<"prefix:"<<dst<<endl;//
			sprintf(t,"%ld",time(NULL));
			//cout<<"t:"<<t<<endl;//
			strcat(dst,t);
			//cout<<"final:"<<dst<<endl;//
			execlp("mv","mv",argv[i],dst,NULL);
		}
		else{
			wait(NULL);
			//cout<<i<<endl;
		}
	}

	return 0;
}
