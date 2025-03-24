/* memory.h - Enhanced Memory Management Declarations */
#ifndef MEMORY_H
#define MEMORY_H
#include <stdint.h>
#include <stddef.h>

/* Initialize memory subsystem */
void memory_init(uint32_t multiboot_info);
/* Kernel memory allocation */
void *kmalloc(size_t size);
void copy_memory(void *dest, void *src, size_t size);
typedef struct
{
    uint32_t entries[1024]; // Simple page directory representation
} PageDirectory;
/* Paging and frame allocation */
typedef struct
{
    uint32_t start;
    uint32_t end;
    uint32_t type;
} MemoryRegion;
typedef struct
{
    MemoryRegion *regions;
    size_t region_count;
    size_t next_region;
} BootInfoFrameAllocator;
typedef struct
{
    void *directory;
} PagingInfo;
uint32_t read_cr3();
PagingInfo init_paging();
BootInfoFrameAllocator init_frame_allocator(MemoryRegion *regions, size_t region_count);
void *allocate_frame(BootInfoFrameAllocator *allocator);
#endif /* MEMORY_H */
