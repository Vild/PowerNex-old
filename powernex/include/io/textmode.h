#ifndef TEXTMODE_H_
#define TEXTMODE_H_

#include <powernex/powernex.h>
#include <stdarg.h>
#include <powernex/io/textbuffer.h>

extern textbuffer_t * currentTTY;

void textmode_clear();
void textmode_resize(int width, int height);
void kprintf(const char * str, ...);
void kprintf_va(const char * str, va_list va);
void kputc(char c);
void kputs(const char * str);
void kputcolor(uint8_t color);

#endif
