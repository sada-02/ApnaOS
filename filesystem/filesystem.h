#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <stdint.h>
#include <stddef.h>
#include "../keyboard/string.h"

#define BLOCK_COUNT 1024        // No of blocks
#define BLOCK_SIZE 4096         // 4 KB
#define MAX_BLOCKS_PER_FILE 4   // 4 blocks or 16 KB size
#define MAX_FILES 256           // 4 * 256 * 4 KB = 4 MB
#define MAX_FILENAME_LEN 255

typedef struct {
    uint32_t inode_count;           
    uint32_t block_count;           
    uint32_t free_inodes;           
    uint32_t free_blocks;           
    uint16_t magic;                 
    char     volume_name[16];       
} Superblock;

typedef struct {
    uint32_t inode_number;
    uint32_t size;
    uint32_t blocks[MAX_BLOCKS_PER_FILE];
    uint16_t permissions;
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

#endif 
