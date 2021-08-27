#include <iostream>
#include<signal.h>
#include<pthread.h>

using namespace std;


void handle(int arg) {
    printf("stop wakin’ me up...\n");
}
int main(int argc, char *argv[]) {
    signal(SIGHUP, handle);//kill -HUP pid
    while (1) ;
}