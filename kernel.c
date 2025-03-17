#include <stdint.h>
#include <stddef.h>
#include "process.h"  // Added for process management integration
#include "memory.h"

// Global variable to track the current line (each line is 80 characters)
static uint16_t vga_line = 0;

// Write a string to the VGA text memory on a new line.
void print_to_screen(const char* message) {
    char *video_memory = (char*) 0xB8000;
    int i = 0;
    int offset = vga_line * 80 * 2;  // 80 characters per line, 2 bytes per character
    while (message[i] != '\0') {
        video_memory[offset + i*2] = message[i];  // Character
        video_memory[offset + i*2 + 1] = 0x07;      // White text on black background
        i++;
    }
    vga_line++;
}

// Convert a 32-bit unsigned integer to a hexadecimal string.
// The buffer must be at least 11 bytes long: "0x" + 8 hex digits + '\0'
void int_to_hex(uint32_t num, char *buffer) {
    const char hex_chars[] = "0123456789ABCDEF";
    buffer[0] = '0';
    buffer[1] = 'x';
    for (int i = 0; i < 8; i++) {
         buffer[i+2] = hex_chars[(num >> ((7 - i) * 4)) & 0xF];
    }
    buffer[10] = '\0';
}

// Convert a 32-bit unsigned integer to a decimal string.
// The buffer should be large enough to hold the result (suggest at least 16 bytes).
void int_to_dec(uint32_t num, char *buffer) {
    char temp[16];
    int pos = 0;
    
    if (num == 0) {
        buffer[0] = '0';
        buffer[1] = '\0';
        return;
    }
    
    // Convert number to digits in reverse order.
    while (num > 0 && pos < 15) {
        temp[pos++] = '0' + (num % 10);
        num /= 10;
    }
    
    // Reverse the digits into the output buffer.
    int i;
    for (i = 0; i < pos; i++) {
        buffer[i] = temp[pos - i - 1];
    }
    buffer[i] = '\0';
}

// A simple wrapper for debugging output.
void debug_print(const char* msg) {
    print_to_screen(msg);
}


// ============================================
// New Changes: Process Management Integration
// ============================================

// Forward declaration for the process management test function.
// This function should be defined in process_test.c and will create dummy processes,
// schedule them, and print their output to the screen.
extern void process_test(void);

// The kernel entry point called from your bootloader (e.g., boot.asm).
// This function initializes process management and then runs the process test.
void kernel_main(uint32_t multiboot_info) {
    print_to_screen("DEBUG: Entering kernel_main.\n");

    // Initialize memory (call memory_init from memory.c)
    memory_init(multiboot_info);

    // Initialize process management system.
    init_process_management();

    print_to_screen("DEBUG: Starting process management test.\n");
    // Execute the process management test (dummy processes are created and scheduled).
    process_test();

    print_to_screen("DEBUG: Kernel execution complete.\n");

    // Halt the CPU indefinitely.
    while (1) {
        asm volatile ("hlt");
    }
}