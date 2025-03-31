// interrupts.h
#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include <stdint.h>

#define IDT_ENTRIES 256

typedef void (*interrupt_handler_t)();

// Register a handler for a specific interrupt.
void register_interrupt_handler(uint8_t n, interrupt_handler_t handler);

// Called by the IRQ assembly stubs via the common handler.
void irq_handler(uint8_t irq);

// New common IRQ handler called from assembly.
void common_irq_handler(uint32_t irq_num);

// Install IRQ entries in the IDT (for IRQ0–IRQ15).
void irq_install();

#endif
