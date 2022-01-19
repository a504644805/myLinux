#include "minicrt.h"

int main(int argc,char** argv){
    write(1,"Hi, I am the user prog, my argv[] is:\n",1024);
    
    for (size_t i = 0; i < argc; i++){
        write(1,argv[i],1024);write(1," \n",1024);
    }    
}