#include <stddef.h>
#include <stdint.h>
#include "../keyboard/string.h"
#include "filesystem.h"
#define min(a, b) (a < b ? a : b)
#define max(a, b) (a > b ? a : b)

Superblock sb;
Inode inode_table[MAX_FILES];
DirectoryEntry root_directory[MAX_FILES]; 
char storage[BLOCK_COUNT][BLOCK_SIZE];
int block_bitmap[BLOCK_COUNT]; 

extern void debug_print(const char* messe);
extern void print_to_screen(const char* message);
extern void itoa(int n, char *str);

void format_disk() {
    sb.inode_count = MAX_FILES;
    sb.block_count = BLOCK_COUNT;
    sb.free_inodes = MAX_FILES;
    sb.free_blocks = BLOCK_COUNT;
    sb.magic = 0xEF53;
    strncpy(sb.volume_name, "ApnaFS", sizeof(sb.volume_name));
    sb.volume_name[sizeof(sb.volume_name) - 1] = '\0';

    for (int i = 0; i < MAX_FILES; i++) {
        inode_table[i].inode_number = 0;
        inode_table[i].size = 0;
        memset(inode_table[i].blocks, -1, sizeof(inode_table[i].blocks));
    }

    for (int i = 0; i < MAX_FILES; i++) {
        root_directory[i].inode_number = 0; 
    }

    for (int i = 0; i < BLOCK_COUNT; i++) {
        block_bitmap[i] = 0;
    }

    debug_print("DEBUG: Disk formatted and file system reset.");
}

void create_file_system() {
    format_disk();
    debug_print("DEBUG: File system created.");
}

int allocate_inode() {
    for (int i = 0; i < MAX_FILES; i++) {
        if (inode_table[i].inode_number == 0) {
            inode_table[i].inode_number = i + 1;
            sb.free_inodes--;
            return i;
        }
    }
    return -1; 
}

int allocate_block() {
    for (int i = 0; i < BLOCK_COUNT; i++) {
        if (block_bitmap[i] == 0) { 
            block_bitmap[i] = 1; 
            sb.free_blocks--; 
            return i; 
        }
    }
    return -1; 
}

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

// we check whether user is authorised for then function he/she wants to perform or not..
int check_permissions(uint16_t permissions, int mode) {
    switch (mode) {
        case 0: return (permissions & 0b100) != 0; 
        case 1: return (permissions & 0b010) != 0; 
        case 2: return (permissions & 0b001) != 0; 
        default: return 0;
    }
}

int chmod_file(const char* filename, uint16_t new_permissions) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (root_directory[i].inode_number != 0 &&
            strncmp(root_directory[i].filename, filename, MAX_FILENAME_LEN) == 0) {
            int inode_index = root_directory[i].inode_number - 1;
            inode_table[inode_index].permissions = new_permissions;
            debug_print("DEBUG: Permissions changed.");
            return 0;
        }
    }
    debug_print("ERROR: File not found.");
    return -1;
}

int create_file(const char* filename) {
    int inode_index = allocate_inode();
    if (inode_index == -1) {
        debug_print("ERROR: No free inodes available.");
        return -1;
    }

    int block_index = allocate_block();
    if (block_index == -1) {
        inode_table[inode_index].inode_number = 0; 
        debug_print("ERROR: No free disk space.");
        return -1;
    }

    
    inode_table[inode_index].size = 0;
    inode_table[inode_index].blocks[0] = block_index;
    inode_table[inode_index].permissions = 0b111;
    
    for (int i = 0; i < MAX_FILES; i++) {
        if (root_directory[i].inode_number == 0) {
            strncpy(root_directory[i].filename, filename, MAX_FILENAME_LEN);
            root_directory[i].inode_number = inode_index + 1;
            debug_print("DEBUG: File created.");
            return inode_index;
        }
    }

    debug_print("ERROR: Root directory full.");
    return -1;
}

int delete_file(const char* filename) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (root_directory[i].inode_number != 0 &&
            strncmp(root_directory[i].filename, filename, MAX_FILENAME_LEN) == 0) {
            
            int inode_index = root_directory[i].inode_number - 1;
            // it's crutial step warna :/ 
            // To check the write permission before deleting the file
            if (!check_permissions(inode_table[inode_index].permissions, 1)) {
                debug_print("ERROR: Permission denied to delete file.");
                return -1;
            }
            for (int j = 0; j < MAX_BLOCKS_PER_FILE; j++) {
                int block_index = inode_table[inode_index].blocks[j];
                if (block_index != -1) {
                    block_bitmap[block_index] = 0; 
                    sb.free_blocks++;
                    inode_table[inode_index].blocks[j] = -1; 
                }
            }
            inode_table[inode_index].inode_number = 0;
            inode_table[inode_index].size = 0;
            sb.free_inodes++;
            root_directory[i].inode_number = 0;
            memset(root_directory[i].filename, 0, MAX_FILENAME_LEN);

            debug_print("DEBUG: File deleted successfully.");
            return 0;
        }
    }
    debug_print("ERROR: File not found.");
    return -1;
}

