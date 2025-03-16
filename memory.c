#include "memory.h"

// Declare external debugging functions from kernel.c.
extern void print_to_screen(const char*);
extern void int_to_hex(uint32_t, char*);
extern void int_to_dec(uint32_t, char*);

// Read the CR3 register (32-bit version) and output its value for debugging.
uint32_t read_cr3() {
    uint32_t value;
    asm volatile ("mov %%cr3, %0" : "=r"(value));
    
    char buffer[16];
    int_to_hex(value, buffer);
    print_to_screen("DEBUG: CR3 = ");
    print_to_screen(buffer);
    
    return value;
}

// Get the active page directory (masking lower bits) and output its value.
PageDirectory* active_page_directory() {
    uint32_t phys_addr = read_cr3() & 0xFFFFF000;
    
    char buffer[16];
    int_to_hex(phys_addr, buffer);
    print_to_screen("DEBUG: Active Page Directory = ");
    print_to_screen(buffer);
    
    return (PageDirectory*) phys_addr;
}

// Initialize paging (for a simple identity-mapped example)
PagingInfo init_paging() {
    PagingInfo pi;
    pi.directory = active_page_directory();
    
    print_to_screen("DEBUG: Paging initialized.");
    return pi;
}

// Initialize a very simple frame allocator with a given memory map.
BootInfoFrameAllocator init_frame_allocator(MemoryRegion* regions, size_t region_count) {
    BootInfoFrameAllocator allocator;
    allocator.regions = regions;
    allocator.region_count = region_count;
    allocator.next_region = 0;
    
    print_to_screen("DEBUG: Frame allocator initialized.");
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
            char addr_buffer[16];
            char space_buffer[16];
            int_to_hex(region.start, addr_buffer);
            int_to_dec(allocated_space, space_buffer);
            
            print_to_screen("DEBUG: Allocated frame at ");
            print_to_screen(addr_buffer);
            print_to_screen("DEBUG: Space allocated (bytes): ");
            print_to_screen(space_buffer);
            
            allocator->next_region++;
            return frame;
        }
    }
    print_to_screen("DEBUG: No free frames available!");
    return 0;
}

// Kernel entry point. The multiboot_info pointer is passed from boot.asm (via ebx).
void kernel_main(uint32_t multiboot_info) {
    print_to_screen("DEBUG: Entering kernel_main.");
    
    // Initialize paging (dummy example).
    PagingInfo paging = init_paging();
    
    // For debugging, assume one dummy memory region: usable memory from 0x100000 to 0x200000.
    MemoryRegion regions[1] = { {0x100000, 0x200000, 1} };
    BootInfoFrameAllocator allocator = init_frame_allocator(regions, 1);
    
    void* frame = allocate_frame(&allocator);
    if (frame) {
        print_to_screen("DEBUG: Frame allocation succeeded.");
    } else {
        print_to_screen("DEBUG: Frame allocation failed!");
    }
    
    // Kernel would continue further initialization here...
}
