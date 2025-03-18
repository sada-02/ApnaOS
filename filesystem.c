#include "filesystem.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define BLOCK_COUNT 1024
#define BLOCK_SIZE 4096

// Global file system structures
Superblock sb;
Inode inode_table[MAX_FILES];
DirectoryEntry root_directory[MAX_FILES];
char storage[BLOCK_COUNT][BLOCK_SIZE]; // Simulated disk storage
int block_bitmap[BLOCK_COUNT]; // 1 = used, 0 = free

extern void print_to_screen(const char* message);

// Formats the disk and resets the file system
void format_disk() {
    sb.magic = 0x12345678;
    sb.total_blocks = BLOCK_COUNT;
    sb.free_blocks = BLOCK_COUNT;
    sb.block_size = BLOCK_SIZE;

    // Reset inodes and block bitmap
    for (int i = 0; i < MAX_FILES; i++) {
        inode_table[i].inode_number = 0;
        inode_table[i].size = 0;
        memset(inode_table[i].blocks, -1, sizeof(inode_table[i].blocks));
        inode_table[i].indirect_block = -1;
    }

    for (int i = 0; i < MAX_FILES; i++) {
        root_directory[i].inode_number = 0; // Mark as empty
    }

    for (int i = 0; i < BLOCK_COUNT; i++) {
        block_bitmap[i] = 0;
    }

    print_to_screen("DEBUG: Disk formatted and file system reset.");
}

// Initializes the file system (calls format_disk)
void create_file_system() {
    format_disk();
    print_to_screen("DEBUG: File system created.");
}

// Finds a free inode
int allocate_inode() {
    for (int i = 0; i < MAX_FILES; i++) {
        if (inode_table[i].inode_number == 0) {
            inode_table[i].inode_number = i + 1;
            return i;
        }
    }
    return -1; // No free inodes available
}

// Finds a free block in the disk storage
int allocate_block() {
    for (int i = 0; i < BLOCK_COUNT; i++) {
        if (block_bitmap[i] == 0) {
            block_bitmap[i] = 1;
            sb.free_blocks--;
            return i;
        }
    }
    return -1; // No free blocks available
}

// Creates a new file and assigns an inode
int create_file(const char* filename) {
    int inode_index = allocate_inode();
    if (inode_index == -1) {
        print_to_screen("ERROR: No free inodes available.");
        return -1;
    }

    int block_index = allocate_block();
    if (block_index == -1) {
        inode_table[inode_index].inode_number = 0; // Free the inode
        print_to_screen("ERROR: No free disk space.");
        return -1;
    }

    // Initialize the inode
    inode_table[inode_index].size = 0;
    inode_table[inode_index].blocks[0] = block_index;

    // Add to root directory
    for (int i = 0; i < MAX_FILES; i++) {
        if (root_directory[i].inode_number == 0) { // Empty slot
            strncpy(root_directory[i].filename, filename, MAX_FILENAME_LEN);
            root_directory[i].inode_number = inode_index + 1;
            print_to_screen("DEBUG: File created.");
            return inode_index;
        }
    }

    print_to_screen("ERROR: Root directory full.");
    return -1;
}

// Reads data from a file
int read_file(int inode_number, char* buffer, size_t size) {
    int inode_index = inode_number - 1;
    if (inode_index < 0 || inode_index >= MAX_FILES || inode_table[inode_index].inode_number == 0) {
        print_to_screen("ERROR: Invalid inode number.");
        return -1;
    }

    int block_index = inode_table[inode_index].blocks[0]; // First direct block
    if (block_index == -1) {
        print_to_screen("ERROR: File has no allocated blocks.");
        return -1;
    }

    size_t read_size = (size > inode_table[inode_index].size) ? inode_table[inode_index].size : size;
    memcpy(buffer, storage[block_index], read_size);

    print_to_screen("DEBUG: File read.");
    return read_size;
}

// Writes data to a file
int write_file(int inode_number, const char* buffer, size_t size) {
    int inode_index = inode_number - 1;
    if (inode_index < 0 || inode_index >= MAX_FILES || inode_table[inode_index].inode_number == 0) {
        print_to_screen("ERROR: Invalid inode number.");
        return -1;
    }

    int block_index = inode_table[inode_index].blocks[0]; // First direct block
    if (block_index == -1) {
        print_to_screen("ERROR: File has no allocated blocks.");
        return -1;
    }

    size_t write_size = (size > BLOCK_SIZE) ? BLOCK_SIZE : size;
    memcpy(storage[block_index], buffer, write_size);
    inode_table[inode_index].size = write_size;

    print_to_screen("DEBUG: File written.");
    return write_size;
}

// Deletes a file
int delete_file(const char* filename) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (root_directory[i].inode_number != 0 &&
            strncmp(root_directory[i].filename, filename, MAX_FILENAME_LEN) == 0) {
            
            int inode_index = root_directory[i].inode_number - 1;
            int block_index = inode_table[inode_index].blocks[0];

            if (block_index != -1) {
                block_bitmap[block_index] = 0; // Free the block
                sb.free_blocks++;
            }

            inode_table[inode_index].inode_number = 0;
            root_directory[i].inode_number = 0; // Remove from directory

            print_to_screen("DEBUG: File deleted.");
            return 0;
        }
    }
    print_to_screen("ERROR: File not found.");
    return -1;
}
