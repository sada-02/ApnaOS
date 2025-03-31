#include "pic.h"
#include "../keyboard/io.h"

void pic_remap()
{
    uint8_t a1 = inb(0x21); // save masks
    uint8_t a2 = inb(0xA1);

    outb(0x20, 0x11); // start initialization sequence in cascade mode
    outb(0xA0, 0x11);

    outb(0x21, 0x20); // Master PIC vector offset
    outb(0xA0, 0x28); // Slave PIC vector offset

    outb(0x21, 0x04); // tell Master PIC there is a slave PIC at IRQ2 (0000 0100)
    outb(0xA0, 0x02); // tell Slave PIC its cascade identity

    outb(0x21, 0x01);
    outb(0xA0, 0x01);

    // Unmask IRQ1 (keyboard) by clearing bit 1 of the master mask:
    a1 &= ~(1 << 1);

    outb(0x21, a1); // restore updated masks
    outb(0xA0, a2);
}
