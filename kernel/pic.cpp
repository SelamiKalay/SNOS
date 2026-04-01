#include "pic.h"

#define PIC1_CMD  0x20
#define PIC1_DATA 0x21
#define PIC2_CMD  0xA0
#define PIC2_DATA 0xA1
#define PIC_EOI   0x20

void pic_init() {
    // ICW1 – cascade, edge triggered, 8-byte intervals
    outb(PIC1_CMD,  0x11); io_wait();
    outb(PIC2_CMD,  0x11); io_wait();
    // ICW2 – vektör offset (IRQ0=32, IRQ8=40)
    outb(PIC1_DATA, 0x20); io_wait();
    outb(PIC2_DATA, 0x28); io_wait();
    // ICW3
    outb(PIC1_DATA, 0x04); io_wait();
    outb(PIC2_DATA, 0x02); io_wait();
    // ICW4 – 8086 modda çalış
    outb(PIC1_DATA, 0x01); io_wait();
    outb(PIC2_DATA, 0x01); io_wait();
    // Tüm IRQ'ları etkinleştir (maskeyi sıfırla)
    outb(PIC1_DATA, 0x00);
    outb(PIC2_DATA, 0x00);
}

void pic_send_eoi_master() { outb(PIC1_CMD, PIC_EOI); }
void pic_send_eoi_slave()  { outb(PIC2_CMD, PIC_EOI); }

void pic_mask(uint8_t irq) {
    uint16_t port = (irq < 8) ? PIC1_DATA : PIC2_DATA;
    if (irq >= 8) irq -= 8;
    outb(port, inb(port) | (1 << irq));
}

void pic_unmask(uint8_t irq) {
    uint16_t port = (irq < 8) ? PIC1_DATA : PIC2_DATA;
    if (irq >= 8) irq -= 8;
    outb(port, inb(port) & ~(1 << irq));
}
