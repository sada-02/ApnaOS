#include "keyboard.h"
#include "io.h"
#include "../interrupts/interrupts.h"
#include "string.h"

extern void debug_print(const char* messe);
extern void print_to_screen(const char* message);
extern void enter_scroll_mode(void);
extern void exit_scroll_mode(void);
extern void scroll_up(void);
extern void scroll_down(void);
extern volatile int in_scroll_mode;

#define KBD_DATA_PORT 0x60
#define KBD_STATUS_PORT 0x64
#define KBD_SCANCODE_RELEASE 0x80
#define HISTORY_SIZE 20

static char input_buffer[BUFFER_SIZE];
volatile int input_ready = 0;
volatile char input_line[BUFFER_SIZE];
static int buffer_index = 0;
static int extended_code = 0;

static char command_history[HISTORY_SIZE][BUFFER_SIZE];
static int history_count = 0;
static int history_index = -1;
static int browsing_history = 0;

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
int caps_lock_on = 0;

void clear_current_input() {
    while (buffer_index > 0) {
        buffer_index--;
        print_to_screen("\b \b");
    }
}

void restore_from_history(int hist_idx) {
    clear_current_input();
    int i = 0;
    while (command_history[hist_idx][i] != '\0' && i < BUFFER_SIZE - 1) {
        input_buffer[i] = command_history[hist_idx][i];
        char buf[2] = {command_history[hist_idx][i], '\0'};
        print_to_screen(buf);
        i++;
    }
    buffer_index = i;
    input_buffer[buffer_index] = '\0';
}

void keyboard_handler() {
    uint8_t scancode = inb(KBD_DATA_PORT);

    if (scancode == 0x2A || scancode == 0x36) {
        shift_pressed = 1;
        return;
    } else if (scancode == 0xAA || scancode == 0xB6) {
        shift_pressed = 0;
        return;
    }
    
    if ((scancode & KBD_SCANCODE_RELEASE) && extended_code) {
        extended_code = 0;
        return;
    }

    if (scancode == 0x3A) {
        caps_lock_on = !caps_lock_on;
        return;
    }
    
    if (scancode == 0xE0) {
        extended_code = 1;
        return;
    }

    if (!(scancode & KBD_SCANCODE_RELEASE)) {
        if (extended_code) {
            extended_code = 0;
            
            if (scancode == 0x49) {
                if (!in_scroll_mode) {
                    enter_scroll_mode();
                } else {
                    scroll_up();
                }
                return;
            }
            
            if (scancode == 0x51) {
                scroll_down();
                return;
            }
            return;
        }
        
        if (scancode == 0x01) {
            if (in_scroll_mode) {
                exit_scroll_mode();
                return;
            }
        }
        
        if (scancode == 0x48) {
            if (history_count > 0) {
                if (!browsing_history) {
                    history_index = history_count - 1;
                    browsing_history = 1;
                } else if (history_index > 0) {
                    history_index--;
                }
                restore_from_history(history_index);
            }
            return;
        }
        
        if (scancode == 0x50) {
            if (browsing_history) {
                if (history_index < history_count - 1) {
                    history_index++;
                    restore_from_history(history_index);
                } else {
                    clear_current_input();
                    browsing_history = 0;
                }
            }
            return;
        }

        char key = 0;

        if (shift_pressed) {
            key = scancode_to_ascii_shift[scancode];
        } else {
            key = scancode_to_ascii[scancode];
        }

        if (key >= 'a' && key <= 'z') {
            if (caps_lock_on ^ shift_pressed) {
                key = key - 32;
            }
        }

        if (key) {
            browsing_history = 0;
            
            if (key == '\n') {
                input_buffer[buffer_index] = '\0';
                
                if (buffer_index > 0) {
                    int hist_slot = history_count % HISTORY_SIZE;
                    int i = 0;
                    while (input_buffer[i] != '\0' && i < BUFFER_SIZE - 1) {
                        command_history[hist_slot][i] = input_buffer[i];
                        i++;
                    }
                    command_history[hist_slot][i] = '\0';
                    if (history_count < HISTORY_SIZE) {
                        history_count++;
                    }
                }
                
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
