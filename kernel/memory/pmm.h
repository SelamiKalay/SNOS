#pragma once
#include <stdint.h>

/* =============================================================================
   PMM – Fiziksel Bellek Yöneticisi
   Bitmap tabanlı: Her bit bir 4 KB çerçeveyi temsil eder.
   ============================================================================= */

void    pmm_init(uint32_t kernel_end, void* mmap_addr, uint32_t mmap_len);
void*   pmm_alloc_frame();               // Bir çerçeve tahsis et
void    pmm_free_frame(void* addr);      // Serbest bırak
uint32_t pmm_free_frames();             // Kaç boş çerçeve kaldı
