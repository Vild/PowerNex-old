#include <stdbool.h> /* C doesn't have booleans by default. */
#include <stddef.h>
#include <stdint.h>
#include <powernex/elf.h>
#include <powernex/multiboot.h>
#include <powernex/io/textmode.h>
#include <powernex/cpu/gdt.h>
#include <powernex/cpu/idt.h>
#include <powernex/cpu/pit.h>
#include <powernex/io/port.h>
#include <powernex/mem/pmm.h>
#include <powernex/mem/vmm.h>

void a();
void b();
void c();

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

	if (inb(0xE9) == 0xE9)
		kprintf("Bochs detected\n");
	
  pmm_init(multiboot->mem_upper);
	vmm_init();

	uint32_t i = multiboot->mmap_addr;
  while (i < multiboot->mmap_addr + multiboot->mmap_length) {
    multiboot_memory_map_t * me = (multiboot_memory_map_t *)i;

    if (me->type == MULTIBOOT_MEMORY_AVAILABLE)
      for (uint64_t j = me->addr; j < me->addr + me->len; j += 0x1000)
        pmm_freePage((uint32_t)j);

    i += me->size + sizeof(uint32_t);
	}

	elf_init(&multiboot->u.elf_sec);
	//a();
	

	__asm__ volatile("sti");
	while(true);
	
	return 0xDEADBEEF;
}

void a() {
	b();
}

void b() {
	c();
}

void c() {
	panic("WOW, works\n");
}
