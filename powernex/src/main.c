#include <stdbool.h> /* C doesn't have booleans by default. */
#include <stddef.h>
#include <stdint.h>
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

/*void a();
void b();
void c();*/

int fn(UNUSED void *arg) {
	while (true) {
		kprintf("a");
	}
	return 0xDEAD;
}	


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

	pmm_init(multiboot->mem_upper);
	vmm_init();

	heap_init();

	uint32_t i = multiboot->mmap_addr;
  while (i < multiboot->mmap_addr + multiboot->mmap_length) {
    multiboot_memory_map_t * me = (multiboot_memory_map_t *)i;

    if (me->type == MULTIBOOT_MEMORY_AVAILABLE)
      for (uint64_t j = me->addr; j < me->addr + me->len; j += 0x1000)
        pmm_freePage((uint32_t)j);

    i += me->size + sizeof(uint32_t);
	}

	elf_init(&(multiboot->u.elf_sec));
	

	__asm__ volatile("sti");
	
	scheduler_init(thread_init());

	uint32_t * stack = kmalloc(sizeof(uint32_t)*0x100);
	stack += 0x100-1;

	thread_t * t = thread_create(&fn, NULL, stack);
	(void) t;
	while (true)
		kprintf("b");
	
	while(true);
	
	return 0xDEADBEEF;
}
