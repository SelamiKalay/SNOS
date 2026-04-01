#pragma once
#include <stdint.h>

/* =============================================================================
   Multiboot v1 yapısı – GRUB bu struct'ı kernel'e EBX üzerinden iletir.
   ============================================================================= */

struct MultibootInfo {
    uint32_t flags;

    // flags[0] – alt/üst bellek
    uint32_t mem_lower;
    uint32_t mem_upper;

    // flags[1] – boot cihazı
    uint32_t boot_device;

    // flags[2] – komut satırı
    uint32_t cmdline;

    // flags[3] – modüller
    uint32_t mods_count;
    uint32_t mods_addr;

    // flags[4,5] – semboller (kullanmıyoruz)
    uint32_t syms[4];

    // flags[6] – bellek haritası
    uint32_t mmap_length;
    uint32_t mmap_addr;

    // flags[7] – sürücüler
    uint32_t drives_length;
    uint32_t drives_addr;

    // flags[8] – config table
    uint32_t config_table;

    // flags[9] – boot loader adı
    uint32_t boot_loader_name;

    // flags[10] – APM table
    uint32_t apm_table;

    // flags[11] – VBE bilgisi
    uint32_t vbe_control_info;
    uint32_t vbe_mode_info;
    uint16_t vbe_mode;
    uint16_t vbe_interface_seg;
    uint16_t vbe_interface_off;
    uint16_t vbe_interface_len;

    // flags[12] – Framebuffer (kullanacağız!)
    uint64_t framebuffer_addr;
    uint32_t framebuffer_pitch;
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;
    uint8_t  framebuffer_bpp;
    uint8_t  framebuffer_type;   // 0=indexed, 1=RGB
    uint8_t  color_info[6];
} __attribute__((packed));

// Bellek haritası girdisi
struct MultibootMmapEntry {
    uint32_t size;
    uint64_t base_addr;
    uint64_t length;
    uint32_t type;   // 1 = kullanılabilir RAM
} __attribute__((packed));

#define MULTIBOOT_MAGIC          0x2BADB002
#define MULTIBOOT_FLAG_MMAP      (1 << 6)
#define MULTIBOOT_FLAG_FB        (1 << 12)
#define MULTIBOOT_MEMORY_AVAIL   1
