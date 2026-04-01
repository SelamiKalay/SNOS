/* =============================================================================
   SNOS – kernel.cpp
   kernel_main: Tüm alt sistemleri sırayla başlatır, masaüstü döngüsüne girer.
   ============================================================================= */
#include <multiboot.h>
#include <string.h>
#include "gdt.h"
#include "idt.h"
#include "pic.h"
#include "memory/pmm.h"
#include "memory/heap.h"
#include "drivers/vesa.h"
#include "drivers/keyboard.h"
#include "drivers/mouse.h"
#include "gui/framebuffer.h"
#include "gui/cursor.h"
#include "gui/desktop.h"
#include "gui/logconsole.h"
#include "fs/ramdisk.h"
#include "apps/terminal.h"

// Linker script'ten gelen sembol
extern uint32_t kernel_end;

extern "C" void kernel_main(uint32_t magic, MultibootInfo* mbi) {
    (void)magic;
    // ================================================
    // 1. CPU Alt Sistemleri
    // ================================================
    gdt_init();
    idt_init();
    __asm__ volatile("sti");   // Kesmeleri etkinleştir

    // ================================================
    // 2. Bellek Yönetimi
    // ================================================
    if (mbi->flags & MULTIBOOT_FLAG_MMAP) {
        pmm_init((uint32_t)&kernel_end,
                 (void*)mbi->mmap_addr,
                 mbi->mmap_length);
    }
    // 32 MB heap – kernel sonrasına yerleştir
    uint32_t heap_start = ((uint32_t)&kernel_end + 0xFFFFF) & ~0xFFFFF;
    heap_init(heap_start, 32 * 1024 * 1024);

    // ================================================
    // 3. Grafik Sürücüsü
    // ================================================
    if (!(mbi->flags & MULTIBOOT_FLAG_FB)) {
        // FB yoksa CPU'yu durdur – GRUB ayarı eksik
        __asm__ volatile("cli; hlt");
    }
    vesa_init(mbi);

    // ================================================
    // 4. GUI Başlat
    // ================================================
    cursor_init();
    desktop_init();

    // ================================================
    // 5. Giriş Sürücüleri
    // ================================================
    keyboard_init();
    mouse_init();

    // ================================================
    // 6. Dosya Sistemi
    // ================================================
    rd_init();

    // ================================================
    // 7. Masaüstü Başlat (Ana Etkileşim Döngüsü)
    // ================================================
    logconsole_print("SNOS Desktop baslatiliyor...");
    desktop_run();   // Sonsuz etkileşim döngüsü

    // Eğer döngüden çıkılırsa sistemi uyut
    __asm__ volatile("cli");
    while (true) {
        __asm__ volatile("hlt");
    }
}
