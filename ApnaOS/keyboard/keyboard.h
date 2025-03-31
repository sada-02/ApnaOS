#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>

#define BUFFER_SIZE 256

void init_keyboard();
void keyboard_handler();

extern volatile int input_ready;
extern volatile char input_line[BUFFER_SIZE];

#endif
