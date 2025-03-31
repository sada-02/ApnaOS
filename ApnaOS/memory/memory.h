/* memory.h - Memory management declarations */
#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>
#include <stddef.h>

// Simple memory block header to track allocations
typedef struct memory_block {
    size_t size;
    uint8_t is_free;
} memory_block_t;

/* Initialize memory subsystem */
void memory_init(uint32_t multiboot_info);

/* Kernel memory allocation */
void* kmalloc(size_t size);

/* Page allocation */
void* allocate_pages(size_t num_pages);

/* Memory operation utilities */
void copy_page_tables(uint32_t parent_cr3, uint32_t child_cr3);
void copy_memory(void* dest, void* src, size_t size);

#endif /* MEMORY_H */