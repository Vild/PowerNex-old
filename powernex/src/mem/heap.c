#include <powernex/mem/heap.h>
#include <powernex/mem/pmm.h>
#include <powernex/mem/vmm.h>

uint32_t heap_max = HEAP_START;
heap_header_t * heap_first = NULL;

static void chunk_alloc(uint32_t start, uint32_t len);
static void chunk_split(heap_header_t * chunk, uint32_t len);
static void chunk_glue(heap_header_t * chunk);
static void chunk_free(heap_header_t * chunk);

void heap_init() {

}

void * kmalloc(uint32_t size) {
	size += sizeof(heap_header_t);

	heap_header_t * cur = heap_first;
	heap_header_t * prev = NULL;

	while (cur) {
		if (!cur->allocated && cur->length >= size) {
			chunk_split(cur, size);
			cur->allocated = 1;
			return (void *)((uint32_t)cur + sizeof(heap_header_t));
		}
		prev = cur;
		cur = cur->next;
	}
	uint32_t chunk_start;
	if (prev)
		chunk_start = (uint32_t)prev + prev->length;
	else {
		chunk_start = HEAP_START;
		heap_first = (heap_header_t *)chunk_start;
	}

	chunk_alloc(chunk_start, size);

	cur = (heap_header_t *)chunk_start;
	cur->prev = prev;
	cur->next = NULL;
	cur->allocated = 1;
	cur->length = size;

	prev->next = cur;
	return (void *)(chunk_start + sizeof(heap_header_t));
}

void kfree(void * p) {
	heap_header_t * header = (heap_header_t *)((uint32_t)p - sizeof(heap_header_t));
	header->allocated = 0;
	chunk_glue(header);
}


static void chunk_alloc(uint32_t start, uint32_t len) {
	while (start + len > heap_max) {
		uint32_t page = pmm_allocPage();
		vmm_map(heap_max, page, PAGE_PRESENT | PAGE_WRITE);
		heap_max += 0x1000;
	}
}

static void chunk_split(heap_header_t * chunk, uint32_t len) {
	if (chunk->length - len > sizeof(heap_header_t)) {
		heap_header_t * newChunk = (heap_header_t *) ((uint32_t)chunk + chunk->length);
		newChunk->prev = chunk;
		newChunk->next = NULL;
		newChunk->allocated = 0;
		newChunk->length = chunk->length - len;
		
		chunk->next = newChunk;
		chunk->length = len;
	}
}


static void chunk_glue(heap_header_t * chunk) {
	if (chunk->next && !chunk->next->allocated) {
		chunk->length = chunk->length + chunk->next->length;
		chunk->next->next->prev = chunk;
		chunk->next = chunk->next->next;
	}

	if (chunk->prev && !chunk->prev->allocated) {
		chunk->prev->length = chunk->prev->length + chunk->length;
		chunk->prev->prev = chunk->next;
		chunk->next->prev = chunk->prev;
		chunk = chunk->prev;
	}

	if (!chunk->next)
		chunk_free(chunk);
}


static void chunk_free(heap_header_t * chunk) {
	if (!chunk->prev)
		heap_first = NULL;
	else
		chunk->prev->next = NULL;

	while ((heap_max - 0x1000) >= (uint32_t)chunk) {
		heap_max -= 0x1000;
		uint32_t page;
		vmm_getMapping(heap_max, &page);
		pmm_freePage(page);
		vmm_unmap(heap_max);
	}
}
