#ifndef INITRD_H_
#define INITRD_H_

#include <powernex/powernex.h>
#include <powernex/fs/fs.h>

typedef struct initrd_header {
	uint32_t count;
} initrd_header_t;

typedef struct initrd_fileHeader {
	uint8_t magic;
	char name[128];
	uint32_t offset;
	uint32_t length;
	uint32_t type;
	int32_t parent;
} initrd_fileHeader_t;

fs_node_t * initrd_init(uint32_t location);

#endif