int read_file(const char* filename, char* buffer, size_t size) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (root_directory[i].inode_number != 0 &&
            strncmp(root_directory[i].filename, filename, MAX_FILENAME_LEN) == 0) {
            
            int inode_index = root_directory[i].inode_number - 1;
            // Check the read permission before reading the file
            if (!check_permissions(inode_table[inode_index].permissions, 0)) {
                debug_print("ERROR: Permission denied to read file.");
                return -1;
            }
            size_t file_size = inode_table[inode_index].size;
            size_t read_size = (size > file_size) ? file_size : size;

            size_t read = 0;
            while (read < read_size) {
                int block_number = read / BLOCK_SIZE;
                int offset = read % BLOCK_SIZE;
                int block_index = inode_table[inode_index].blocks[block_number];
                if (block_index == -1) break;

                size_t chunk = min(BLOCK_SIZE - offset, read_size - read);
                memcpy(buffer + read, storage[block_index] + offset, chunk);
                read += chunk;
            }

            debug_print("DEBUG: File read.");
            return read;
        }
    }
    debug_print("ERROR: File not found.");
    return -1;
}

int write_file(const char* filename, const char* buffer, size_t size) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (root_directory[i].inode_number != 0 &&
            strncmp(root_directory[i].filename, filename, MAX_FILENAME_LEN) == 0) {

            int inode_index = root_directory[i].inode_number - 1;
            // as we should check write permission before writing/modifiying the file
            if (!check_permissions(inode_table[inode_index].permissions, 1)) {
                debug_print("ERROR: Permission denied to write file.");
                return -1;
            }
            size_t max_size = MAX_BLOCKS_PER_FILE * BLOCK_SIZE;
            size_t write_size = min(size, max_size);
 
            int current_blocks = max((inode_table[inode_index].size + BLOCK_SIZE - 1) / BLOCK_SIZE, 1);
            int total_blocks_needed = (write_size + BLOCK_SIZE - 1) / BLOCK_SIZE;
            int needed_blocks = total_blocks_needed - current_blocks;
            
            if (needed_blocks > 0) {
                int allocated = allocate_blocks(inode_index, needed_blocks);
                if (allocated < needed_blocks) {
                    write_size = (current_blocks + allocated) * BLOCK_SIZE;
                    debug_print("WARNING: Not enough space to write full data, truncating.");
                }
            }

            size_t written = 0;
            for (int j = 0; j < MAX_BLOCKS_PER_FILE && written < write_size; j++) {
                int block_index = inode_table[inode_index].blocks[j];
                if (block_index == -1) break;

                size_t chunk = min(BLOCK_SIZE, write_size - written);
                memcpy(storage[block_index], buffer + written, chunk);
                written += chunk;
            }

            inode_table[inode_index].size = written;
            debug_print("DEBUG: File written.");
            return written;
        }
    }
    debug_print("ERROR: File not found.");
    return -1;
}

int append_to_file(const char* filename, const char* buffer, size_t size) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (root_directory[i].inode_number != 0 &&
            strncmp(root_directory[i].filename, filename, MAX_FILENAME_LEN) == 0) {

            int inode_index = root_directory[i].inode_number - 1;
            // To check write permission before appending
            if (!check_permissions(inode_table[inode_index].permissions, 1)) {
                debug_print("ERROR: Permission denied to append to file.");
                return -1;
            }
            size_t current_size = inode_table[inode_index].size;
            size_t max_size = MAX_BLOCKS_PER_FILE * BLOCK_SIZE;

            if (current_size >= max_size) {
                debug_print("ERROR: File already at max size.");
                return -1;
            }

            size_t append_size = min(size, max_size - current_size);

            int current_blocks = 0;
            for (int b = 0; b < MAX_BLOCKS_PER_FILE; ++b) {
                if (inode_table[inode_index].blocks[b] != -1) {
                    current_blocks++;
                }
            }

            size_t used_in_last_block = current_size % BLOCK_SIZE;
            size_t space_left_in_last = (used_in_last_block == 0 && current_size > 0) ? 0 : (BLOCK_SIZE - used_in_last_block);
            size_t overflow = (append_size > space_left_in_last) ? (append_size - space_left_in_last) : 0;
            int needed_blocks = (overflow + BLOCK_SIZE - 1) / BLOCK_SIZE;

            if (needed_blocks > 0) {
                int allocated = allocate_blocks(inode_index, needed_blocks);
                if (allocated < needed_blocks) {
                    append_size = space_left_in_last + allocated * BLOCK_SIZE;
                    debug_print("WARNING: Appending partially due to limited space.");
                }
            }

            
            size_t written = 0;

            while (written < append_size) {
                size_t offset = (current_size + written) % BLOCK_SIZE;
                int block_number = (current_size + written) / BLOCK_SIZE;
                int block_index = inode_table[inode_index].blocks[block_number];

                if (block_index == -1) {
                    debug_print("ERROR: Block not allocated.");
                    break;
                }

                size_t chunk = min(BLOCK_SIZE - offset, append_size - written);
                memcpy(storage[block_index] + offset, buffer + written, chunk);
                written += chunk;
            }

            inode_table[inode_index].size += written;
            debug_print("DEBUG: Data appended to file.");
            return written;
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