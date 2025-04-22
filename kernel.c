#include <stdint.h>
#include <stddef.h>
#include <vmm.h>

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

#include "keyboard/gdt.h"
#include "keyboard/keyboard.h"
#include "keyboard/string.h"

#include "process/process.h"
#include "process/syscall.h"   

#include "memory/memory.h"

#include "interrupts/idt.h"
#include "interrupts/pic.h"
#include "interrupts/interrupts.h"

#include "filesystem/filesystem.h" 

#include "serial.h"

extern void dummy_process_1(void);
extern void dummy_process_2(void);
extern void dummy_process_3(void);
extern void syscall_test(void);
extern void process_test(void);

int atoi(const char *s) {
    int num = 0;
    int sign = 1;
    if (*s == '-') {
        sign = -1;
        s++;
    }
    while (*s) {
        if (*s >= '0' && *s <= '9') {
            num = num * 10 + (*s - '0');
        } else {
            break;
        }
        s++;
    }
    return sign * num;
}

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

if (vmm_init()) {
        print("VMM initialized.\n"); //  added for VMM
    } else {
        print("VMM failed to initialize.\n");
    }

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

void cli_loop(void) {
    char input[MAX_INPUT_LENGTH];

    while (1) {
        print_to_screen("CLI> ");
        read_line(input, MAX_INPUT_LENGTH);
        char *token1 = strtok(input, " \t");
        if (!token1) {
            continue;
        }
        if (strcmp(token1, "exit") == 0) {
            print_to_screen("Exiting kernel CLI...\n");
            break;
        }
        else if (strcmp(token1, "process") == 0) {
            char *token2 = strtok(NULL, " \t");
            if (!token2) {
                print_to_screen("Usage: process <dummy1|dummy2|dummy3|syscall test|process test|start> [priority]\n");
                continue;
            }
            if (strcmp(token2, "start") == 0) {
                print_to_screen("Starting scheduled processes...\n");
                schedule();
                continue;
            }
            
            int priority = 1;
            char *token3 = strtok(NULL, " \t"); 
            char *token4 = strtok(NULL, " \t");

            if (strcmp(token2, "syscall") == 0 && token3 && strcmp(token3, "test") == 0) {
                if (token4) priority = atoi(token4);
                print_to_screen("Queueing syscall_test process...\n");
        
                uint32_t *stack_top = (uint32_t *) kmalloc(4096);
                if (!stack_top) {
                    print_to_screen("Error: Unable to allocate process stack.\n");
                    continue;
                }
        
                create_process(
                    get_new_pid(),
                    (uint32_t *) syscall_test,
                    stack_top + (4096 / sizeof(uint32_t)),
                    priority, 1, 2
                );
                continue;
            }

            if (strcmp(token2, "process") == 0 && token3 && strcmp(token3, "test") == 0) {
                if (token4) priority = atoi(token4);
                print_to_screen("Queueing process_test process...\n");
        
                uint32_t *stack_top = (uint32_t *) kmalloc(4096);
                if (!stack_top) {
                    print_to_screen("Error: Unable to allocate process stack.\n");
                    continue;
                }
        
                create_process(
                    get_new_pid(),
                    (uint32_t *) process_test,
                    stack_top + (4096 / sizeof(uint32_t)),
                    priority, 1, 2
                );
                continue;
            }

            if (token3) {
                priority = atoi(token3);
            }
            
            int found = 0;
            for (int i = 0; i < num_process_commands; i++) {
                if (strcmp(token2, process_commands[i].name) == 0) {
                    print_to_screen("Queueing process...\n");
                    uint32_t *stack_top = (uint32_t *) kmalloc(4096);
                    if (!stack_top) {
                        print_to_screen("Error: Unable to allocate process stack.\n");
                        found = 1;
                        break;
                    }
                    create_process(
                        get_new_pid(),
                        (uint32_t *) process_commands[i].func,
                        stack_top + (4096 / sizeof(uint32_t)),
                        priority,
                        1, 2
                    );
                    found = 1;
                    break;
                }
            }
            if (!found) {
                print_to_screen("Error: Unknown process name.\n");
            }
        }
        else if (strcmp(token1, "mkfile") == 0) {
            char *token2 = strtok(NULL, " \t");
            if (!token2) {
                print_to_screen("Usage: mkfile <filename>\n");
                continue;
            }
            if (create_file(token2) == -1) {
                print_to_screen("Error: Failed to create file.\n");
            } else {
                print_to_screen("File created successfully.\n");
            }
        }
        else if (strcmp(token1, "readfile") == 0) {
            char *token2 = strtok(NULL, " \t");
            if (!token2) {
                print_to_screen("Usage: readfile <filename>\n");
                continue;
            }
            char buffer[128];
            int bytes_read = read_file(token2, buffer, sizeof(buffer));
            if (bytes_read == -1) {
                print_to_screen("Error: Failed to read file.\n");
            } else {
                buffer[bytes_read] = '\0'; // Null-terminate the string
                print_to_screen("File content: ");
                print_to_screen(buffer);
                print_to_screen("\n");
            }
        }
        else if (strcmp(token1, "writefile") == 0) {
            char *token2 = strtok(NULL, " \t");
            char *token3 = strtok(NULL, "\n"); // Remaining input as data
            if (!token2 || !token3) {
                print_to_screen("Usage: writefile <filename> <data>\n");
                continue;
            }
            if (write_file(token2, token3, strlen(token3)) == -1) {
                print_to_screen("Error: Failed to write to file.\n");
            } else {
                print_to_screen("Data written to file successfully.\n");
            }
        }
        else if (strcmp(token1, "appendfile") == 0) {
            char *token2 = strtok(NULL, " \t");
            char *token3 = strtok(NULL, "\n"); // Remaining input as data
            if (!token2 || !token3) {
                print_to_screen("Usage: appendfile <filename> <data>\n");
                continue;
            }
            if (append_to_file(token2, token3, strlen(token3)) == -1) {
                print_to_screen("Error: Failed to append to file.\n");
            } else {
                print_to_screen("Data appended to file successfully.\n");
            }
        }
        else if (strcmp(token1, "rmfile") == 0) {
            char *token2 = strtok(NULL, " \t");
            if (!token2) {
                print_to_screen("Usage: rmfile <filename>\n");
                continue;
            }
            if (delete_file(token2) == -1) {
                print_to_screen("Error: Failed to delete file.\n");
            } else {
                print_to_screen("File deleted successfully.\n");
            }
        }
        else if (strcmp(token1, "ls") == 0) {
            list_files();
        }
        else {
            print_to_screen("Unknown command. Use 'process', 'mkfile', 'readfile', 'writefile', 'appendfile', 'rmfile', 'ls', or 'exit'.\n");
        }
    }
}

