#include<stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char *argv[]){
    char buffer[80] = "/home/xy/test/t";
    int fd = open(buffer, O_RDONLY);
    int size = read(fd, buffer, sizeof(buffer));
    close(fd);
}
