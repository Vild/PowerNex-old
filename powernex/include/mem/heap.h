#ifndef HEAP_H_
#define HEAP_H_

#include <powernex/powernex.h>
#include <stdint.h>

#define HEAP_START 0xD0000000
#define HEAP_END   0xFFBFF000

typedef unsigned int u32; // Fix gcc -pedantic problem with bitfield

typedef struct heap_header {
	struct heap_header * prev;
	struct heap_header * next;
	u32 allocated : 1;
	u32 length : 31;
} __attribute__((packed)) heap_header_t;

void heap_init();

void * kmalloc(uint32_t size);
void kfree(void * p);

#endif
