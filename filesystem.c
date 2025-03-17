#include "filesystem.h"
#include "memory.h" // For memory allocation functions
#include <string.h>
extern void print_to_screen(const char*);

// Global filesystem structures for our dummy implementation
static Inode inode_table[MAX_FILES];
static DirectoryEntry directory_entries[MAX_FILES];
static uint32_t next_inode = 1;      // Inode numbers start at 1
static uint32_t next_dir_entry = 0;  // Next available directory entry

void format_disk() {
    // Initialize superblock
    Superblock sb;
    sb.magic = 0x12345678;
    sb.total_blocks = 1024;
    sb.free_blocks = 1024;
    sb.block_size = 4096;

    // Write superblock to disk (dummy implementation)
    print_to_screen("DEBUG: Disk formatted with new file system.");
}

void create_file_system() {
    format_disk();
    // Initialize other structures like inode table, root directory, etc.
    print_to_screen("DEBUG: File system created.");
}

// int create_file(const char* filename) {
//     // Dummy implementation to create a file
//     print_to_screen("DEBUG: File created.");
//     return 0; // Return file descriptor
// }
int create_file(const char* filename) {
    // Check if directory is full
    if (next_dir_entry >= MAX_FILES) {
        print_to_screen("ERROR: Directory full. Cannot create new file.\n");
        return -1;
    }
    
    // Check filename length (strncpy will use up to MAX_FILENAME_LEN)
    if (strlen(filename) > MAX_FILENAME_LEN) {
        print_to_screen("ERROR: Filename is too long.\n");
        return -1;
    }
    
    // Allocate a new inode number and initialize an inode structure
    uint32_t inode_num = next_inode++;
    Inode inode;
    inode.inode_number = inode_num;
    inode.size = 0;  // New file is empty
    for (int i = 0; i < 12; i++) {
        inode.blocks[i] = 0;  // No data blocks allocated yet
    }
    inode.indirect_block = 0;
    
    // Save the new inode in the inode table (using inode_num - 1 as index)
    inode_table[inode_num - 1] = inode;
    
    // Create a new directory entry
    DirectoryEntry *entry = &directory_entries[next_dir_entry++];
    strncpy(entry->filename, filename, MAX_FILENAME_LEN);
    entry->filename[MAX_FILENAME_LEN - 1] = '\0'; // Ensure null termination
    entry->inode_number = inode_num;
    
    print_to_screen("DEBUG: File created successfully.\n");
    return inode_num; // Returning the inode number as the file descriptor
}


int read_file(int fd, char* buffer, size_t size) {
    // Dummy implementation to read a file
    print_to_screen("DEBUG: File read.");
    return size; // Return number of bytes read
}

int write_file(int fd, const char* buffer, size_t size) {
    // Dummy implementation to write a file
    print_to_screen("DEBUG: File written.");
    return size; // Return number of bytes written
}

int delete_file(const char* filename) {
    // Dummy implementation to delete a file
    print_to_screen("DEBUG: File deleted.");
    return 0; // Return success
}
