#include <powernex/mem/pmm.h>
#include <powernex/mem/vmm.h>

uint32_t pmm_stackLoc = PMM_STACK_ADDR;
uint32_t pmm_stackMax = PMM_STACK_ADDR;
uint32_t pmm_location;
bool pmm_pagingActive;

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
