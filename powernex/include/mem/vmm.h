#ifndef VMM_H_
#define VMM_H_

#include <powernex/powernex.h>
#include <stdint.h>
#include <stdbool.h>

#define PAGE_PRESENT 0x1
#define PAGE_WRITE 0x2
#define PAGE_USER 0x4
#define PAGE_MASK 0xFFFFF000
#define PAGE_DIR_VIRTUAL_ADDR 0xFFBFF000
#define PAGE_TABLE_VIRTUAL_ADDR 0xFFC00000

#define PAGE_DIR_IDX(x) ((uint32_t)x/1024)
#define PAGE_TABLE_IDX(x) ((uint32_t)x%1024)

typedef uint32_t vmm_pageDirectory_t;

void vmm_init();

void vmm_switchPageDirectory(vmm_pageDirectory_t * pd);

void vmm_map(uint32_t va, uint32_t pa, uint32_t flags);
void vmm_unmap(uint32_t va);

bool vmm_getMapping(uint32_t va, uint32_t * pa);

#endif
