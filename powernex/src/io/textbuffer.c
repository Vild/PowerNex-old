#include <powernex/io/textbuffer.h>

#include <powernex/string.h>
#include <powernex/mem/heap.h>

const int TAB_SIZE = 8;

static char * itoa(int32_t value, char* result, int base);
static char * utoa(uint32_t value, char* result, int base);

void tb_init(textbuffer_t * buf, int width, int height) {
	buf->width = width;
	buf->height = height;
	buf->text = kmalloc(width*height);
	buf->color = kmalloc(width*height);
	tb_clear(buf);
}

#include <powernex/io/port.h>
void tb_initClone(textbuffer_t * buf, int width, int height, textbuffer_t * parent) {
	tb_init(buf, width, height);
	
	int yoff = 0;
	if (height < parent->y)
		yoff = parent->y - height;

	if (false && width >= parent->width) {
		for (int y = yoff; y < parent->y; y++) {
			memcpy(&(buf->text[(y-yoff)*width]), &(parent->text[y*parent->width]), parent->width);
			memcpy(&(buf->color[(y-yoff)*width]), &(parent->color[y*parent->width]), parent->width);
		}
		buf->x = parent->x;
		buf->y = parent->y - yoff;
	} else {		
		for (int y = 0; y < parent->y; y++)
			for (int x = 0; x < parent->x; x++)
				tb_putc(buf, parent->color[(y * parent->width)+x], parent->text[(y * parent->width)+x]);
	}
}

void tb_free(textbuffer_t * buf) {
	if (!buf)
		return;
	kfree(buf->color);
	kfree(buf->text);
}

void tb_clear(textbuffer_t * buf) {
	memset(buf->text, ' ', buf->width*buf->height);
	memset(buf->color, DEFAULT_COLOR, buf->width*buf->height);
	buf->x = 0;
	buf->y = 0;
}

void tb_putc(textbuffer_t * buf, textbuffer_color_t color, char c) {
	if (!buf)
		return;
	if (c == '\n') {
		buf->x = 0;
		buf->y++;
	} else if (c == '\r')
		buf->x = 0;
	else if (c == '\b') {
		buf->x--;
		if (buf->x < 0) {
			buf->x = buf->width - 1;
			buf->y--;
			if (buf->y < 0) {
				buf->y = 0;
				buf->x = 0;
			}
		}
		buf->text[(buf->y * buf->width) + buf->x] = ' ';
		buf->color[(buf->y * buf->width) + buf->x] = DEFAULT_COLOR;
	} else if (c == '\t')
		buf->x = (buf->x+TAB_SIZE) & ~TAB_SIZE;
	else {
		buf->text[(buf->y * buf->width) + buf->x] = c;
		buf->color[(buf->y * buf->width) + buf->x] = color;
		buf->x++;
	}
	if (buf->x > buf->width - 1) {
		buf->y++;
		buf->x -= buf->width;
	}
	if (buf->y > buf->height - 1) {
		memmove(buf->text, buf->text+buf->width, (buf->height-1)*buf->width);
		memmove(buf->color, buf->color+buf->width, (buf->height-1)*buf->width);

		for (int i = 0; i < buf->width; i++) {
			buf->text[(buf->height-1)*buf->width+i] = ' ';
			buf->color[(buf->height-1)*buf->width+i] = DEFAULT_COLOR;
		}
		buf->y--;
	}
}

void tb_puts(textbuffer_t * buf, textbuffer_color_t color, const char * str) {
	if (!buf)
		return;
	while (*str)
		tb_putc(buf, color, *(str++));
}

void tb_printf(textbuffer_t * buf, textbuffer_color_t color, const char * str, ...) {
	if (!buf)
		return;
	va_list va;
	va_start(va, str);
	tb_printf_va(buf, color, str, va);
	va_end(va);
}

void tb_printf_va(textbuffer_t * buf, textbuffer_color_t color, const char * str, va_list va) {
	if (!buf)
		return;
	char tmp[64] = "";
	while (*str) {
		if (*str == '%') {
			str++;
			if (!str)
				break;
			
			switch (*str) {
			case 'd':
			case 'i':
				tb_puts(buf, color, itoa(va_arg(va, int), tmp, 10));
				break;
			case 'u':
				tb_puts(buf, color, utoa(va_arg(va, uint32_t), tmp, 10));
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
				tb_puts(buf, color, utoa(va_arg(va, uint32_t), tmp, 16));
				break;
			case 'o':
				tb_puts(buf, color, itoa(va_arg(va, int), tmp, 8));
				break;
			case 's':
				tb_puts(buf, color, va_arg(va, const char *));
				break;
			case 'c':
				tb_putc(buf, color, (char)va_arg(va, int));
				break;
			case 'p':
				tb_puts(buf, color, utoa((uint32_t)va_arg(va, void *), tmp, 16));
				break;
			case 'b':
				tb_puts(buf, color, itoa(va_arg(va, int), tmp, 2));
				break;
			default:
//				kput(*str);
				break;
			}
		}	else
			tb_putc(buf, color, *str);
		str++;
	}
}

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
