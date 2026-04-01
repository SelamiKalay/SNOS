#pragma once
#include <stdint.h>

/* =============================================================================
   IDT (Interrupt Descriptor Table)
   256 giriş: 0-31 CPU istisnası, 32-47 donanım IRQ, 48+ sistem çağrıları
   ============================================================================= */

struct IdtEntry {
    uint16_t base_lo;
    uint16_t sel;
    uint8_t  always0;
    uint8_t  flags;
    uint16_t base_hi;
} __attribute__((packed));

struct IdtPointer {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

// İstek kayıtları (ISR çağrıldığında stack'te)
struct Registers {
    uint32_t ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; // pusha
    uint32_t int_no, err_code;
    uint32_t eip, cs, eflags, useresp, ss;            // CPU otomatik iter
};

typedef void (*IrqHandler)(Registers*);

void idt_init();
void irq_register_handler(int irq, IrqHandler h);
