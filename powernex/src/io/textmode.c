#include <powernex/io/textmode.h>

#include <string.h>

#include <powernex/elf.h>
#include <powernex/mem/heap.h>

static textbuffer_t * earlyBoot = NULL;
static textbuffer_t * tty = NULL;
textbuffer_t * currentTTY = NULL;

static uint8_t color = DEFAULT_COLOR;

void textmode_clear() {
	if (!tty) {
		earlyBoot = tty = kmalloc(sizeof(textbuffer_t));
		tb_init(tty, 80, 25);
	} else
		tb_clear(tty);

  currentTTY = tty;
}

void textmode_resize(int width, int height) {
	textbuffer_t * tty_ = kmalloc(sizeof(textbuffer_t));
	tb_initClone(tty_, width, height, currentTTY);
	currentTTY = tty_;
	if (tty != earlyBoot)
		tb_free(tty);
	tty = currentTTY;
	return;
}

void kprintf(const char * str, ...) {
	va_list va;
	va_start(va, str);
  kprintf_va(str, va);
	va_end(va);
}

void kprintf_va(const char * str, va_list va) {
  tb_printf_va(tty, color, str, va);
}

void kputs(const char * str) {
  tb_puts(tty, color, str);
}

void kputc(char c) {
	tb_putc(tty, color, c);
}

void kputcolor(uint8_t newcolor) {
	color = newcolor;
}

void panic(const char * str, ...) {
	kputcolor(makecolor(COLOR_RED, COLOR_LIGHT_GREY));
	kprintf("#### SYSTEM PANIC ####\n");
	va_list va;
	va_start(va, str);
	kprintf_va(str, va);
	va_end(va);
	kprintf("\n####  STACK TRACE ####\n");
	elf_printStackTrace();
	kprintf("######################\n");
	while (1);
}
