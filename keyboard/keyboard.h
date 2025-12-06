#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>

#define BUFFER_SIZE 256

void init_keyboard();
void keyboard_handler();
void switch_history_context(int user_id);

extern volatile int input_ready;
extern volatile char input_line[BUFFER_SIZE];

#endif
