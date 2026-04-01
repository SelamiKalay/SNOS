#pragma once
#include <stdint.h>

/* =============================================================================
   GDT (Global Descriptor Table)
   3 segment: null, kernel code (ring 0), kernel data (ring 0)
   ============================================================================= */

struct GdtEntry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_mid;
    uint8_t  access;
    uint8_t  granularity;
    uint8_t  base_high;
} __attribute__((packed));

struct GdtPointer {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

void gdt_init();
