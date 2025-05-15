#include "memory.h"

#define KERNEL_MEMORY_SIZE (1024 * 1024 * 16)  // 16 MB
#define PAGE_SIZE 4096
#define NUM_PAGES (KERNEL_MEMORY_SIZE / PAGE_SIZE)

static uint8_t kernel_memory[KERNEL_MEMORY_SIZE];

// Metadata for each page: 0 = free, 1 = head of allocation, 2 = tail part
static uint8_t page_table[NUM_PAGES];

static size_t last_page_index = 0; 

extern void debug_print(const char* messe);
extern void print_to_screen(const char* message);

void memory_init(uint32_t multiboot_info) {
    (void)multiboot_info;

    for (uint32_t i = 0; i < KERNEL_MEMORY_SIZE; i++) {
        kernel_memory[i] = 0;
    }

    for (size_t i = 0; i < NUM_PAGES; ++i) {
        page_table[i] = 0;
    }

    last_page_index = 0;
    debug_print("DEBUG: Kernel memory initialized.");
}

void* allocate_pages(size_t num_pages) {
    size_t start = last_page_index;
    size_t count = 0;
    size_t found_start = (size_t)-1;

    for (size_t i = 0; i < NUM_PAGES; ++i) {
        size_t index = (start + i) % NUM_PAGES;

        if (page_table[index] == 0) {
            if (found_start == (size_t)-1) {
                found_start = index;
            }
            count++;
            if (count == num_pages) {
                page_table[found_start] = 1; 
                for (size_t j = 1; j < num_pages; ++j) {
                    page_table[(found_start + j) % NUM_PAGES] = 2;  
                }

                last_page_index = (found_start + num_pages) % NUM_PAGES;
                return &kernel_memory[found_start * PAGE_SIZE];
            }
        } else {
            count = 0;
            found_start = (size_t)-1;
        }
    }

    return NULL; 
}

void* kmalloc(size_t size) {
    size_t total_size = size + sizeof(memory_block_t);
    size_t num_pages = (total_size + PAGE_SIZE - 1) / PAGE_SIZE;

    void* mem = allocate_pages(num_pages);
    if (!mem) return NULL;

    memory_block_t* block = (memory_block_t*)mem;
    block->size = size;
    block->is_free = 0;

    return (void*)((uint8_t*)block + sizeof(memory_block_t));
}

void kfree(void* ptr) {
    if (ptr == NULL) return;

    memory_block_t* block = (memory_block_t*)((uint8_t*)ptr - sizeof(memory_block_t));
    block->is_free = 1;

    uintptr_t offset = (uintptr_t)((uint8_t*)block - kernel_memory);
    size_t start_page = offset / PAGE_SIZE;

    if (page_table[start_page] == 1) {
        page_table[start_page] = 0;
        size_t i = start_page + 1;
        while (i < NUM_PAGES && page_table[i] == 2) {
            page_table[i] = 0;
            i++;
        }
    }

    debug_print("DEBUG: Memory freed");
}

void copy_page_tables(uint32_t parent_cr3, uint32_t child_cr3) {
    copy_memory((void*)child_cr3, (void*)parent_cr3, PAGE_SIZE);
}

void copy_memory(void* dest, void* src, size_t size) {
    uint8_t* d = (uint8_t*)dest;
    uint8_t* s = (uint8_t*)src;

    for (size_t i = 0; i < size; i++) {
        d[i] = s[i];
    }
}
