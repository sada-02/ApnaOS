#include <stdint.h>
#include <stddef.h>

__attribute__((section(".multiboot")))
__attribute__((used)) static const struct {
    uint32_t magic;
    uint32_t flags;
    uint32_t checksum;
} multiboot_header = {
    0x1BADB002,
    0x00010003,
    -(0x1BADB002 + 0x00010003)
};

#include "gdt.h"
#include "process.h"
#include "memory.h"
#include "serial.h"
#include "keyboard.h"
#include "idt.h"
#include "pic.h"
#include "interrupts.h"
#include "filesystem.h"
#include "string.h"

extern void dummy_process_1(void);
extern void dummy_process_2(void);
extern void dummy_process_3(void);

void itoa(int n, char *str)
{
    int i = 0, sign = n;
    if(sign < 0) n = -n;
    do { str[i++] = n % 10 + '0'; n /= 10; } while(n > 0);
    if(sign < 0) str[i++] = '-';
    str[i] = '\0';
    int j = 0;
    char temp;
    for(j = 0; j < i / 2; j++){
        temp = str[j];
        str[j] = str[i - j - 1];
        str[i - j - 1] = temp;
    }
}

void int_to_hex(uint32_t num, char *buffer)
{
    const char hex_chars[] = "0123456789ABCDEF";
    buffer[0] = '0'; buffer[1] = 'x';
    for (int i = 0; i < 8; i++)
        buffer[i + 2] = hex_chars[(num >> ((7 - i) * 4)) & 0xF];
    buffer[10] = '\0';
}

void int_to_dec(uint32_t num, char *buffer)
{
    char temp[16];
    int pos = 0;
    if(num == 0){ buffer[0] = '0'; buffer[1] = '\0'; return; }
    while(num > 0 && pos < 15){ temp[pos++] = '0' + (num % 10); num /= 10; }
    int i;
    for(i = 0; i < pos; i++) buffer[i] = temp[pos - i - 1];
    buffer[i] = '\0';
}

void int_to_str(int value, char *buffer)
{
    int i = 0;
    if(value == 0){ buffer[i++] = '0'; buffer[i] = '\0'; return; }
    char temp[10];
    int j = 0;
    while(value){ temp[j++] = '0' + (value % 10); value /= 10; }
    while(j > 0){ buffer[i++] = temp[--j]; }
    buffer[i] = '\0';
}

void debug_print(const char *msg)
{
    serial_print(msg);
    serial_print("\r\n");
}

void debug_int(uint32_t val)
{
    char buffer[16];
    int_to_hex(val, buffer);
    debug_print(buffer);
}

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEMORY ((volatile char *)0xB8000)

static uint16_t vga_row = 0;
static uint16_t vga_col = 0;

void scroll_screen(void)
{
    volatile char *video = VGA_MEMORY;
    for (int row = 1; row < VGA_HEIGHT; row++){
        for (int col = 0; col < VGA_WIDTH; col++){
            int from = (row * VGA_WIDTH + col) * 2;
            int to = ((row - 1) * VGA_WIDTH + col) * 2;
            video[to] = video[from];
            video[to+1] = video[from+1];
        }
    }
    int last_row = (VGA_HEIGHT - 1) * VGA_WIDTH * 2;
    for (int col = 0; col < VGA_WIDTH; col++){
        video[last_row + col*2] = ' ';
        video[last_row + col*2 + 1] = 0x07;
    }
    vga_row = VGA_HEIGHT - 1;
    vga_col = 0;
}

void putchar(char c)
{
    volatile char *video = VGA_MEMORY;
    if(c == '\n'){
        vga_row++;
        vga_col = 0;
    } else {
        int offset = (vga_row * VGA_WIDTH + vga_col) * 2;
        video[offset] = c;
        video[offset+1] = 0x07;
        vga_col++;
        if(vga_col >= VGA_WIDTH){ vga_row++; vga_col = 0; }
    }
    if(vga_row >= VGA_HEIGHT) scroll_screen();
}

void print_to_screen(const char *message)
{
    while(*message){ putchar(*message); message++; }
}

