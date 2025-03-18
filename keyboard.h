#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stddef.h>
#include <stdint.h>

// Reads input from the keyboard into the provided buffer.
// Returns the number of bytes read, or -1 on error.
int read_from_input(char *buffer, size_t size);

#endif // KEYBOARD_H
