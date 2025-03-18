#include <stdint.h>
#include <stddef.h>
#include "process.h"
#include "memory.h"
#include "serial.h"

// We'll keep the existing VGA-based print_to_screen in case you still want
// minimal text output on the QEMU display. But all "debug_print" calls will
// go to the serial console now.

// Global variable to track the current line (each line is 80 characters)
static uint16_t vga_line = 0;

// Write a string to the VGA text memory on a new line (optional).
void print_to_screen(const char* message) {
    char *video_memory = (char*) 0xB8000;
    int i = 0;
    int offset = vga_line * 80 * 2;  // 80 characters per line, 2 bytes per character
    while (message[i] != '\0') {
        video_memory[offset + i*2] = message[i];  // Character
        video_memory[offset + i*2 + 1] = 0x07;    // White text on black background
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
// Instead of using VGA, we'll redirect to serial.
void debug_print(const char* msg) {
    serial_print(msg);         // Print to serial console
    serial_print("\r\n");      // Add newline for clarity
}

// Forward declaration for the process management test function.
extern void process_test(void);

// The kernel entry point called from your bootloader.
void kernel_main(uint32_t multiboot_info) {
    // Initialize serial for debug messages
    serial_init();
    debug_print("DEBUG: Entering kernel_main.");

    // Initialize memory
    memory_init(multiboot_info);
    debug_print("DEBUG: Memory initialized.");

    // Initialize process management system.
    init_process_management();
    debug_print("DEBUG: Process management initialized.");

    debug_print("DEBUG: Starting process management test.");
    // Execute the process management test
    process_test();

    debug_print("DEBUG: Kernel execution complete.");

    // Halt the CPU indefinitely.
    while (1) {
        asm volatile ("hlt");
    }
}
