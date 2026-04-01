#include "pmm.h"
#include <string.h>
#include <multiboot.h>

#define FRAME_SIZE   4096
#define MAX_FRAMES   (256 * 1024)   // 4 GB / 4 KB = 1M çerçeve; 256K ile sınırla

static uint32_t bitmap[MAX_FRAMES / 32];
static uint32_t total_frames  = 0;
static uint32_t free_frames_n = 0;

static inline void bit_set(uint32_t frame) {
    bitmap[frame / 32] |=  (1u << (frame % 32));
}
static inline void bit_clear(uint32_t frame) {
    bitmap[frame / 32] &= ~(1u << (frame % 32));
}
static inline bool bit_test(uint32_t frame) {
    return (bitmap[frame / 32] >> (frame % 32)) & 1;
}

void pmm_init(uint32_t kernel_end, void* mmap_addr, uint32_t mmap_len) {
    // Tüm bitleri dolu (kullanılmış) olarak işaretle
    memset(bitmap, 0xFF, sizeof(bitmap));
    free_frames_n = 0;

    // GRUB bellek haritasından kullanılabilir bölgeleri serbest bırak
    uint8_t* ptr = (uint8_t*)mmap_addr;
    uint8_t* end = ptr + mmap_len;

    while (ptr < end) {
        MultibootMmapEntry* e = (MultibootMmapEntry*)ptr;
        if (e->type == MULTIBOOT_MEMORY_AVAIL) {
            uint32_t base   = (uint32_t)e->base_addr;
            uint32_t length = (uint32_t)e->length;

            // İlk 1 MB ve kernel bölgesini koru
            if (base < 0x100000) {
                uint32_t skip = 0x100000 - base;
                if (skip >= length) { ptr += e->size + 4; continue; }
                base += skip; length -= skip;
            }
            // Kernel sonrasını serbest bırak
            if (base < kernel_end) {
                uint32_t skip = kernel_end - base;
                if (skip >= length) { ptr += e->size + 4; continue; }
                base += skip; length -= skip;
            }

            uint32_t frame_start = base / FRAME_SIZE;
            uint32_t frame_count = length / FRAME_SIZE;

            for (uint32_t i = 0; i < frame_count; i++) {
                uint32_t f = frame_start + i;
                if (f < MAX_FRAMES) {
                    bit_clear(f);
                    free_frames_n++;
                    if (f > total_frames) total_frames = f;
                }
            }
        }
        ptr += e->size + 4;
    }
}

void* pmm_alloc_frame() {
    for (uint32_t i = 0; i <= total_frames / 32; i++) {
        if (bitmap[i] == 0xFFFFFFFF) continue;
        for (int j = 0; j < 32; j++) {
            uint32_t f = i * 32 + j;
            if (!bit_test(f)) {
                bit_set(f);
                free_frames_n--;
                return (void*)(f * FRAME_SIZE);
            }
        }
    }
    return NULL;  // Bellek doldu
}

void pmm_free_frame(void* addr) {
    uint32_t frame = (uint32_t)addr / FRAME_SIZE;
    if (bit_test(frame)) {
        bit_clear(frame);
        free_frames_n++;
    }
}

uint32_t pmm_free_frames() { return free_frames_n; }
