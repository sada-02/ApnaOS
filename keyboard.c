#include "keyboard.h"

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static int shift_pressed = 0;

static char scancode_to_ascii_unshifted[128] = {
    [0x02] = '1', [0x03] = '2', [0x04] = '3', [0x05] = '4',
    [0x06] = '5', [0x07] = '6', [0x08] = '7', [0x09] = '8',
    [0x0A] = '9', [0x0B] = '0',
    [0x10] = 'q', [0x11] = 'w', [0x12] = 'e', [0x13] = 'r',
    [0x14] = 't', [0x15] = 'y', [0x16] = 'u', [0x17] = 'i',
    [0x18] = 'o', [0x19] = 'p',
    [0x1C] = '\n',
    [0x1E] = 'a', [0x1F] = 's', [0x20] = 'd', [0x21] = 'f',
    [0x22] = 'g', [0x23] = 'h', [0x24] = 'j', [0x25] = 'k',
    [0x26] = 'l',
    [0x2C] = 'z', [0x2D] = 'x', [0x2E] = 'c', [0x2F] = 'v',
    [0x30] = 'b', [0x31] = 'n', [0x32] = 'm',
    [0x39] = ' '
};

static char scancode_to_ascii_shifted[128] = {
    [0x02] = '!', [0x03] = '@', [0x04] = '#', [0x05] = '$',
    [0x06] = '%', [0x07] = '^', [0x08] = '&', [0x09] = '*',
    [0x0A] = '(', [0x0B] = ')',
    [0x10] = 'Q', [0x11] = 'W', [0x12] = 'E', [0x13] = 'R',
    [0x14] = 'T', [0x15] = 'Y', [0x16] = 'U', [0x17] = 'I',
    [0x18] = 'O', [0x19] = 'P',
    [0x1E] = 'A', [0x1F] = 'S', [0x20] = 'D', [0x21] = 'F',
    [0x22] = 'G', [0x23] = 'H', [0x24] = 'J', [0x25] = 'K',
    [0x26] = 'L',
    [0x2C] = 'Z', [0x2D] = 'X', [0x2E] = 'C', [0x2F] = 'V',
    [0x30] = 'B', [0x31] = 'N', [0x32] = 'M',
    [0x39] = ' '
};

char get_ascii(uint8_t scancode) {
    if (scancode == 0x2A || scancode == 0x36) { // Shift pressed
        shift_pressed = 1;
        return 0;
    }
    if (scancode == 0xAA || scancode == 0xB6) { // Shift released
        shift_pressed = 0;
        return 0;
    }
    if (scancode & 0x80) // Ignore key releases
        return 0;

    return shift_pressed ? scancode_to_ascii_shifted[scancode] : scancode_to_ascii_unshifted[scancode];
}

int read_from_input(char *buffer, size_t size) {
    if (!buffer || size == 0)
        return -1;
    
    while (1) {
        while (!(inb(0x64) & 1))
            ;

        uint8_t scancode = inb(0x60);
        char ascii = get_ascii(scancode);
        if (ascii == 0)
            continue;

        buffer[0] = ascii;
        return 1;
    }
}

int read_line(char *buffer, size_t size) {
    if (!buffer || size == 0)
        return -1;

    size_t count = 0;
    while (count < size - 1) {
        int ret = read_from_input(buffer + count, 1);
        if (ret < 0)
            return -1;
        if (ret == 0)
            continue;

        if (buffer[count] == '\n' || buffer[count] == '\r') {
            count++;
            break;
        }
        count++;
    }
    buffer[count] = '\0';
    return count;
}
