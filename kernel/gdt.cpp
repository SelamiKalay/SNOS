#include "gdt.h"

static GdtEntry gdt_entries[3];
static GdtPointer gdt_ptr;

extern "C" void gdt_flush(uint32_t);   // isr.asm içinde tanımlı

static void set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt_entries[num].base_low    = (base & 0xFFFF);
    gdt_entries[num].base_mid    = (base >> 16) & 0xFF;
    gdt_entries[num].base_high   = (base >> 24) & 0xFF;
    gdt_entries[num].limit_low   = (limit & 0xFFFF);
    gdt_entries[num].granularity = ((limit >> 16) & 0x0F) | (gran & 0xF0);
    gdt_entries[num].access      = access;
}

void gdt_init() {
    gdt_ptr.limit = (sizeof(GdtEntry) * 3) - 1;
    gdt_ptr.base  = (uint32_t)&gdt_entries;

    set_gate(0, 0, 0, 0, 0);                  // Null segment (zorunlu)
    set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);   // Kernel Code (ring 0, 32-bit)
    set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);   // Kernel Data (ring 0, 32-bit)

    gdt_flush((uint32_t)&gdt_ptr);
}
