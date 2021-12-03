#include "headfile/minicrt.h"
int strcmp(const char *s1, const char *s2){
    int ret;
    for ( ; *s1!=0 && *s2!=0 ; s1++,s2++)
    {
        if(ret=(*s1-*s2) != 0){
            return ret;
        }
    }

    if(*s1==0 && *s2==0){
        return 0;
    }
    else if(*s1==0 && *s2!=0){
        return -1;
    }
    else if(*s1!=0 && *s2==0){
        return 1;
    }
    else{
        //impossible
    }
}

char *itoa(int value, char *string, int radix){
    if(radix<=1||radix>16||string==NULL){
        return NULL;
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
        
        return string;
    }
}

int strlen(const char *s){
    int count=0;
    for(;*s!='\0';s++){
        count++;
    }
    return count;
}