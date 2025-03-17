#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>
#include <stddef.h>

// In 32-bit mode, we use a Page Directory with 1024 entries.
typedef struct PageDirectory {
    uint32_t entries[1024];
} PageDirectory;

// Read CR3 returns a 32-bit address in 32-bit mode.
uint32_t read_cr3();
PageDirectory* active_page_directory();

// Paging information structure
typedef struct {
    PageDirectory* directory;
} PagingInfo;

// Initializes paging (dummy example)
PagingInfo init_paging();

// Dummy frame allocator structures
typedef struct {
} EmptyFrameAllocator;

void* allocate_frame_empty();

// Memory map region definition (32-bit addresses)
typedef struct {
    uint32_t start;
    uint32_t end;
    uint32_t type;    // 1 = usable, other values reserved.
} MemoryRegion;

// Boot info frame allocator structure
typedef struct {
    MemoryRegion* regions;
    size_t region_count;
    size_t next_region;
} BootInfoFrameAllocator;

BootInfoFrameAllocator init_frame_allocator(MemoryRegion* regions, size_t region_count);
void* allocate_frame(BootInfoFrameAllocator* allocator);

// The kernel main function – note the single parameter: pointer to multiboot info.
void memory_init(uint32_t multiboot_info);

#endif // MEMORY_H
