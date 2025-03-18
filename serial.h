#ifndef SERIAL_H
#define SERIAL_H

#include <stdint.h>

// Initialize the serial port.
void serial_init(void);

// Write a single character to the serial port.
void serial_putc(char c);

// Write a string to the serial port.
void serial_print(const char *str);

#endif // SERIAL_H
