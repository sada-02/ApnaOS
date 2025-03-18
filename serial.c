#include "serial.h"

#define COM1 0x3F8  // Base I/O port for COM1

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// Initialize COM1 for 38400 baud, 8N1.
void serial_init(void) {
    outb(COM1 + 1, 0x00);    // Disable interrupts
    outb(COM1 + 3, 0x80);    // Enable DLAB
    outb(COM1 + 0, 0x03);    // Divisor = 3 (low byte) -> 38400 baud
    outb(COM1 + 1, 0x00);    // (high byte)
    outb(COM1 + 3, 0x03);    // 8 bits, no parity, one stop bit
    outb(COM1 + 2, 0xC7);    // Enable FIFO, clear them, 14-byte threshold
    outb(COM1 + 4, 0x0B);    // IRQs enabled, RTS/DSR set
}

// Write one character to COM1.
void serial_putc(char c) {
    // Wait for the transmit buffer to be empty
    while ((inb(COM1 + 5) & 0x20) == 0)
        ;
    outb(COM1, c);
}

// Write a string to COM1.
void serial_print(const char *str) {
    while (*str) {
        serial_putc(*str++);
    }
}
