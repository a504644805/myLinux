#include<stdio.h>

void f(int c){

}

typeof(f) f2{
	printf("f2\n");
}
//make break points and whatis c
int main(){
	int a=0;
	__typeof(a) c=0;
	while(1){
		c++;
		printf("done\n");
	}
}
