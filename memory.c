#include "memory.h"

// Declare external debugging functions from kernel.c.
extern void debug_print(const char*);
extern void int_to_hex(uint32_t, char*);
extern void int_to_dec(uint32_t, char*);

// Read the CR3 register (32-bit version) and output its value for debugging.
uint32_t read_cr3() {
    uint32_t value;
    asm volatile ("mov %%cr3, %0" : "=r"(value));
    
    char buffer[16];
    int_to_hex(value, buffer);
    debug_print("DEBUG: CR3 = ");
    debug_print(buffer);
    
    return value;
}

// Get the active page directory (masking lower bits) and output its value.
PageDirectory* active_page_directory() {
    uint32_t phys_addr = read_cr3() & 0xFFFFF000;
    
    char buffer[16];
    int_to_hex(phys_addr, buffer);
    debug_print("DEBUG: Active Page Directory = ");
    debug_print(buffer);
    
    return (PageDirectory*) phys_addr;
}

// Initialize paging (for a simple identity-mapped example)
PagingInfo init_paging() {
    PagingInfo pi;
    pi.directory = active_page_directory();
    
    debug_print("DEBUG: Paging initialized.");
    return pi;
}

// Initialize a very simple frame allocator with a given memory map.
BootInfoFrameAllocator init_frame_allocator(MemoryRegion* regions, size_t region_count) {
    BootInfoFrameAllocator allocator;
    allocator.regions = regions;
    allocator.region_count = region_count;
    allocator.next_region = 0;
    
    debug_print("DEBUG: Frame allocator initialized.");
    return allocator;
}

// Allocate a frame from the memory regions; for simplicity, return the start address
// and print the allocated amount of space.
void* allocate_frame(BootInfoFrameAllocator* allocator) {
    for (; allocator->next_region < allocator->region_count; allocator->next_region++) {
        MemoryRegion region = allocator->regions[allocator->next_region];
        if (region.type == 1) {  // Usable memory
            void* frame = (void*) region.start;
            
            // Calculate the allocated space (here, the entire region size)
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

// Kernel entry point. The multiboot_info pointer is passed from boot.asm (via ebx).
void memory_init(uint32_t multiboot_info) {
    // debug_print("DEBUG: Entering kernel_main.");
    
    // Initialize paging (dummy example).
    PagingInfo paging = init_paging();
    
    // For debugging, assume one dummy memory region: usable memory from 0x100000 to 0x200000.
    MemoryRegion regions[1] = { {0x100000, 0x800000, 1} };
    BootInfoFrameAllocator allocator = init_frame_allocator(regions, 1);
    
    void* frame = allocate_frame(&allocator);
    if (frame) {
        debug_print("DEBUG: Frame allocation succeeded.");
    } else {
        debug_print("DEBUG: Frame allocation failed!");
    }
    
    // Kernel would continue further initialization here...
}
