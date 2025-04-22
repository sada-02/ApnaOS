#include "idt.h"

struct idt_entry idt[256];
struct idt_ptr idtp;

extern void isr_stub();       
extern void idt_load(uint32_t);

void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags)
{
    idt[num].base_low = base & 0xFFFF;
    idt[num].base_high = (base >> 16) & 0xFFFF;
    idt[num].sel = sel;
    idt[num].always0 = 0;
    idt[num].flags = flags;
}

void idt_install()
{
    for (int i = 0; i < 32; i++)
    {
        idt_set_gate(i, (uint32_t)isr_stub, 0x08, 0x8E);
    }
    idtp.limit = (sizeof(struct idt_entry) * 256) - 1;
    idtp.base = (uint32_t)&idt;
    idt_load((uint32_t)&idtp);
}
