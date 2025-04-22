#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>
#include <stddef.h>

typedef struct memory_block {
    size_t size;
    uint8_t is_free;
} memory_block_t;

void memory_init(uint32_t multiboot_info);

void* kmalloc(size_t size);

void* allocate_pages(size_t num_pages);

void copy_page_tables(uint32_t parent_cr3, uint32_t child_cr3);
void copy_memory(void* dest, void* src, size_t size);

#endif