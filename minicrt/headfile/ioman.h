#ifndef _IO_MANAGEMENT
#define _IO_MANAGEMENT
typedef struct{
    int fd;
}FILE;
#define O_RDONLY            0
#define O_WRONLY            1
#define O_RDWR              2
#define O_CREAT          0x40
#define O_TRUNC         0x200
#define O_APPEND        0x400

#define S_IRWXU  00700

int init_io();

int open(const char *pathname, int flags, int mode);
int close(int fd);
int write(int fd, const void *buf, int count);

//current only support r,r+,w,w+
FILE *fopen(const char *pathname, const char *mode);
int fclose(FILE *stream);
int fwrite(const void *ptr, int size, int nmemb,FILE *stream);


int fputc(int c, FILE *stream);
int fputs(const char *s, FILE *stream);

int vfprintf(FILE *stream, const char *format, va_list ap);
int fprintf(FILE *stream, const char *format, ...);
int printf(const char *format, ...);


#endif