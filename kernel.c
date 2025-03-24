#include <stdint.h>
/* Multiboot Header - must be in the first 8KB of the binary */
__attribute__((section(".multiboot")))
__attribute__((used)) static const struct
{
    uint32_t magic;
    uint32_t flags;
    uint32_t checksum;
} multiboot_header = {
    0x1BADB002,                // Multiboot magic number.
    0x00010003,                // Flags: align modules on page boundaries + memory info provided.
    -(0x1BADB002 + 0x00010003) // Checksum: so that magic + flags + checksum = 0.
};
#include <stddef.h>
#include "gdt.h"
#include "process.h"
#include "memory.h"
#include "serial.h"
#include "keyboard.h"
#include "idt.h"
#include "pic.h"
#include "interrupts.h"

void itoa(int n, char *str)
{
    int i = 0;
    int sign = n;
    if (sign < 0)
    {
        n = -n;
    }
    do
    {
        str[i++] = n % 10 + '0';
        n /= 10;
    } while (n > 0);
    if (sign < 0)
    {
        str[i++] = '-';
    }
    str[i] = '\0';
    int j = 0;
    char temp;
    for (j = 0; j < i / 2; j++)
    {
        temp = str[j];
        str[j] = str[i - j - 1];
        str[i - j - 1] = temp;
    }
}

// We'll keep the existing VGA-based print_to_screen in case you still want
// minimal text output on the QEMU display. But all "debug_print" calls will
// go to the serial console now.

// Global variable to track the current line (each line is 80 characters)
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEMORY ((volatile char *)0xB8000)

/* Track our current position in the text buffer */
static uint16_t vga_row = 0;
static uint16_t vga_col = 0;

/* Scrolls the screen up by one line when the screen is full */
void scroll_screen(void)
{
    volatile char *video = VGA_MEMORY;

    // Shift all rows up by 1
    for (int row = 1; row < VGA_HEIGHT; row++)
    {
        for (int col = 0; col < VGA_WIDTH; col++)
        {
            int from = (row * VGA_WIDTH + col) * 2;
            int to = ((row - 1) * VGA_WIDTH + col) * 2;
            video[to] = video[from];
            video[to + 1] = video[from + 1];
        }
    }

    // Clear the last row (set it to spaces)
    int last_row_offset = (VGA_HEIGHT - 1) * VGA_WIDTH * 2;
    for (int col = 0; col < VGA_WIDTH; col++)
    {
        video[last_row_offset + col * 2] = ' ';      // Blank character
        video[last_row_offset + col * 2 + 1] = 0x07; // White on black
    }

    // Reset cursor to the last row, column 0
    vga_row = VGA_HEIGHT - 1;
    vga_col = 0;
}

/* Low-level function to place a single character at the current cursor position */
void putchar(char c)
{
    volatile char *video = VGA_MEMORY;

    // If we encounter a newline, move to the next row, reset col
    if (c == '\n')
    {
        vga_row++;
        vga_col = 0;
    }
    else
    {
        int offset = (vga_row * VGA_WIDTH + vga_col) * 2;
        video[offset] = c;        // Character
        video[offset + 1] = 0x07; // Light grey on black
        vga_col++;

        // If we reach end of line, move to next row
        if (vga_col >= VGA_WIDTH)
        {
            vga_row++;
            vga_col = 0;
        }
    }

    // If we exceed the last row, scroll the screen
    if (vga_row >= VGA_HEIGHT)
    {
        scroll_screen();
    }
}

/* Higher-level function to print an entire string */
void print_to_screen(const char *message)
{
    while (*message)
    {
        putchar(*message);
        message++;
    }
}
// Converts a 32-bit number to a hexadecimal string and prints it
void debug_int(uint32_t val)
{
    char buffer[16]; // "0x" + 8 hex digits + '\0'
    itoa(val, buffer);

    serial_print(buffer);
    serial_print("\r\n"); // Newline for better readability
}

// Convert a 32-bit unsigned integer to a hexadecimal string.
// The buffer must be at least 11 bytes long: "0x" + 8 hex digits + '\0'
void int_to_hex(uint32_t num, char *buffer)
{
    const char hex_chars[] = "0123456789ABCDEF";
    buffer[0] = '0';
    buffer[1] = 'x';
    for (int i = 0; i < 8; i++)
    {
        buffer[i + 2] = hex_chars[(num >> ((7 - i) * 4)) & 0xF];
    }
    buffer[10] = '\0';
}

// Convert a 32-bit unsigned integer to a decimal string.
void int_to_dec(uint32_t num, char *buffer)
{
    char temp[16];
    int pos = 0;

    if (num == 0)
    {
        buffer[0] = '0';
        buffer[1] = '\0';
        return;
    }

    // Convert number to digits in reverse order.
    while (num > 0 && pos < 15)
    {
        temp[pos++] = '0' + (num % 10);
        num /= 10;
    }

    // Reverse the digits into the output buffer.
    int i;
    for (i = 0; i < pos; i++)
    {
        buffer[i] = temp[pos - i - 1];
    }
    buffer[i] = '\0';
}

// A simple wrapper for debugging output.
// Instead of using VGA, we'll redirect to serial.
void debug_print(const char *msg)
{
    serial_print(msg);    // Print to serial console
    serial_print("\r\n"); // Add newline for clarity
}

// Forward declaration for the process management test function.
extern void process_test(void);

void int_to_str(int value, char *buffer)
{
    // Assuming value is non-negative and buffer is large enough.
    int i = 0;
    if (value == 0)
    {
        buffer[i++] = '0';
        buffer[i] = '\0';
        return;
    }
    // Temporary buffer to hold digits in reverse order.
    char temp[10];
    int j = 0;
    while (value)
    {
        temp[j++] = '0' + (value % 10);
        value /= 10;
    }
    // Reverse the digits into buffer.
    while (j > 0)
    {
        buffer[i++] = temp[--j];
    }
    buffer[i] = '\0';
}

// The kernel entry point called from your bootloader.
void kernel_main(uint32_t multiboot_info)
{
    // Initialize serial for debug messages
    serial_init();
    debug_print("DEBUG: Entering kernel_main.");
    for (int i = 1; i <= 30; i++)
    {
        print_to_screen("Debug: line ");
        char num[10];
        int_to_str(i, num);
        print_to_screen(num);
        print_to_screen("\n");
    }

    // Initialize memory
    memory_init(multiboot_info);
    debug_print("DEBUG: Memory initialized.");

    // Main loop.
    debug_print("DEBUG: Kernel execution complete.");
    gdt_install();
    print_to_screen("DEBUG: GDT installed.\n");

    // Remap PIC and install IDT.
    pic_remap();
    idt_install();
    // Install IRQ handlers (for hardware interrupts)
    irq_install();
    print_to_screen("DEBUG: IDT and IRQ handlers installed.\n");
    // Initialize process management system

    // Initialize keyboard.
    init_keyboard();
    print_to_screen("DEBUG: Keyboard initialized. Press keys!\n");

    // Enable interrupts.
    asm volatile("sti");
    // Initialize system calls
    //    init_syscalls();
    //    debug_print("DEBUG: System calls initialized.");

    init_process_management();
    debug_print("DEBUG: Process management initialized.");

    debug_print("DEBUG: Starting process management test.");
    // Execute the process management test
    process_test();

    while (1)
    {
        asm volatile("hlt");
    }
}
