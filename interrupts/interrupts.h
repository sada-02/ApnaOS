// interrupts.h
#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include <stdint.h>

#define IDT_ENTRIES 256

typedef void (*interrupt_handler_t)();

void register_interrupt_handler(uint8_t n, interrupt_handler_t handler);

void irq_handler(uint8_t irq);

void common_irq_handler(uint32_t irq_num);

void irq_install();

#endif
