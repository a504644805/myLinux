#include "string.h"

/*The  memset()  function fills the first n bytes of the memory area pointed
       to by s with the constant byte c.*/
void *memset(void *s, int c, size_t n){
    ASSERT(s!=NULL);
    char* dst=(char*)s;
    for (size_t i = 0; i < n; i++)
    {
        dst[i]=c;
    }
    return s;
}

/*The  memcpy()  function copies n bytes from memory area src to memory area
       dest.  The memory areas must not overlap.  Use memmove(3)  if  the  memory
       areas do overlap.*/
void *memcpy(void *dest, const void *_src, size_t n){
    ASSERT(dest!=NULL && _src!=NULL);
    char* dst=(char*)dest;
    char* src=(char*)_src;
    for (size_t i = 0; i < n; i++)
    {
        dst[i]=src[i];
    }
    return dest;
}

/*The  memcmp()  function  compares  the  first n bytes (each interpreted as
       unsigned char) of the memory areas s1 and s2.*/
int memcmp(const void *_s1, const void *_s2, size_t n){
    ASSERT(_s1!=NULL && _s2!=NULL);
    unsigned char* s1=(unsigned char*)_s1;
    unsigned char* s2=(unsigned char*)_s2; 
    for (size_t i = 0; i < n; i++)
    {
        if(s1[i]==s2[i]){
            continue;
        }
        else{
            return s1[i]-s2[i];
        }
    }
    return 0;
}