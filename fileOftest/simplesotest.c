#include<stdio.h>
#include<dlfcn.h>

//gcc -o simplesotest simplesotest.c -ldl
//./simplesotest /lib/.../libm-...

//double (*f)(double);
int main(int argc,char* argv[]){
	void* handle=dlopen(argv[1],RTLD_NOW);
	if(handle==NULL){	
		printf("fail dlopen\n");
		return 0;
	}
	
	void* f=dlsym(handle,argv[2]);
	if((error=dlerror())!=NULL){
		printf("fail dlsym\n");
		return 0;
	}
	double rt;

//	printf("sin(3.1415926)=%f\n",f(3.1415926));
	
	dlclose(handle);
}
