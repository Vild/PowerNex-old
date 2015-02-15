#include <powernex/io/textmode.h>

#include <stdarg.h>
#include <string.h>

#include <powernex/elf.h>
#include <powernex/io/port.h>

const int TAB_SIZE = 8;
const int WIDTH = 80;
const int HEIGHT = 25;

#define makechar(c) ((c & 0xFF) | ((attr & 0xFF) << 8))

static uint16_t * vidmem = (uint16_t *)0xB8000;

static uint8_t attr = DEFAULT_COLOR;
static int x = 0;
static int y = 0;

static char * itoa(int32_t value, char* result, int base);
static char * utoa(uint32_t value, char* result, int base);
static void textmode_updateCursor();
static void kprintf_(const char * str, va_list va);
static inline void kputs_(const char * str);
static inline void kputc_(char str);

void textmode_clear() {
	uint16_t clear = makechar(' ');
	for (int i = 0; i < WIDTH*HEIGHT; i++)
		vidmem[i] = clear;
	x = y = 0;
	textmode_updateCursor();
}


void kprintf(const char * str, ...) {
	va_list va;
	va_start(va, str);
	kprintf_(str, va);
	va_end(va);
}

static void kprintf_(const char * str, va_list va) {
	char buf[64] = "";
	while (*str) {
		if (*str == '%') {
			str++;
			if (!str)
				break;
			
			switch (*str) {
			case 'd':
			case 'i':
				kputs(itoa(va_arg(va, int), buf, 10));
				break;
			case 'u':
				kputs(utoa(va_arg(va, uint32_t), buf, 10));
				break;
			case 'f':
			case 'F':
			case 'e':
			case 'E':
			case 'g':
			case 'G':
			case 'a':
			case 'A':
				(void)va_arg(va, double);
				break;
			case 'x':
			case 'X':
				kputs(utoa(va_arg(va, uint32_t), buf, 16));
				break;
			case 'o':
				kputs(itoa(va_arg(va, int), buf, 8));
				break;
			case 's':
				kputs(va_arg(va, const char *));
				break;
			case 'c':
				kputc((char)va_arg(va, int));
				break;
			case 'p':
				kputs(utoa((uint32_t)va_arg(va, void *), buf, 16));
				break;
			case 'b':
				kputs(itoa(va_arg(va, int), buf, 2));
				break;
			default:
//				kput(*str);
				break;
			}
		}	else
			kputc(*str);
		str++;
	}
	textmode_updateCursor();
}

void kputs(const char * str) {
	kputs_(str);
	textmode_updateCursor();
}

static inline void kputs_(const char * str) {
	while (*str)
		kputc_(*(str++));
}

void kputc(char str) {
	kputc_(str);
	textmode_updateCursor();
}

static inline void kputc_(char c) {
	outb(0xE9, c); //BOCHS
	if (c == '\n') {
		x = 0;
		y++;
	} else if (c == '\r')
		x = 0;
	else if (c == '\b') {
		x--;
		if (x < 0)
			x = 0;
	} else if (c == '\t')
		x = (x+TAB_SIZE) & ~TAB_SIZE; //TODO: Fix
	else {
		vidmem[(y * WIDTH) + x] = makechar(c);
		x++;
	}
	if (x > WIDTH - 1) {
		y++;
		x -= WIDTH;
	}
	if (y > HEIGHT - 1) {
		memmove(vidmem, vidmem+WIDTH, (HEIGHT-1)*WIDTH*2);

		for (int i = 0; i < WIDTH; i++)
			vidmem[(HEIGHT-1)*WIDTH+i] = makechar(' ');
		y--;
	}
}


void kputcolor(uint8_t color) {
	attr = color;
}

void panic(const char * str, ...) {
	kputcolor(makecolor(COLOR_RED, COLOR_LIGHT_GREY));
	kprintf("#### SYSTEM PANIC ####\n");
	va_list va;
	va_start(va, str);
	kprintf_(str, va);
	va_end(va);
	kprintf("####  STACK TRACE ####\n");
	elf_printStackTrace();
	kprintf("######################\n");
	while (1);
}

static void textmode_updateCursor() {
	uint16_t loc = (uint16_t)(y * 80 + x);
	outb(0x3D4, 14);                // Tell the VGA board we are setting the high cursor byte.
	outb(0x3D5, loc >> 8); 					// Send the high cursor byte.
	outb(0x3D4, 15);                // Tell the VGA board we are setting the low cursor byte.
	outb(0x3D5, (uint8_t)loc);	// Send the low cursor byte.
}

/**
 * C++ version 0.4 char* style "itoa":
 * Written by Lukás Chmela
 * Released under GPLv3.

 */
static char * itoa(int32_t value, char* result, int base) {
	// check that the base if valid
	if (base < 2 || base > 36) {
		*result = '\0';
		return result;
	}

	char* ptr = result, *ptr1 = result, tmp_char;
	int32_t tmp_value;

	do {
		tmp_value = value;
		value /= base;
		*ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
	} while (value);

	// Apply negative sign
	if (tmp_value < 0)
		*ptr++ = '-';
	*ptr-- = '\0';
	
	while(ptr1 < ptr) {
		tmp_char = *ptr;
		*ptr--= *ptr1;
		*ptr1++ = tmp_char;
	}
	
	return result;
}


static char * utoa(uint32_t value, char* result, int base) {
	// check that the base if valid
	if (base < 2 || base > 36) {
		*result = '\0';
		return result;
	}

	char* ptr = result, *ptr1 = result, tmp_char;
	uint32_t tmp_value;

	do {
		tmp_value = value;
		value /= base;
		*ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
	} while (value);

	// Apply negative sign
	*ptr-- = '\0';
	
	while(ptr1 < ptr) {
		tmp_char = *ptr;
		*ptr--= *ptr1;
		*ptr1++ = tmp_char;
	}
	
	return result;
}

