#ifndef STR_H
#define STR_H
#include "global.h"
#include "debug.h"
/*The  memset()  function fills the first n bytes of the memory area pointed
       to by s with the constant byte c.*/
void *memset(void *s, int c, size_t n);

/*The  memcpy()  function copies n bytes from memory area src to memory area
       dest.  The memory areas must not overlap.  Use memmove(3)  if  the  memory
       areas do overlap.*/
void *memcpy(void *dest, const void *_src, size_t n);

/*The  memcmp()  function  compares  the  first n bytes (each interpreted as
       unsigned char) of the memory areas s1 and s2.*/
int memcmp(const void *_s1, const void *_s2, size_t n);


/* The  strcpy()  function copies the string pointed to by src, including the
       terminating null byte ('\0'), to the  buffer  pointed  to  by  dest.   The
       strings  may  not  overlap,  and the destination string dest must be large
       enough to receive the copy.  Beware of buffer overruns!  (See BUGS.)*/
char *strcpy(char *dest, const char *src);

/*The strcat() function appends the src string to the dest string, overwrit‐
       ing the terminating null byte ('\0') at the end of dest, and then  adds  a
       terminating  null  byte.  The strings may not overlap, and the dest string
       must have enough space for the result.*/
char *strcat(char *dest, const char *src);

/*The  strcmp()  function compares the two strings s1 and s2.  It returns an
       integer less than, equal to, or greater than zero if s1 is found,  respec‐
       tively, to be less than, to match, or be greater than s2.*/
int strcmp(const char *s1, const char *s2);


/*The  strchr()  function  returns  a pointer to the first occurrence of the
       character c in the string s.*/
char *strchr(const char *s, int c);

/*The strrchr() function returns a pointer to the  last  occurrence  of  the
       character c in the string s.*/
char *strrchr(const char *s, int c);

/*The strlen() function calculates the length of the string pointed to by s,
       excluding the terminating null byte ('\0').*/
size_t strlen(const char *s);


#endif