#include <powernex/mem/pmm.h>
#include <powernex/mem/vmm.h>

uint32_t pmm_stackLoc = PMM_STACK_ADDR;
uint32_t pmm_stackMax = PMM_STACK_ADDR;
uint32_t pmm_location;
bool pmm_pagingActive = 0;

void pmm_init(uint32_t start) {
	pmm_location = (start + 0x1000) & PAGE_MASK;
}

uint32_t pmm_allocPage() {
	if (pmm_pagingActive) {
		if (pmm_stackLoc == PMM_STACK_ADDR)
			panic("Error: Out of memory.");

		pmm_stackLoc -= sizeof(uint32_t);

		return *((uint32_t *)pmm_stackLoc);
	} else
		return pmm_location += 0x1000;
}

void pmm_freePage(uint32_t p) {
	if (p < pmm_location)
		return;

	if (pmm_stackMax <= pmm_stackLoc) {
		vmm_map(pmm_stackMax, p, PAGE_PRESENT | PAGE_WRITE);
		pmm_stackMax += 0x1000;
	} else {
		uint32_t * stack = (uint32_t *)pmm_stackLoc;
		*stack = p;
		pmm_stackLoc += sizeof(uint32_t);
	}
}

void pmm_setUp(multiboot_info_t * multiboot) {
	uint32_t i = multiboot->mmap_addr;
  while (i < multiboot->mmap_addr + multiboot->mmap_length) {
    multiboot_memory_map_t * me = (multiboot_memory_map_t *)i;

    if (me->type == MULTIBOOT_MEMORY_AVAILABLE)
      for (uint64_t j = me->addr; j < me->addr + me->len; j += 0x1000)
        pmm_freePage((uint32_t)j);

    i += me->size + sizeof(uint32_t);
	}
}
