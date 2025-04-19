#include "gdt.h"

// We need three GDT entries: null, code, data.
struct gdt_entry gdt[3];
struct gdt_ptr gp;

// External assembly routine to load our new GDT.
extern void gdt_flush(uint32_t);

static void gdt_set_gate(int num, unsigned long base, unsigned long limit,
                         uint8_t access, uint8_t gran)
{
    gdt[num].base_low = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high = (base >> 24) & 0xFF;

    gdt[num].limit_low = (limit & 0xFFFF);
    gdt[num].granularity = ((limit >> 16) & 0x0F);

    gdt[num].granularity |= (gran & 0xF0);
    gdt[num].access = access;
}

void gdt_install()
{
    gp.limit = (sizeof(struct gdt_entry) * 3) - 1;
    gp.base = (uint32_t)&gdt;

    // Null descriptor.
    gdt_set_gate(0, 0, 0, 0, 0);
    // Code segment: base=0, limit=4GB, access=0x9A, gran=0xCF.
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);
    // Data segment: base=0, limit=4GB, access=0x92, gran=0xCF.
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

    // Load the new GDT.
    gdt_flush((uint32_t)&gp);
}