#define MAX_INPUT_LENGTH 128

typedef struct {
    const char *name;
    void (*func)(void);
} ProcessCommand;

ProcessCommand process_commands[] = {
    {"dummy1", dummy_process_1},
    {"dummy2", dummy_process_2},
    {"dummy3", dummy_process_3}
};
const int num_process_commands = sizeof(process_commands)/sizeof(ProcessCommand);

extern volatile int input_ready;
extern volatile char input_line[BUFFER_SIZE];

void read_line(char *buffer, int max_length)
{
    while(!input_ready){ asm volatile("hlt"); }
    int i = 0;
    while(input_line[i] != '\0' && i < max_length-1){ buffer[i] = input_line[i]; i++; }
    buffer[i] = '\0';
    input_ready = 0;
}

void cli_loop()
{
    char input[MAX_INPUT_LENGTH];
    while(1){
        print_to_screen("CLI> ");
        read_line(input, MAX_INPUT_LENGTH);
        if(strcmp(input, "exit") == 0){ print_to_screen("Exiting CLI...\n"); break; }
        int found = 0;
        for (int i = 0; i < num_process_commands; i++){
            if(strcmp(input, process_commands[i].name) == 0){
                print_to_screen("Process found. Executing process...\n");
                process_commands[i].func();
                found = 1;
                break;
            }
        }
        if(!found){ print_to_screen("Error: Process not found.\n"); }
    }
}


void test_filesystem() {
    print_to_screen("Testing filesystem...\n");

    // Create a file
    const char *filename = "testfile.txt";
    read_line(filename, MAX_INPUT_LENGTH);
    int inode = create_file(filename);
    if (inode == -1) {
        print_to_screen("Failed to create file.\n");
        return;
    }
    print_to_screen("File created successfully.\n");
    
    // Write to the file
    const char *data = "Hello, ApnaOS!";
    read_line(data, MAX_INPUT_LENGTH);
    int bytes_written = write_file(inode + 1, data, strlen(data));
    if (bytes_written == -1) {
        print_to_screen("Failed to write to file.\n");
        return;
    }
    print_to_screen("Data written to file successfully.\n");

    // Read from the file
    char buffer[128];
    int bytes_read = read_file(inode + 1, buffer, sizeof(buffer));
    if (bytes_read == -1) {
        print_to_screen("Failed to read from file.\n");
        return;
    }

    buffer[bytes_read] = '\0'; // Null-terminate the string
    print_to_screen("Data read from file: ");
    print_to_screen(buffer);
    print_to_screen("\n");

    // Delete the file
    if (delete_file(filename) == -1) {
        print_to_screen("Failed to delete file.\n");
        return;
    }
    print_to_screen("File deleted successfully.\n");

    print_to_screen("Filesystem test completed.\n");
}

void kernel_main(uint32_t multiboot_info)
{
    serial_init();
    debug_print("DEBUG: Entering kernel_main.");
    for (int i = 1; i <= 30; i++){
        print_to_screen("Debug: line ");
        char num[10];
        int_to_str(i, num);
        print_to_screen(num);
        print_to_screen("\n");
    }
    memory_init(multiboot_info);
    debug_print("DEBUG: Memory initialized.");

    create_file_system();
    debug_print("DEBUG: Filesystem initialized.");
    
    gdt_install();
    print_to_screen("DEBUG: GDT installed.\n");
    pic_remap();
    idt_install();
    irq_install();
    print_to_screen("DEBUG: IDT and IRQ handlers installed.\n");
    init_keyboard();
    
    print_to_screen("DEBUG: Keyboard initialized. Press keys!\n");
    asm volatile("sti");

    // Initialize system calls
    init_syscalls();
    debug_print("DEBUG: System calls initialized.");
    syscall_test();
    
    // init_process_management();
    // debug_print("DEBUG: Process management initialized.");
    // process_test();

    test_filesystem();

    // cli_loop();
    
    // asm volatile("cli");
    print_to_screen("Kernel execution terminated.\n");
    while(1){ asm volatile("hlt"); }
}
