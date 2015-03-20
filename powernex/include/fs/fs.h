#ifndef FS_H_
#define FS_H_

#include <powernex/powernex.h>

enum {
	FS_FILE        = 0x1,
	FS_DIRECTORY   = 0x2,
	FS_CHARDEVICE  = 0x3,
	FS_BLOCKDEVICE = 0x4,
	FS_PIPE        = 0x5,
	FS_SYMLINK     = 0x6,
	FS_MOUNTPOINT  = 0x7
};

typedef struct fs_node fs_node_t;
typedef struct fs_dirent fs_dirent_t;

typedef uint32_t (*fs_read_f)(fs_node_t * node, uint32_t offset, uint32_t size, uint8_t * buffer);
typedef uint32_t (*fs_write_f)(fs_node_t * node, uint32_t offset, uint32_t size, uint8_t * buffer);
typedef void (*fs_open_f)(fs_node_t * node);
typedef void (*fs_close_f)(fs_node_t * node);
typedef fs_dirent_t * (*fs_readdir_f)(fs_node_t * node, uint32_t index);
typedef fs_node_t * (*fs_finddir_f)(fs_node_t * node, const char * name);

struct fs_node {
	char name[128];
	uint32_t mask;
	uint32_t uid;
	uint32_t gid;
	uint32_t flags;
	uint32_t inode;
	uint32_t length;
	uint32_t impl;

	fs_read_f read;
	fs_write_f write;
	fs_open_f open;
	fs_close_f close;
	fs_readdir_f readdir;
	fs_finddir_f finddir;

	fs_node_t * ptr; // Used by mountpoints and symlinks.
};

struct fs_dirent {
	char name[128];
	uint32_t ino;
};

extern fs_node_t * fs_root;

uint32_t fs_read(fs_node_t * node, uint32_t offset, uint32_t size, uint8_t * buffer);
uint32_t fs_write(fs_node_t * node, uint32_t offset, uint32_t size, uint8_t * buffer);
void fs_open(fs_node_t * node);
void fs_close(fs_node_t * node);
fs_dirent_t * fs_readdir(fs_node_t * node, uint32_t index);
fs_node_t * fs_finddir(fs_node_t * node, const char * name);

#endif