void test_filesystem() {
    print_to_screen("Testing filesystem...\n");

    const char *filename = "testfile.txt";
    read_line((char *)filename, MAX_INPUT_LENGTH);
    int inode = create_file(filename);
    if (inode == -1) {
        print_to_screen("Failed to create file.\n");
        return;
    }
    print_to_screen("File created successfully.\n");
    
    const char *data = "Hello, ApnaOS!";
    read_line((char *)data, MAX_INPUT_LENGTH);
    int bytes_written = write_file(filename, data, strlen(data));
    if (bytes_written == -1) {
        print_to_screen("Failed to write to file.\n");
        return;
    }
    print_to_screen("Data written to file successfully.\n");
    char buffer[128];
    int bytes_read = read_file(filename, buffer, sizeof(buffer));
    if (bytes_read == -1) {
        print_to_screen("Failed to read from file.\n");
        return;
    }
    buffer[bytes_read] = '\0'; 
    print_to_screen("Data read from file: ");
    print_to_screen(buffer);
    print_to_screen("\n");
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
    init_syscalls();
    debug_print("DEBUG: System calls initialized.");

    init_process_management();
    debug_print("DEBUG: Process management initialized.");
    cli_loop();
    
    asm volatile("cli");
    print_to_screen("Kernel execution terminated.\n");
    while(1){ asm volatile("hlt"); }
}
