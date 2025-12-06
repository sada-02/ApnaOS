#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <stdint.h>
#include <stddef.h>
#include "../keyboard/string.h"

#define BLOCK_COUNT 1024        // No of blocks
#define BLOCK_SIZE 4096         // size 4 KB each
#define MAX_BLOCKS_PER_FILE 4   // 4 blocks or 16 KB size
#define MAX_FILES 256           // 4 * 256 * 4 KB = 4 MB
#define MAX_FILENAME_LEN 255
#define MAX_PATH_LEN 1024
#define MAX_ENTRIES_PER_DIR 64

#define FILE_TYPE_REGULAR 0
#define FILE_TYPE_DIRECTORY 1

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
    uint8_t file_type;
    uint32_t parent_inode;
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
int chmod_file(const char* filename, uint16_t new_permissions);
void list_files();
int create_directory(const char* dirname);
int change_directory(const char* path);
void get_current_path(char* buffer, size_t size);
int resolve_path(const char* path);

#endif 
