#ifndef TEXTBUFFER_H_
#define TEXTBUFFER_H_

#include <powernex/powernex.h>
#include <stdint.h>
#include <stdarg.h>

#define makecolor(f, b) (((b&0xF) << 4) | (f & 0xF))

#define DEFAULT_COLOR makecolor(COLOR_CYAN, COLOR_BLACK)

typedef enum textbuffer_color {
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
	COLOR_YELLOW = 14,
	COLOR_WHITE = 15,
	MAX_COLOR_NUMS
} textbuffer_color_t;


typedef struct textbuffer {
	char * text;
	uint8_t * color;
	int width;
	int height;
	int x;
	int y;
} textbuffer_t;

void tb_init(textbuffer_t * buf, int width, int height);
void tb_initClone(textbuffer_t * buf, int width, int height, textbuffer_t * parent);
void tb_free(textbuffer_t * buf);
void tb_clear(textbuffer_t * buf);
void tb_putc(textbuffer_t * buf, textbuffer_color_t color, char c);
void tb_puts(textbuffer_t * buf, textbuffer_color_t color, const char * str);
void tb_printf(textbuffer_t * buf, textbuffer_color_t color, const char * str, ...);
void tb_printf_va(textbuffer_t * buf, textbuffer_color_t color, const char * str, va_list va);

#endif
