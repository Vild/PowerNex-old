#ifndef HEAP_H_
#define HEAP_H_

#include <powernex/powernex.h>

#define HEAP_START 0xD0000000
#define HEAP_END   0xFFBFF000

typedef struct heap_header {
	struct heap_header * prev;
	struct heap_header * next;
	BITS(1, allocated);
	BITS(31, length);
} __attribute__((packed)) heap_header_t;

void heap_init();

void * kmalloc(uint32_t size);
void kfree(void * p);

#endif
