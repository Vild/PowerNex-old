#include <powernex/powernex.h>
#include <powernex/elf.h>
#include <powernex/multiboot.h>
#include <powernex/io/textmode.h>
#include <powernex/cpu/gdt.h>
#include <powernex/cpu/idt.h>
#include <powernex/cpu/pit.h>
#include <powernex/cpu/thread.h>
#include <powernex/cpu/scheduler.h>
#include <powernex/io/port.h>
#include <powernex/io/keyboard.h>
#include <powernex/mem/heap.h>
#include <powernex/mem/pmm.h>
#include <powernex/mem/vmm.h>
#include <powernex/fs/fs.h>
#include <powernex/fs/initrd.h>
#include <powernex/string.h>
#include <stdarg.h>

static void step(const char * msg, ...);
static void setup(multiboot_info_t * multiboot);
static void welcome();

int kmain(UNUSED int multiboot_magic, multiboot_info_t * multiboot) {
	setup(multiboot);
	kputc('\n');
	
	welcome();
	kputc('\n');

	char user[64];
	char pass[64];
	uint32_t count;
	while(true) { //Login
		kprintf("PowerNex login: ");
		user[0] = pass[0] = count = 0;
		while (true) { //Readline user
			char c = kb_getc();
			if (c) {
				if (c == '\n') {
					kputc('\n');
					break;
				} else if (c == '\b') {
					if (count > 0) {
						--count;
						user[count] = '\0';
						kputc('\b');
					}
				} else {
					kputc(c);
					if (count < sizeof(user) - 1)
						user[count++] = c;
				}
			}
		}
		user[count] = 0;
		count = 0;
		kprintf("Password: ");
		while (true) { //Readline pass
			char c = kb_getc();
			if (c) {
				if (c == '\n') {
					kputc('\n');
					break;
				} else if (c == '\b') {
					if (count > 0) {
						--count;
						pass[count] = '\0';
						kputc('\b');
					}
				} else {
					kputc('*'); //No echo on password
				
					if (count < sizeof(pass) - 1)
						pass[count++] = c;
				}
			}
		}
		pass[count] = 0;
		if (!strcmp(user, "root") && !strcmp(pass, "root"))
			break;
		
		kputcolor(makecolor(COLOR_RED, COLOR_BLACK));
		kprintf("Sorry, try again.\n\n");
		kputcolor(DEFAULT_COLOR);
	}
	
	kputc('\n');

	while (true) {
		kprintf("root@PowerNex# ");
		while (true) {
			char c = kb_getc();
			if (c)
				kputc(c);
			if (c == '\n')
				break;
		}
		kputcolor(makecolor(COLOR_RED, COLOR_BLACK));
		kprintf("NOT IMPLEMENTED YET\n");
		kputcolor(DEFAULT_COLOR);
	}
	
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

static void welcome() {
	const char * line1 = "Welcome to PowerNex!";
	const char * line2 = "Power for the Next generation";
	const char * line3 = "Version: ";
	const char * line3_ = build_git_version;
	const char * line4 = "Created by: Dan Printzell";
	const char * line5 = "License: MIT? maybe";

	
  kputcolor(makecolor(COLOR_GREEN, COLOR_BLACK));
	kputc(' '   );kputc(' '   );kputc('\xB3');kputc(' '   );kputc(' '   );
	kputcolor(DEFAULT_COLOR);kprintf(" %s", line1);kputc('\n');
  kputcolor(makecolor(COLOR_GREEN, COLOR_BLACK));
	
	kputc('\xC9');kputc('\xCD');kputc('\xB3');kputc('\xCD');kputc('\xBB');
	kputcolor(DEFAULT_COLOR);kprintf(" %s", line2);kputc('\n');
  kputcolor(makecolor(COLOR_GREEN, COLOR_BLACK));
	
	kputc('\xBA');kputc(' '   );kputc('\xB3');kputc(' '   );kputc('\xBA');
	kputcolor(DEFAULT_COLOR);kprintf(" %s%s", line3, line3_);kputc('\n');
	kputcolor(makecolor(COLOR_GREEN, COLOR_BLACK));
	
	kputc('\xBA');kputc(' '   );kputc(' '   );kputc(' '   );kputc('\xBA');
	kputcolor(DEFAULT_COLOR);kprintf(" %s", line4);kputc('\n');
  kputcolor(makecolor(COLOR_GREEN, COLOR_BLACK));
	
	kputc('\xC8');kputc('\xCD');kputc('\xCD');kputc('\xCD');kputc('\xBC');
	kputcolor(DEFAULT_COLOR);kprintf(" %s", line5);kputc('\n');
}

static void setup(multiboot_info_t * multiboot) {
	//Textmode
	textmode_clear(); // Also initalizes textmode
	
	//GDT
	step("Initializing GDT...");
	gdt_init();
	step("Initializing IDT...");
	idt_init();

	//Memory	
	step("Initializing PMM with %d MB lower, %d MB upper...", multiboot->mem_lower, multiboot->mem_upper);
	pmm_init(multiboot->mem_upper);
	step("Initializing VMM...");
	vmm_init();
	step("Initializing Heap...");
	heap_init();
	
	step("Initializing memory...");
	pmm_setUp(multiboot);

	//Debuging
	step("Initializing Backtrace...");
	elf_init(&(multiboot->u.elf_sec));

	//Exceptions
	step("Initializing enabling Exceptions...");
	__asm__ volatile("sti");

	//Multithreading
	step("Initializing Scheduler...");
	scheduler_init(thread_init());

	//Hardware
	step("Initializing PIT with %d HZ...", 100);
	pit_init(100/*HZ*/);

	//Register keyboard
	step("Initializing Keyboard driver...");
	kb_init();

	step("Initializing Initrd...");
	if (multiboot->mods_count == 0)
		panic("No initrd defined in boot.cfg!");
	
	uint32_t initrd_location = *((uint32_t *)multiboot->mods_addr);
  // uint32_t initrd_end = *(uint32_t *)(multiboot->mods_addr+4);
	fs_root = initrd_init(initrd_location);
}
