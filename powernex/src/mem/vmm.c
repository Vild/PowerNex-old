#include <powernex/mem/vmm.h>
#include <powernex/string.h>
#include <powernex/mem/pmm.h>
#include <powernex/cpu/idt.h>
#include <powernex/io/textmode.h>

//#error "http://www.jamesmolloy.co.uk/tutorial_html/6.-Paging.html TODO"

uint32_t * pageDirectory = (uint32_t *)PAGE_DIR_VIRTUAL_ADDR;
uint32_t * pageTables = (uint32_t *)PAGE_TABLE_VIRTUAL_ADDR;

vmm_pageDirectory_t * currentDirectory;

static void vmm_pageFault(registers_t * regs);

void vmm_init() {
	uint32_t cr0;
	
	idt_registerHandler(14, &vmm_pageFault);

	vmm_pageDirectory_t * pd = (vmm_pageDirectory_t *)pmm_allocPage();

	memset(pd, 0, 0x1000);

	pd[0] = pmm_allocPage() | PAGE_PRESENT | PAGE_WRITE;
	uint32_t * pt = (uint32_t *) (pd[0] & PAGE_MASK);
	for (int i = 0; i < 1024; i++)
		pt[i] = i*0x1000 | PAGE_PRESENT | PAGE_WRITE;
	
	pd[1022] = pmm_allocPage() | PAGE_PRESENT | PAGE_WRITE;
	pt = (uint32_t *)(pd[1022] & PAGE_MASK);
	memset(pt, 0, 0x1000);

	pt[1023] = (uint32_t)pd | PAGE_PRESENT | PAGE_WRITE;

	pd[1023] = (uint32_t)pd | PAGE_PRESENT | PAGE_WRITE;

	vmm_switchPageDirectory(pd);
	
	__asm__ volatile("mov %%cr0, %0" : "=r"(cr0));
	cr0 |= 0x80000000;	
	__asm__ volatile("mov %0, %%cr0" : : "r"(cr0));
	
	uint32_t pt_idx = PAGE_DIR_IDX((PMM_STACK_ADDR >> 12));
	pageDirectory[pt_idx] = pmm_allocPage() | PAGE_PRESENT | PAGE_WRITE;
	memset((void *)pageTables[pt_idx*1024], 0, 0x1000);

	pmm_pagingActive = true;
}

void vmm_switchPageDirectory(vmm_pageDirectory_t * pd) {
	currentDirectory = pd;
	__asm__ volatile("mov %0, %%cr3" : : "r" (pd));
}

void vmm_map(uint32_t va, uint32_t pa, uint32_t flags) {
	uint32_t virtual_page = va / 0x1000;
	uint32_t pt_idx = PAGE_DIR_IDX(virtual_page);

	if (!pageDirectory[pt_idx]) {
		pageDirectory[pt_idx] = pmm_allocPage() | PAGE_PRESENT | PAGE_WRITE;
		memset((void*)pageTables[pt_idx*1024], 0, 0x1000);
	}
	pageTables[virtual_page] = (pa & PAGE_MASK) | flags;
}

void vmm_unmap(uint32_t va) {
	uint32_t virtal_page = va / 0x1000;
	pageTables[virtal_page] = 0;
	__asm__ volatile("invlpg (%0)" : : "a" (va));
}

bool vmm_getMapping(uint32_t va, uint32_t * pa) {
	uint32_t virtual_page = va / 0x1000;
	uint32_t pt_idx = PAGE_DIR_IDX(virtual_page);
	if (!pageDirectory[pt_idx])
		return false;

	if (pageTables[virtual_page]) {
		if (pa)
			*pa = pageTables[virtual_page] & PAGE_MASK;
		return true;
	}
	return false;
}

static void vmm_pageFault(registers_t * regs) {
	uint32_t cr2;
	__asm__ volatile("mov %%cr2, %0" : "=r" (cr2));

	// The error code gives us details of what happened.
	int present   = !(regs->err_code & 0x1); // Page not present
	int rw = regs->err_code & 0x2;           // Write operation?
	int us = regs->err_code & 0x4;           // Processor was in user-mode?
	int reserved = regs->err_code & 0x8;     // Overwritten CPU-reserved bits of page entry?
	int id = regs->err_code & 0x10;          // Caused by an instruction fetch?
   
	panic("Page fault! (%s%s%s%s%s) at 0x%x, faulting address 0x%x\nError code: %x\n",
	      present ? "present " : "",
	      rw ? "read-only " : "",
	      us ? "user-mode " : "",
	      reserved ? "reserved " : "",
	      id ? "instructio_Fetch " : "",
	      regs->eip, cr2, regs->err_code);
}
