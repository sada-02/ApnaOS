#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <stdint.h>

#define MAX_FILENAME_LEN 255
#define MAX_FILES 1024

typedef struct {
    uint32_t magic;          // Magic number to identify the file system
    uint32_t total_blocks;   // Total number of blocks
    uint32_t free_blocks;    // Number of free blocks
    uint32_t block_size;     // Size of each block
} Superblock;

typedef struct {
    uint32_t inode_number;   // Inode number
    uint32_t size;           // Size of the file
    uint32_t blocks[12];     // Direct block pointers
    uint32_t indirect_block; // Indirect block pointer
} Inode;

typedef struct {
    char filename[MAX_FILENAME_LEN];
    uint32_t inode_number;
} DirectoryEntry;

#endif // FILESYSTEM_H