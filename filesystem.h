#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <stdint.h>
#include <stddef.h>

#define MAX_FILENAME_LEN 255
#define MAX_FILES 1024
#define BLOCK_COUNT 1024
#define BLOCK_SIZE 4096

// Superblock structure
typedef struct {
    uint32_t magic;          // Magic number to identify the file system
    uint32_t total_blocks;   // Total number of blocks
    uint32_t free_blocks;    // Number of free blocks
    uint32_t block_size;     // Size of each block
} Superblock;

// Inode structure
typedef struct {
    uint32_t inode_number;   // Unique identifier for file
    uint32_t size;           // File size in bytes
    uint32_t blocks[12];     // Direct block pointers (12 direct blocks)
    uint32_t indirect_block; // Pointer to an indirect block (for larger files)
} Inode;

// Directory entry
typedef struct {
    char filename[MAX_FILENAME_LEN];
    uint32_t inode_number; // Links to an inode
} DirectoryEntry;

// Function prototypes
void format_disk();
void create_file_system();
int create_file(const char* filename);
int read_file(int inode_number, char* buffer, size_t size);
int write_file(int inode_number, const char* buffer, size_t size);
int delete_file(const char* filename);

#endif // FILESYSTEM_H
