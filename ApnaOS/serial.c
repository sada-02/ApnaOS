#include "serial.h"
#include "io.h"

#define SERIAL_PORT 0x3F8

void serial_init()
{
    outb(SERIAL_PORT + 1, 0x00); // Disable interrupts
    outb(SERIAL_PORT + 3, 0x80); // Enable DLAB
    outb(SERIAL_PORT + 0, 0x03); // Set baud rate divisor (low byte) to 3 (38400 baud)
    outb(SERIAL_PORT + 1, 0x00); // High byte of divisor
    outb(SERIAL_PORT + 3, 0x03); // 8 bits, no parity, one stop bit
    outb(SERIAL_PORT + 2, 0xC7); // Enable FIFO, clear them, 14-byte threshold
    outb(SERIAL_PORT + 4, 0x0B); // IRQs enabled, RTS/DSR set
}

void serial_print(const char *message)
{
    while (*message)
    {
        // Wait until transmit buffer is empty
        while (!(inb(SERIAL_PORT + 5) & 0x20))
            ;
        outb(SERIAL_PORT, *message++);
    }
}
