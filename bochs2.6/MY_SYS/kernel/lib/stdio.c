#include "stdio.h"

int printf(const char *format, ...){
    va_list ap;
    va_start(ap,format);
    char buf[1024]={0};
    vsprintf(buf,format,ap);
    return write(buf);
}

int vsprintf(char *str, const char *format, va_list ap){
    enum state{
        N,
        T
    };
    enum state s=N;

    for(;*format!='\0';format++){
        switch (*format)
        {
        case '%':
            if(s==N){
                s=T;
            }
            else{//s==T
                *(str++)='%';
                s=N;
            }
            break;
        case 'd':
            if(s==N){
                *(str++)='d';
            }
            else{
                int i=va_arg(ap,int);
                if(i<0){
                    *(str++)='-';
                    i=-i;
                }
                uitoa(i,str,10);
                str+=strlen(str);
                s=N;
            }
            break;
        case 'x':{
            if(s==N){
                *(str++)='x';
            }
            else{
                uitoa(va_arg(ap,uint32_t),str,16);
                str+=strlen(str);
                s=N;
            }
            break;
        }
        case 's':
            if(s==N){
                *(str++)='s';
            }
            else{
                strcpy(str,va_arg(ap,char*));
                str+=strlen(str);
                s=N;
            }
            break;
        default:
            *(str++)=*format;
            break;
        }
    }

    return 0;//I'm kind tired so no error judges :(
}
