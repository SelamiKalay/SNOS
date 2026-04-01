#pragma once
#include <stdint.h>

// I/O port yardımcıları
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile("outb %0, %1" :: "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t val;
    __asm__ volatile("inb %1, %0" : "=a"(val) : "Nd"(port));
    return val;
}

static inline void outw(uint16_t port, uint16_t val) {
    __asm__ volatile("outw %0, %1" :: "a"(val), "Nd"(port));
}

static inline void io_wait() {
    outb(0x80, 0);
}

// Güç yönetimi: ACPI Shutdown
static inline void snos_shutdown() {
    // 1. QEMU ACPI Kapatma Portu
    outw(0x604, 0x2000);
    
    // 2. VirtualBox ACPI Kapatma Portu
    outw(0x4004, 0x3400);

    // 3. Bochs / Eski sistemler için alternatif
    outw(0xB004, 0x2000);

    // 4. Fallback: Gerçek donanım veya sinyalin işe yaramadığı durumlar
    // Kesmeleri kapat ve işlemciyi sonsuz uyku döngüsüne al.
    __asm__ volatile("cli");
    while (true) {
        __asm__ volatile("hlt");
    }
}
