// // keyboard.c
#include "keyboard.h"
#include "io.h"
#include "interrupts.h"

#define KBD_DATA_PORT 0x60
#define KBD_STATUS_PORT 0x64
#define KBD_SCANCODE_RELEASE 0x80
#define BUFFER_SIZE 256

static char input_buffer[BUFFER_SIZE];
volatile int input_ready = 0;          // Flag: input is complete
volatile char input_line[BUFFER_SIZE]; // Global complete input line
static int buffer_index = 0;
extern void print_to_screen(const char *);

static const char scancode_to_ascii[128] = {
    /*  0 */ 0, /*  1 */ 27, /*  2 */ '1', /*  3 */ '2',
    /*  4 */ '3', /*  5 */ '4', /*  6 */ '5', /*  7 */ '6',
    /*  8 */ '7', /*  9 */ '8', /* 10 */ '9', /* 11 */ '0',
    /* 12 */ '-', /* 13 */ '=', /* 14 */ '\b', /* 15 */ '\t', // Backspace, Tab
    /* 16 */ 'q', /* 17 */ 'w', /* 18 */ 'e', /* 19 */ 'r',
    /* 20 */ 't', /* 21 */ 'y', /* 22 */ 'u', /* 23 */ 'i',
    /* 24 */ 'o', /* 25 */ 'p', /* 26 */ '[', /* 27 */ ']',
    /* 28 */ '\n', /* 29 */ 0, /* 30 */ 'a', /* 31 */ 's', // Enter, Control
    /* 32 */ 'd', /* 33 */ 'f', /* 34 */ 'g', /* 35 */ 'h',
    /* 36 */ 'j', /* 37 */ 'k', /* 38 */ 'l', /* 39 */ ';',
    /* 40 */ '\'', /* 41 */ '`', /* 42 */ 0, /* 43 */ '\\', // Left shift
    /* 44 */ 'z', /* 45 */ 'x', /* 46 */ 'c', /* 47 */ 'v',
    /* 48 */ 'b', /* 49 */ 'n', /* 50 */ 'm', /* 51 */ ',',
    /* 52 */ '.', /* 53 */ '/', /* 54 */ 0, /* 55 */ '*', // Right shift, Keypad *
    /* 56 */ 0, /* 57 */ ' ', /* 58 */ 0, /* 59 */ 0,
    /* 60 */ 0, /* 61 */ 0, /* 62 */ 0, /* 63 */ 0,
    /* 64 */ 0, /* 65 */ 0, /* 66 */ 0, /* 67 */ 0,
    /* 68 */ 0, /* 69 */ 0, /* 70 */ 0, /* 71 */ 0,
    /* 72 */ 0, /* 73 */ 0, /* 74 */ 0, /* 75 */ 0,
    /* 76 */ 0, /* 77 */ 0, /* 78 */ 0, /* 79 */ 0,
    /* 80 */ 0, /* 81 */ 0, /* 82 */ 0, /* 83 */ 0,
    /* 84 */ 0, /* 85 */ 0, /* 86 */ 0, /* 87 */ 0,
    /* 88 */ 0, /* 89 */ 0, /* 90 */ 0, /* 91 */ 0,
    /* 92 */ 0, /* 93 */ 0, /* 94 */ 0, /* 95 */ 0,
    /* 96 */ 0, /* 97 */ 0, /* 98 */ 0, /* 99 */ 0,
    /*100 */ 0, /*101 */ 0, /*102 */ 0, /*103 */ 0,
    /*104 */ 0, /*105 */ 0, /*106 */ 0, /*107 */ 0,
    /*108 */ 0, /*109 */ 0, /*110 */ 0, /*111 */ 0,
    /*112 */ 0, /*113 */ 0, /*114 */ 0, /*115 */ 0,
    /*116 */ 0, /*117 */ 0, /*118 */ 0, /*119 */ 0,
    /*120 */ 0, /*121 */ 0, /*122 */ 0, /*123 */ 0,
    /*124 */ 0, /*125 */ 0, /*126 */ 0, /*127 */ 0};

void keyboard_handler()
{
    uint8_t scancode = inb(KBD_DATA_PORT);
    // Only handle key press (not release)
    if (!(scancode & KBD_SCANCODE_RELEASE))
    {
        char key = scancode_to_ascii[scancode];
        if (key)
        {
            // Handle newline (Enter key)
            if (key == '\n')
            {
                // Terminate buffer and process the input line.
                input_buffer[buffer_index] = '\0';
                print_to_screen("\nYou entered: ");
                print_to_screen(input_buffer);
                print_to_screen("\n");
                buffer_index = 0; // Reset for the next line.
            }
            // Handle backspace
            else if (key == '\b')
            {
                if (buffer_index > 0)
                {
                    buffer_index--;
                    // Optionally, remove the character from the screen.
                    print_to_screen("\b \b"); // Simple backspace effect.
                }
            }
            // Add normal character to buffer
            else
            {
                if (buffer_index < BUFFER_SIZE - 1)
                {
                    input_buffer[buffer_index++] = key;
                    // Optionally echo the character to the screen:
                    char buf[2] = {key, '\0'};
                    print_to_screen(buf);
                }
            }
        }
    }
}

