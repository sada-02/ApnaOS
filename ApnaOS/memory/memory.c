#include "memory.h"

/* Kernel memory pool */
#define KERNEL_MEMORY_SIZE (1024 * 1024 * 16)  // 16 MB
static uint8_t kernel_memory[KERNEL_MEMORY_SIZE];
static uint32_t kernel_memory_offset = 0;

extern void debug_print(const char* messe);
extern void print_to_screen(const char* message);

void kfree(void* ptr) {
    if (ptr == NULL) {
        return; // Nothing to free
    }
    
    // Get the memory block header (located just before the allocated memory)
    memory_block_t* block = (memory_block_t*)((uint8_t*)ptr - sizeof(memory_block_t));
    
    // Mark the block as free
    block->is_free = 1;
    
    debug_print("DEBUG: Memory freed");
}


void memory_init(uint32_t multiboot_info) {
    // Initialize kernel memory
    for (uint32_t i = 0; i < KERNEL_MEMORY_SIZE; i++) {
        kernel_memory[i] = 0;
    }
    kernel_memory_offset = 0;
    
    debug_print("DEBUG: Kernel memory initialized.");
    
    // Additional memory initialization would go here
    // Parse multiboot_info to set up page tables, etc.
}

void* kmalloc(size_t size) {
    // Calculate total size needed including the header
    size_t total_size = size + sizeof(memory_block_t);
    
    if (kernel_memory_offset + total_size > KERNEL_MEMORY_SIZE) {
        return NULL; // Out of memory
    }
    
    // Get pointer to the memory block header
    memory_block_t* block = (memory_block_t*)&kernel_memory[kernel_memory_offset];
    
    // Initialize the header
    block->size = size;
    block->is_free = 0;
    
    // Update the offset
    kernel_memory_offset += total_size;
    
    // Ensure alignment to 4 bytes
    if (kernel_memory_offset % 4 != 0) {
        kernel_memory_offset += 4 - (kernel_memory_offset % 4);
    }
    
    // Return pointer to the allocated memory (after the header)
    return (void*)((uint8_t*)block + sizeof(memory_block_t));
}


void* allocate_pages(size_t num_pages) {
    return kmalloc(num_pages * 4096);
}

void copy_page_tables(uint32_t parent_cr3, uint32_t child_cr3) {
    // In a real implementation, this would copy page tables
    // For now, just use memcpy as a placeholder
    copy_memory((void*)child_cr3, (void*)parent_cr3, 4096);
}

void copy_memory(void* dest, void* src, size_t size) {
    uint8_t* d = (uint8_t*)dest;
    uint8_t* s = (uint8_t*)src;
    
    for (size_t i = 0; i < size; i++) {
        d[i] = s[i];
    }
}