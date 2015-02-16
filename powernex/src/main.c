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
#include <powernex/mem/heap.h>
#include <powernex/mem/pmm.h>
#include <powernex/mem/vmm.h>

static void setup(multiboot_info_t * multiboot);
static void welcome();

int kmain(int multiboot_magic, multiboot_info_t * multiboot) {
	setup(multiboot);

	welcome();
	
	kprintf("Welcome to PowerNex!\n");
	if (multiboot_magic == MULTIBOOT_BOOTLOADER_MAGIC)
		kprintf("You are using '%s' as your bootloader.\n", multiboot->boot_loader_name);
	else
		kprintf("ERROR:\tUNKNOWN BOOTLOADER\n");

	kputcolor(makecolor(COLOR_RED, COLOR_GREEN));
	while(true) {
		kprintf("a");
		thread_sleep(1000);
	}
	
	while(true);
	
	return 0xDEADBEEF;
}

static void welcome() {
	const char * line1 = "Welcome to PowerNex!";
	const char * line2 = "POWER for the NEXt generation";
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
	gdt_init();
	idt_init();

	//Memory
	pmm_init(multiboot->mem_upper);
	vmm_init();
	heap_init();
	pmm_setUp(multiboot);

	//Debuging
	elf_init(&(multiboot->u.elf_sec));

	//Exceptions
	__asm__ volatile("sti");

	//Multithreading
	scheduler_init(thread_init());

	//Hardware
	pit_init(100/*HZ*/);
}
