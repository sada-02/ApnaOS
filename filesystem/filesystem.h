#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <stdint.h>
#include <stddef.h>
#include "string.h"

#define MAX_FILENAME_LEN 255
#define MAX_FILES 512
// #define BLOCK_COUNT 128 * 1024 // No of blocks
// #define BLOCK_SIZE 4096 // 4 KB
// #define MAX_BLOCKS_PER_FILE 256 // 256 blocks or 1 MB size

// Superblock structure
typedef struct {
    uint32_t magic;          
    uint32_t total_blocks;   
    uint32_t free_blocks;   
    uint32_t block_size;   
} Superblock;


typedef struct {
    uint32_t inode_number;   
    uint32_t size;           
    uint32_t blocks[12];     
    uint32_t indirect_block; 
} Inode;

typedef struct {
    char filename[MAX_FILENAME_LEN];
    uint32_t inode_number;
} DirectoryEntry;

void format_disk();
void create_file_system();
int allocate_blocks(int inode_index, size_t required_blocks);
int create_file(const char* filename);
int read_file(const char* filename, char* buffer, size_t size);
int write_file(const char* filename, const char* buffer, size_t size);
int append_to_file(const char* filename, const char* buffer, size_t size);
int delete_file(const char* filename);
void list_files();

#endif // FILESYSTEM_H
