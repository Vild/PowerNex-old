#include <stdbool.h> /* C doesn't have booleans by default. */
#include <stddef.h>
#include <stdint.h>
#include <powernex/multiboot.h>
#include <powernex/io/textmode.h>
#include <powernex/cpu/gdt.h>
#include <powernex/cpu/idt.h>

int kmain(int multiboot_magic, multiboot_info_t * multiboot) {
	textmode_clear(); // Also initalizes textmode
	kprintf("Welcome to PowerNex!\n");
	if (multiboot_magic == MULTIBOOT_BOOTLOADER_MAGIC)
		kprintf("You are using '%s' as your bootloader.\n", multiboot->boot_loader_name);
	else
		kprintf("ERROR:\tUNKNOWN BOOTLOADER\n");
	
	gdt_init();
	idt_init();

	__asm__ volatile("int $0x1");
	__asm__ volatile("int $0x2");
	__asm__ volatile("int $0x3");
	__asm__ volatile("int $0x4");
	return 0xDEADBEEF;
}