void init_keyboard()
{
    // Register the keyboard handler for IRQ1 (mapped to interrupt 33)
    register_interrupt_handler(33, keyboard_handler);
    print_to_screen("DEBUG: Keyboard interrupt handler registered.\n");
}
// #include "keyboard.h"
// #include "io.h"
// #include "screen.h"
// #include "interrupts.h"

// #define BUFFER_SIZE 256
// #define KBD_DATA_PORT 0x60
// #define KBD_STATUS_PORT 0x64
// #define KBD_SCANCODE_RELEASE 0x80

// static char input_buffer[BUFFER_SIZE];
// static int buffer_index = 0;

// static const char scancode_to_ascii[128] = {
//     /*  0 */ 0, /*  1 */ 27, /*  2 */ '1', /*  3 */ '2',
//     /*  4 */ '3', /*  5 */ '4', /*  6 */ '5', /*  7 */ '6',
//     /*  8 */ '7', /*  9 */ '8', /* 10 */ '9', /* 11 */ '0',
//     /* 12 */ '-', /* 13 */ '=', /* 14 */ '\b', /* 15 */ '\t', // Backspace, Tab
//     /* 16 */ 'q', /* 17 */ 'w', /* 18 */ 'e', /* 19 */ 'r',
//     /* 20 */ 't', /* 21 */ 'y', /* 22 */ 'u', /* 23 */ 'i',
//     /* 24 */ 'o', /* 25 */ 'p', /* 26 */ '[', /* 27 */ ']',
//     /* 28 */ '\n', /* 29 */ 0, /* 30 */ 'a', /* 31 */ 's', // Enter, Control
//     /* 32 */ 'd', /* 33 */ 'f', /* 34 */ 'g', /* 35 */ 'h',
//     /* 36 */ 'j', /* 37 */ 'k', /* 38 */ 'l', /* 39 */ ';',
//     /* 40 */ '\'', /* 41 */ '`', /* 42 */ 0, /* 43 */ '\\', // Left shift
//     /* 44 */ 'z', /* 45 */ 'x', /* 46 */ 'c', /* 47 */ 'v',
//     /* 48 */ 'b', /* 49 */ 'n', /* 50 */ 'm', /* 51 */ ',',
//     /* 52 */ '.', /* 53 */ '/', /* 54 */ 0, /* 55 */ '*', // Right shift, Keypad *
//     /* 56 */ 0, /* 57 */ ' ', /* 58 */ 0, /* 59 */ 0,
//     /* 60 */ 0, /* 61 */ 0, /* 62 */ 0, /* 63 */ 0,
//     /* 64 */ 0, /* 65 */ 0, /* 66 */ 0, /* 67 */ 0,
//     /* 68 */ 0, /* 69 */ 0, /* 70 */ 0, /* 71 */ 0,
//     /* 72 */ 0, /* 73 */ 0, /* 74 */ 0, /* 75 */ 0,
//     /* 76 */ 0, /* 77 */ 0, /* 78 */ 0, /* 79 */ 0,
//     /* 80 */ 0, /* 81 */ 0, /* 82 */ 0, /* 83 */ 0,
//     /* 84 */ 0, /* 85 */ 0, /* 86 */ 0, /* 87 */ 0,
//     /* 88 */ 0, /* 89 */ 0, /* 90 */ 0, /* 91 */ 0,
//     /* 92 */ 0, /* 93 */ 0, /* 94 */ 0, /* 95 */ 0,
//     /* 96 */ 0, /* 97 */ 0, /* 98 */ 0, /* 99 */ 0,
//     /*100 */ 0, /*101 */ 0, /*102 */ 0, /*103 */ 0,
//     /*104 */ 0, /*105 */ 0, /*106 */ 0, /*107 */ 0,
//     /*108 */ 0, /*109 */ 0, /*110 */ 0, /*111 */ 0,
//     /*112 */ 0, /*113 */ 0, /*114 */ 0, /*115 */ 0,
//     /*116 */ 0, /*117 */ 0, /*118 */ 0, /*119 */ 0,
//     /*120 */ 0, /*121 */ 0, /*122 */ 0, /*123 */ 0,
//     /*124 */ 0, /*125 */ 0, /*126 */ 0, /*127 */ 0};

// void keyboard_handler()
// {
//     uint8_t scancode = inb(KBD_DATA_PORT);
//     // Only process key press events (ignore releases)
//     if (!(scancode & KBD_SCANCODE_RELEASE))
//     {
//         char key = scancode_to_ascii[scancode];
//         if (key)
//         {
//             if (key == '\n')
//             {
//                 // Terminate the input buffer
//                 input_buffer[buffer_index] = '\0';
//                 print_to_screen("\nYou entered: ");
//                 print_to_screen(input_buffer);
//                 print_to_screen("\nExecution finished.\n");
//                 // Disable interrupts and halt the CPU
//                 asm volatile("cli");
//                 while (1)
//                 {
//                     asm volatile("hlt");
//                 }
//             }
//             else if (key == '\b')
//             {
//                 if (buffer_index > 0)
//                 {
//                     buffer_index--;
//                     // Backspace effect on screen
//                     print_to_screen("\b \b");
//                 }
//             }
//             else
//             {
//                 if (buffer_index < BUFFER_SIZE - 1)
//                 {
//                     input_buffer[buffer_index++] = key;
//                     // Echo the character to the screen
//                     char buf[2] = {key, '\0'};
//                     print_to_screen(buf);
//                 }
//             }
//         }
//     }
// }
