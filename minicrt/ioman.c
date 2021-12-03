#include "headfile/minicrt.h"

/*
 The mode argument specifies the file mode bits be applied when a new file is created. This argument  must  be  supplied  when  O_CREAT  or  O_TMPFILE is specified in flags; if neither O_CREAT nor O_TMPFILE is specified, then mode is ignored.
*/
int open(const char *pathname, int flags, int mode){
    int retval;
    asm volatile("syscall":"=a"(retval):"a"(2),"D"(pathname),"S"(flags),"d"(mode));
    return retval;
}

int close(int fd){
    int retval;
    asm volatile("syscall":"=a"(retval):"a"(3),"D"(fd));
    return retval;
}

int write(int fd, const void *buf, int count){
    int retval;
    asm volatile("syscall":"=a"(retval):"a"(1),"D"(fd),"S"(buf),"d"(count));
    return retval;
}


//current only support r,r+,w,w+
FILE *fopen(const char *pathname, const char *mode){

    int flags=0;
    if(strcmp(mode,"r")==0){
        flags=O_RDONLY;
    }
    else if(strcmp(mode,"w")==0){
        flags=O_WRONLY | O_CREAT | O_TRUNC;        
    }
    else if(strcmp(mode,"r+")==0){
        flags=O_RDWR;
    }
    else if(strcmp(mode,"w+")==0){
        flags= O_RDWR | O_CREAT | O_TRUNC;
    }
    else{
        return NULL;
    }

    FILE* ret=(FILE*)malloc(sizeof(FILE));
    ret->fd=open(pathname,flags,S_IRWXU);
    if(ret->fd < 0){
        free(ret);
        return NULL;
    }
    else{
        return ret;
    }
}

int fclose(FILE *stream){
    int ret=close(stream->fd);
    free(stream);
    if(ret<0){
        return -1;
    }
    else{
        return 0;
    }

}

//On  success,  the  number of bytes written is returned
//On error, -1 is returned and errno is set appropriately(Pretend :)
int fwrite(const void *ptr, int size, int nmemb,FILE *stream){
    int ret=write(stream->fd,ptr,nmemb*size);
    if(ret<0){
        return -1;
    }
    else{
        return ret;
    }
}

int init_io(){

    return 0;
}

int vfprintf(FILE *stream, const char *format, va_list ap){
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
                fputc('%',stream);
                s=N;
            }
            break;
        case 'd':
            if(s==N){
                fputc('d',stream);
            }
            else{
                char buf[32];//put buf here to make sure buf would init with 0 every time get into this case, so that after itoa, buf end with '\0'
                fputs(itoa(va_arg(ap,int),buf,10),stream);
                s=N;
            }
            break;
        case 's':
            if(s==N){
                fputc('s',stream);
            }
            else{
                fputs(va_arg(ap,char*),stream);
                s=N;
            }
            break;
        default:
            fputc(*format,stream);
            break;
        }
    }

    return 0;//I'm kind tired so no error judges :(
}

int fputc(int c, FILE *stream){
    return fwrite((void*)(&c),1,1,stream);
}
int fputs(const char *s, FILE *stream){
    return fwrite(s,strlen(s),1,stream);
}
int fprintf(FILE *stream, const char *format, ...){
    va_list ap;
    va_start(ap,format);
    return vfprintf(stream,format,ap);
}
asmlinkage int printf(const char *format, ...){
    FILE stdout;
    stdout.fd=1;
    va_list ap;
    va_start(ap,format);
    return vfprintf(&stdout,format,ap);

}
