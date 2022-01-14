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


/* The  strcpy()  function copies the string pointed to by src, including the
       terminating null byte ('\0'), to the  buffer  pointed  to  by  dest.   The
       strings  may  not  overlap,  and the destination string dest must be large
       enough to receive the copy.  Beware of buffer overruns!  (See BUGS.)*/
char *strcpy(char *dest, const char *src){
    char* r=dest;
    while(*(dest++)=*(src++));
    return r;
}

/*The strcat() function appends the src string to the dest string, overwrit‐
       ing the terminating null byte ('\0') at the end of dest, and then  adds  a
       terminating  null  byte.  The strings may not overlap, and the dest string
       must have enough space for the result.*/
char *strcat(char *dest, const char *src){
    ASSERT(dest!=NULL && src!=NULL);
    char* str = dest;
    while (*str++);
    --str;
    while((*str++ = *src++)); 
    return dest;
}

/*The  strcmp()  function compares the two strings s1 and s2.  It returns an
       integer less than, equal to, or greater than zero if s1 is found,  respec‐
       tively, to be less than, to match, or be greater than s2.*/
int strcmp(const char *s1, const char *s2){
    while(*s1 && *s2){
        if(*s1 == *s2){
            s1++;
            s2++;
            continue;
        }
        else{
            return *s1-*s2;
        }
    }
    //有一方先读完
    if(!(*s1) && !(*s2)){
        return 0;
    }
    else if(!(*s1)){
        return -1;
    }
    else if(!(*s2)){
        return 1;
    }
    else{
        ASSERT(1==2);
    }
}

/*The strlen() function calculates the length of the string pointed to by s,
       excluding the terminating null byte ('\0').*/
size_t strlen(const char *s){
    int count=0;
    for(;*s!='\0';s++){
        count++;
    }
    return count;
}


char *uitoa(uint32_t value, char *string, int radix){
    if(radix<=1||radix>16||string==NULL){
        return NULL;
    }
    else{
        if(value==0){
            *string='0';
        }
        else{
            char t[]={'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};
            char* s=string;
            for(;value!=0;value/=radix){
                *s++ = t[value%radix];
            }
            //reverse
            s--;
            char* s2=string;
            char temp;
            for(;s>s2;s--,s2++){
                temp=*s;
                *s=*s2;
                *s2=temp;
            }
        }
        return string;
    }
}


