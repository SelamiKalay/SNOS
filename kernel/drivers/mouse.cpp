#include "mouse.h"
#include "../kernel/idt.h"
#include "../kernel/pic.h"
#include "../gui/cursor.h"

#define MOUSE_CMD  0x64
#define MOUSE_DATA 0x60

static uint8_t mouse_cycle = 0;
static int8_t  mouse_byte[3];

static void mouse_wait(int type) {
    int timeout = 100000;
    if (type == 0) {  // data
        while (timeout-- && !(inb(MOUSE_CMD) & 1));
    } else {           // signal
        while (timeout-- &&  (inb(MOUSE_CMD) & 2));
    }
}

static void mouse_write(uint8_t data) {
    mouse_wait(1);
    outb(MOUSE_CMD, 0xD4);
    mouse_wait(1);
    outb(MOUSE_DATA, data);
}

static uint8_t mouse_read() {
    mouse_wait(0);
    return inb(MOUSE_DATA);
}

static void mouse_handler(Registers* r) {
    (void)r;
    uint8_t status = inb(MOUSE_CMD);
    if (!(status & 0x20)) return;   // fare verisi değilse çık

    uint8_t data = inb(MOUSE_DATA);

    switch (mouse_cycle) {
        case 0:
            mouse_byte[0] = (int8_t)data;
            if (data & 0x08) mouse_cycle = 1;  // bit 3 her zaman 1 olmalı
            break;
        case 1:
            mouse_byte[1] = (int8_t)data;
            mouse_cycle = 2;
            break;
        case 2:
            mouse_byte[2] = (int8_t)data;
            mouse_cycle = 0;

            bool left  = (mouse_byte[0] & 0x01) != 0;
            bool right = (mouse_byte[0] & 0x02) != 0;
            cursor_set_buttons(left, right);

            int dx =  (int)mouse_byte[1];
            int dy = -(int)mouse_byte[2];  // Y ekseni ters
            cursor_move(dx, dy);
            break;
    }
}

void mouse_init() {
    // AUX enable
    mouse_wait(1); outb(MOUSE_CMD, 0xA8);
    // Interrupt enable
    mouse_wait(1); outb(MOUSE_CMD, 0x20);
    mouse_wait(0);
    uint8_t status = inb(MOUSE_DATA) | 2;
    mouse_wait(1); outb(MOUSE_CMD, 0x60);
    mouse_wait(1); outb(MOUSE_DATA, status);
    // Default+enable
    mouse_write(0xF6);
    mouse_read();   // ACK
    mouse_write(0xF4);
    mouse_read();

    irq_register_handler(12, mouse_handler);
}
