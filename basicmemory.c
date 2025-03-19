#include "basicmemory.h"

/* Kernel memory pool */
#define KERNEL_MEMORY_SIZE (1024 * 1024 * 16)  // 16 MB
static uint8_t kernel_memory[KERNEL_MEMORY_SIZE];
static uint32_t kernel_memory_offset = 0;

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
    if (kernel_memory_offset + size > KERNEL_MEMORY_SIZE) {
        return NULL;  // Out of memory
    }
    
    void* ptr = &kernel_memory[kernel_memory_offset];
    kernel_memory_offset += size;
    
    // Ensure alignment to 4 bytes
    if (kernel_memory_offset % 4 != 0) {
        kernel_memory_offset += 4 - (kernel_memory_offset % 4);
    }
    
    return ptr;
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