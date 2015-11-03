#include <powernex/string.h>


// Copy len bytes from src to dest.
void memcpy(void * dst, const void * src_, uint32_t len) {
	uint8_t * dest = (uint8_t *)dst;
	const uint8_t * src = (uint8_t *)src_;
  for(; len != 0; len--) *dest++ = *src++;
}

// Write len copies of val into dest.
void memset(void * dst, uint8_t val, uint32_t len) {
	uint8_t * dest = (uint8_t *)dst;
  for ( ; len != 0; len--) *dest++ = val;
}

void * memmove(void * dstptr, const void * srcptr, size_t size) {
	unsigned char * dst = (unsigned char*) dstptr;
	const unsigned char * src = (const unsigned char*) srcptr;
	if (dst < src)
		for (size_t i = 0; i < size; i++)
			dst[i] = src[i];
	else
		for (size_t i = size; i != 0; i--)
			dst[i-1] = src[i-1];
	return dstptr;
}

int strcmp(const char * str1, const char * str2) {
  while (*str1 && *str2 && (*str1++ == *str2++))
    ;

  if (*str1 == '\0' && *str2 == '\0')
    return 0;

  if (*str1 == '\0')
    return -1;
  else
		return 1;
}

int strncmp(const char * str1, const char * str2, int count) {
	int i = 0;
  while (*str1 && *str2 && (*str1++ == *str2++) && i++ < count)
    ;

  if ((*str1 == '\0' && *str2 == '\0') || i == count)
    return 0;

  if (*str1 == '\0')
    return -1;
  else
		return 1;
}

char * strcpy(char * dest, const char * src) {
  while (*src)
    *dest++ = *src++;
  *dest = '\0';
	return dest;
}

char * strcat(char * dest, const char * src) {
  while (*(dest++));

  while (*src)
    *dest++ = *src++;
  *dest = '\0';
  return dest;
}

int strlen(const char * src) {
  int i = 0;
  while (*src++)
    i++;
  return i;
}
