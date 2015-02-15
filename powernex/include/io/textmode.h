#ifndef TEXTMODE_H_
#define TEXTMODE_H_

#include <powernex/powernex.h>

#define makecolor(f, b) (((b&0xF) << 4) | (f & 0xF))

#define DEFAULT_COLOR makecolor(COLOR_CYAN, COLOR_BLACK)

enum textmode_color {
	COLOR_BLACK = 0,
	COLOR_BLUE = 1,
	COLOR_GREEN = 2,
	COLOR_CYAN = 3,
	COLOR_RED = 4,
	COLOR_MAGENTA = 5,
	COLOR_BROWN = 6,
	COLOR_LIGHT_GREY = 7,
	COLOR_DARK_GREY = 8,
	COLOR_LIGHT_BLUE = 9,
	COLOR_LIGHT_GREEN = 10,
	COLOR_LIGHT_CYAN = 11,
	COLOR_LIGHT_RED = 12,
	COLOR_LIGHT_MAGENTA = 13,
	COLOR_LIGHT_BROWN = 14,
	COLOR_WHITE = 15,
	MAX_COLOR_NUMS
};

void textmode_clear();
void kprintf(const char * str, ...);
void kputc(char c);
void kputs(const char * str);
void kputcolor(uint8_t color);

#endif
