#include "memory.h"

/* Kernel memory pool */
#define KERNEL_MEMORY_SIZE (1024 * 1024 * 16) // 16 MB
static uint8_t kernel_memory[KERNEL_MEMORY_SIZE];
static uint32_t kernel_memory_offset = 0;

// Declare external debugging functions from kernel.c.
extern void debug_print(const char *);
extern void int_to_hex(uint32_t, char *);
extern void int_to_dec(uint32_t, char *);
extern void print_to_screen(const char *);
extern void debug_int(uint32_t);

// Read the CR3 register (32-bit version) and output its value for debugging.
uint32_t read_cr3()
{
    uint32_t value;
    asm volatile("mov %%cr3, %0" : "=r"(value));

    char buffer[16];
    int_to_hex(value, buffer);
    debug_print("DEBUG: CR3 = ");
    debug_print(buffer);

    return value;
}

// Get the active page directory
PageDirectory *active_page_directory()
{
    uint32_t phys_addr = read_cr3() & 0xFFFFF000;

    char buffer[16];
    int_to_hex(phys_addr, buffer);
    debug_print("DEBUG: Active Page Directory = ");
    debug_print(buffer);

    return (PageDirectory *)phys_addr;
}

// Initialize paging
PagingInfo init_paging()
{
    PagingInfo pi;
    pi.directory = active_page_directory();
    debug_print("DEBUG: Paging initialized.");
    return pi;
}

// Frame allocator setup
BootInfoFrameAllocator init_frame_allocator(MemoryRegion *regions, size_t region_count)
{
    BootInfoFrameAllocator allocator;
    allocator.regions = regions;
    allocator.region_count = region_count;
    allocator.next_region = 0;

    debug_print("DEBUG: Frame allocator initialized.");
    return allocator;
}

// Allocate a frame
void *allocate_frame(BootInfoFrameAllocator *allocator)
{
    for (; allocator->next_region < allocator->region_count; allocator->next_region++)
    {
        MemoryRegion region = allocator->regions[allocator->next_region];
        if (region.type == 1)
        { // Usable memory
            void *frame = (void *)region.start;

            uint32_t allocated_space = region.end - region.start;
            char addr_buffer[32];
            char space_buffer[32];
            int_to_hex(region.start, addr_buffer);
            int_to_dec(allocated_space, space_buffer);

            debug_print("DEBUG: Allocated frame at ");
            debug_print(addr_buffer);
            debug_print("DEBUG: Space allocated (bytes): ");
            debug_print(space_buffer);

            allocator->next_region++;
            return frame;
        }
    }
    debug_print("DEBUG: No free frames available!");
    return 0;
}

void memory_init(uint32_t multiboot_info)
{
    // Initialize kernel memory
    for (uint32_t i = 0; i < KERNEL_MEMORY_SIZE; i++)
    {
        kernel_memory[i] = 0;
    }
    kernel_memory_offset = 0;
    debug_print("DEBUG: Kernel memory initialized.");

    // Initialize paging
    PagingInfo paging = init_paging();

    // Assume a dummy memory region for debugging
    MemoryRegion regions[1] = {{0x100000, 0x800000, 1}};
    BootInfoFrameAllocator allocator = init_frame_allocator(regions, 1);
    void *frame = allocate_frame(&allocator);
    if (frame)
    {
        debug_print("DEBUG: Frame allocation succeeded.");
    }
    else
    {
        debug_print("DEBUG: Frame allocation failed!");
    }
}

void *kmalloc(size_t size)
{
    if (kernel_memory_offset + size > KERNEL_MEMORY_SIZE)
    {
        return NULL; // Out of memory
    }
    void *ptr = &kernel_memory[kernel_memory_offset];
    kernel_memory_offset += size;

    if (kernel_memory_offset % 4 != 0)
    {
        kernel_memory_offset += 4 - (kernel_memory_offset % 4);
    }
    return ptr;
}

void copy_memory(void *dest, void *src, size_t size)
{
    uint8_t *d = (uint8_t *)dest;
    uint8_t *s = (uint8_t *)src;
    for (size_t i = 0; i < size; i++)
    {
        d[i] = s[i];
    }
}
