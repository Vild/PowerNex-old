#ifndef STRING_H_
#define STRING_H_

#include <powernex/powernex.h>

void memcpy(void * dest, const void * src, uint32_t len);
void memset(void * dest, uint8_t val, uint32_t len);
void * memmove(void *, const void *, size_t);
int strcmp(const char * str1, const char * str2);
int strncmp(const char * str1, const char * str2, int count);
char * strcpy(char * dest, const char * src);
char * strcat(char * dest, const char * src);
int strlen(const char * src);

#endif
