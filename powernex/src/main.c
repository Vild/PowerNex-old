#include <stdbool.h> /* C doesn't have booleans by default. */
#include <stddef.h>
#include <stdint.h>
#include <powernex/multiboot.h>
#include <powernex/io/textmode.h>
#include <powernex/cpu/gdt.h>
#include <powernex/cpu/idt.h>
#include <powernex/cpu/pit.h>

int kmain(int multiboot_magic, multiboot_info_t * multiboot) {
	textmode_clear(); // Also initalizes textmode
	kprintf("Welcome to PowerNex!\n");
	if (multiboot_magic == MULTIBOOT_BOOTLOADER_MAGIC)
		kprintf("You are using '%s' as your bootloader.\n", multiboot->boot_loader_name);
	else
		kprintf("ERROR:\tUNKNOWN BOOTLOADER\n");

	kprintf("\n\n\n\n");
	
	gdt_init();
	idt_init();
	pit_init(20);
	__asm__ volatile("sti");
	while(true);
	
	return 0xDEADBEEF;
}
