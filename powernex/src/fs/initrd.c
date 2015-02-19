#include <powernex/fs/initrd.h>
#include <powernex/string.h>
#include <powernex/mem/heap.h>

initrd_header_t * initrd_header;
initrd_fileHeader_t * file_headers;
fs_node_t * initrd_root;
fs_node_t * initrd_dev;
fs_node_t * root_nodes;
uint32_t root_nodes_count;

fs_dirent_t dirent;

static uint32_t initrd_read(fs_node_t * node, uint32_t offset, uint32_t size, uint8_t * buffer) {
	initrd_fileHeader_t header = file_headers[node->inode];
	if (offset > header.length)
		return 0;

	if ((offset + size) > header.length)
		size = header.length - offset;

	memcpy(buffer, (uint8_t *)(header.offset+offset), size);
	return size;
}

static fs_dirent_t * initrd_readdir(fs_node_t * node, uint32_t index) {
	if (node == initrd_root && index == 0) {
		strcpy(dirent.name, "dev");
		dirent.ino = 0;
		return &dirent;
	}

	if ((index - 1) >= root_nodes_count)
		return NULL;

	strcpy(dirent.name, root_nodes[index - 1].name);
	dirent.ino = root_nodes[index-1].inode;
	return &dirent;
}

static fs_node_t * initrd_finddir(fs_node_t * node, const char * name) {
	if (node == initrd_root && !strcmp(name, "dev"))
		return initrd_dev;

	for (uint32_t i = 0; i < root_nodes_count; i++)
		if (!strcmp(name, root_nodes[i].name))
			return &root_nodes[i];
	
	return NULL;
}

fs_node_t * initrd_init(uint32_t location) {
	initrd_header = (initrd_header_t *)location;
	file_headers = (initrd_fileHeader_t *)(location + sizeof(initrd_header_t));

	initrd_root = (fs_node_t *)kmalloc(sizeof(fs_node_t));
	memset(initrd_root, 0, sizeof(fs_node_t));
	strcpy(initrd_root->name, "initrd");
	initrd_root->flags = FS_DIRECTORY;
	initrd_root->readdir = &initrd_readdir;
	initrd_root->finddir = &initrd_finddir;

	initrd_dev = (fs_node_t *)kmalloc(sizeof(fs_node_t));
	memset(initrd_dev, 0, sizeof(fs_node_t));
	strcpy(initrd_dev->name, "dev");
	initrd_dev->flags = FS_DIRECTORY;
	initrd_dev->readdir = &initrd_readdir;
	initrd_dev->finddir = &initrd_finddir;

	uint32_t totalsize = sizeof(fs_node_t) * initrd_header->count;
	root_nodes = (fs_node_t *)kmalloc(totalsize);
	root_nodes_count = initrd_header->count;
	memset(root_nodes, 0, totalsize);
	for (uint32_t i = 0; i < initrd_header->count; i++) {
		file_headers[i].offset += location;

		strcpy(root_nodes[i].name, file_headers[i].name);
		root_nodes[i].length = file_headers[i].length;
		root_nodes[i].inode = i;
		root_nodes[i].flags = file_headers[i].type;
		root_nodes[i].read = &initrd_read;
	}
	return initrd_root;
}
