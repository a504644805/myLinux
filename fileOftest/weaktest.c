#include<stdio.h>

int __foo(){
	printf("__foo called\n");
}

int foo() __attribute__(weak,(alias("__foo")));


int main(){
	foo();	
}
