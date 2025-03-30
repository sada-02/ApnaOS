#include <stddef.h>
#include <stdint.h>
#include "string.h"
#include "filesystem.h"

#define BLOCK_COUNT 1024 // No of blocks
#define BLOCK_SIZE 4096 // 4 KB
#define MAX_BLOCKS_PER_FILE 2 // 2 blocks or 8 KB size

// Global file system structures
Superblock sb;
Inode inode_table[MAX_FILES];
DirectoryEntry root_directory[MAX_FILES];
char storage[BLOCK_COUNT][BLOCK_SIZE]; // Simulated disk storage of size 4 MB
int block_bitmap[BLOCK_COUNT]; // 1 = used, 0 = free

extern void debug_print(const char* messe);
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

    debug_print("DEBUG: Disk formatted and file system reset.");
}

// Initializes the file system (calls format_disk)
void create_file_system() {
    format_disk();
    debug_print("DEBUG: File system created.");
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

int allocate_block() {
    for (int i = 0; i < BLOCK_COUNT; i++) {
        if (block_bitmap[i] == 0) { // Find a free block
            block_bitmap[i] = 1; // Mark as allocated
            sb.free_blocks--; // Update superblock
            return i; // Return the allocated block index
        }
    }
    return -1; // No free blocks available
}

// Finds a free block in the disk storage
int allocate_blocks(int inode_index, size_t required_blocks) {
    int allocated = 0;
    for (int i = 0; i < MAX_BLOCKS_PER_FILE && allocated < required_blocks; i++) {
        if (inode_table[inode_index].blocks[i] == -1) {
            int new_block = allocate_block();
            if (new_block == -1) break;
            inode_table[inode_index].blocks[i] = new_block;
            allocated++;
        }
    }
    return allocated;
}


// Creates a new file and assigns an inode
int create_file(const char* filename) {
    int inode_index = allocate_inode();
    if (inode_index == -1) {
        debug_print("ERROR: No free inodes available.");
        return -1;
    }

    int block_index = allocate_block();
    if (block_index == -1) {
        inode_table[inode_index].inode_number = 0; // Free the inode
        debug_print("ERROR: No free disk space.");
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
            debug_print("DEBUG: File created.");
            return inode_index;
        }
    }

    debug_print("ERROR: Root directory full.");
    return -1;
}

// Updated delete_file function
int delete_file(const char* filename) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (root_directory[i].inode_number != 0 &&
            strncmp(root_directory[i].filename, filename, MAX_FILENAME_LEN) == 0) {
            
            int inode_index = root_directory[i].inode_number - 1;

            // Free all blocks allocated to the file
            for (int j = 0; j < MAX_BLOCKS_PER_FILE; j++) {
                int block_index = inode_table[inode_index].blocks[j];
                if (block_index != -1) {
                    block_bitmap[block_index] = 0; // Mark block as free
                    sb.free_blocks++;
                    inode_table[inode_index].blocks[j] = -1; // Reset block entry
                }
            }

            // Reset inode entry
            inode_table[inode_index].inode_number = 0;
            inode_table[inode_index].size = 0;
            inode_table[inode_index].indirect_block = -1;

            // Clear directory entry
            root_directory[i].inode_number = 0;
            memset(root_directory[i].filename, 0, MAX_FILENAME_LEN);

            debug_print("DEBUG: File deleted successfully.");
            return 0;
        }
    }
    debug_print("ERROR: File not found.");
    return -1;
}

// Updated read_file function
int read_file(const char* filename, char* buffer, size_t size) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (root_directory[i].inode_number != 0 &&
            strncmp(root_directory[i].filename, filename, MAX_FILENAME_LEN) == 0) {
            
            int inode_index = root_directory[i].inode_number - 1;
            int block_index = inode_table[inode_index].blocks[0]; // First direct block
            if (block_index == -1) {
                debug_print("ERROR: File has no allocated blocks.");
                return -1;
            }

            size_t read_size = (size > inode_table[inode_index].size) ? inode_table[inode_index].size : size;
            memcpy(buffer, storage[block_index], read_size);

            debug_print("DEBUG: File read.");
            return read_size;
        }
    }
    debug_print("ERROR: File not found.");
    return -1;
}

// Updated write_file function
int write_file(const char* filename, const char* buffer, size_t size) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (root_directory[i].inode_number != 0 &&
            strncmp(root_directory[i].filename, filename, MAX_FILENAME_LEN) == 0) {
            
            int inode_index = root_directory[i].inode_number - 1;
            int block_index = inode_table[inode_index].blocks[0]; // First direct block
            if (block_index == -1) {
                debug_print("ERROR: File has no allocated blocks.");
                return -1;
            }

            size_t write_size = (size > BLOCK_SIZE) ? BLOCK_SIZE : size;
            memcpy(storage[block_index], buffer, write_size);
            inode_table[inode_index].size = write_size;

            debug_print("DEBUG: File written.");
            return write_size;
        }
    }
    debug_print("ERROR: File not found.");
    return -1;
}

// Updated append_to_file function
int append_to_file(const char* filename, const char* buffer, size_t size) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (root_directory[i].inode_number != 0 &&
            strncmp(root_directory[i].filename, filename, MAX_FILENAME_LEN) == 0) {
            
            int inode_index = root_directory[i].inode_number - 1;
            size_t current_size = inode_table[inode_index].size;
            if (current_size + size > BLOCK_SIZE) {
                debug_print("ERROR: File size exceeds block limit.");
                return -1;
            }

            int block_index = inode_table[inode_index].blocks[0];
            memcpy(storage[block_index] + current_size, buffer, size);
            inode_table[inode_index].size += size;

            debug_print("DEBUG: Data appended to file.");
            return size;
        }
    }
    debug_print("ERROR: File not found.");
    return -1;
}

void list_files() {
    debug_print("DEBUG: Listing files.");
    for (int i = 0; i < MAX_FILES; i++) {
        if (root_directory[i].inode_number != 0) {
            print_to_screen(root_directory[i].filename);
            print_to_screen("\n");
        }
    }
}

