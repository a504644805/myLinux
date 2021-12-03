#ifndef _STR
#define _STR

//itoa is a windows API, use sprintf(ANSI API) is better(which both glibc and windows support)
char *itoa(int value, char *string, int radix);

int strcmp(const char *s1, const char *s2);

int strlen(const char *s);

char *strcpy(char *dest, const char *src);

#endif
