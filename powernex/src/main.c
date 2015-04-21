#include <powernex/powernex.h>
#include <powernex/elf.h>
#include <powernex/multiboot.h>
#include <powernex/io/textmode.h>
#include <powernex/cpu/gdt.h>
#include <powernex/cpu/idt.h>
#include <powernex/cpu/pit.h>
#include <powernex/cpu/task.h>
#include <powernex/io/port.h>
#include <powernex/io/keyboard.h>
#include <powernex/mem/heap.h>
#include <powernex/mem/paging.h>
#include <powernex/fs/fs.h>
#include <powernex/fs/initrd.h>
#include <powernex/string.h>
#include <powernex/cli.h>
#include <powernex/io/vbe.h>
#include <stdarg.h>

static void step(const char * msg, ...);
static void setup(multiboot_info_t * multiboot);

int kmain(UNUSED int multiboot_magic, multiboot_info_t * multiboot) {
	setup(multiboot);
	kputc('\n');
	
	cli_start();
	return 0xDEADBEEF;
}

static void step(const char * str, ...) {
	kputc('[');
	kputcolor(makecolor(COLOR_MAGENTA, COLOR_BLACK));
	kputc('*');
	kputcolor(DEFAULT_COLOR);
	kputc(']');
	kputc(' ');
	va_list va;
	va_start(va, str);
	kprintf_va(str, va);
	va_end(va);
	kputc('\n');
}

static void setup(multiboot_info_t * multiboot) {
  //Textmode
	textmode_clear(); // Also initalizes textmode
	
	//GDT
	step("Initializing GDT...");
	gdt_init();
	step("Initializing IDT...");
	idt_init();

	//Initializing FPU (Floating pointer unit)
	__asm__ volatile ("fninit");
	uint16_t tmp = 0x37F;
	__asm__ volatile("fldcw %0" :: "m"(tmp));
	__asm__ volatile ("clts");
	size_t t;
	__asm__ volatile ("mov %%cr4, %0" : "=r"(t));
	t |= 3 << 9;
	__asm__ volatile ("mov %0, %%cr4" :: "r"(t));

	//Memory	
	step("Initializing paging with %d MB...", (multiboot->mem_lower + multiboot->mem_upper)/1024);
	paging_init(multiboot);

	//Debuging
	step("Initializing Backtrace...");
	elf_init(&(multiboot->u.elf_sec));

	//Multithreading
	step("Initializing Multithreading...");
	task_init();

	//Graphics
	step("Initializing VBE");
	vbe_init(multiboot);

	//Hardware
	step("Initializing PIT with %d HZ...", 50);
	pit_init(50/*HZ*/);

	//Register keyboard
	step("Initializing Keyboard driver...");
	kb_init();

	//Exceptions
	step("Initializing enabling Exceptions...");
	__asm__ volatile("sti");

	step("Initializing Initrd...");
	if (multiboot->mods_count == 0)
		panic("No initrd defined in grub.cfg!");
	
	uint32_t initrd_location = *((uint32_t *)multiboot->mods_addr);
  // uint32_t initrd_end = *(uint32_t *)(multiboot->mods_addr+4);
	fs_root = initrd_init(initrd_location);
}
