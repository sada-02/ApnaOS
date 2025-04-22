#include "keyboard.h"
#include "io.h"
#include "../interrupts/interrupts.h"
#include "string.h"

extern void debug_print(const char* messe);
extern void print_to_screen(const char* message);

#define KBD_DATA_PORT 0x60
#define KBD_STATUS_PORT 0x64
#define KBD_SCANCODE_RELEASE 0x80

static char input_buffer[BUFFER_SIZE];
volatile int input_ready = 0;
volatile char input_line[BUFFER_SIZE];
static int buffer_index = 0;

static const char scancode_to_ascii[128] = {
    0, 27, '1', '2', '3', '4', '5', '6',
    '7', '8', '9', '0', '-', '=', '\b', '\t',
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i',
    'o', 'p', '[', ']', '\n', 0, 'a', 's',
    'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',
    '\'', '`', 0, '\\', 'z', 'x', 'c', 'v',
    'b', 'n', 'm', ',', '.', '/', 0, '*',
    0, ' ', 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static const char scancode_to_ascii_shift[128] = {
    0, 27, '!', '@', '#', '$', '%', '^',
    '&', '*', '(', ')', '_', '+', '\b', '\t',
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I',
    'O', 'P', '{', '}', '\n', 0, 'A', 'S',
    'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',
    '"', '~', 0, '|', 'Z', 'X', 'C', 'V',
    'B', 'N', 'M', '<', '>', '?', 0, '*',
    0, ' ', 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

int shift_pressed = 0;

void keyboard_handler() {
    uint8_t scancode = inb(KBD_DATA_PORT);

    if (scancode == 0x2A || scancode == 0x36) {
        shift_pressed = 1;
        return;
    } else if (scancode == 0xAA || scancode == 0xB6) {
        shift_pressed = 0;
        return;
    }

    if (!(scancode & KBD_SCANCODE_RELEASE)) {
        char key;
        if (shift_pressed) {
            key = scancode_to_ascii_shift[scancode];
        } else {
            key = scancode_to_ascii[scancode];
        }

        if (key) {
            if (key == '\n') {
                input_buffer[buffer_index] = '\0';
                int i = 0;
                while (input_buffer[i] != '\0' && i < BUFFER_SIZE) {
                    input_line[i] = input_buffer[i];
                    i++;
                }
                input_line[i] = '\0';
                input_ready = 1;
                print_to_screen("\n");
                buffer_index = 0;
            } else if (key == '\b') {
                if (buffer_index > 0) {
                    buffer_index--;
                    print_to_screen("\b \b");
                }
            } else {
                if (buffer_index < BUFFER_SIZE - 1) {
                    input_buffer[buffer_index++] = key;
                    char buf[2] = {key, '\0'};
                    print_to_screen(buf);
                }
            }
        }
    }
}

void init_keyboard() {
    register_interrupt_handler(33, keyboard_handler);
    print_to_screen("DEBUG: Keyboard interrupt handler registered.\n");
}
