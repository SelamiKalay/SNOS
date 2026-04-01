#pragma once
#include <stdint.h>
#include <multiboot.h>

/* =============================================================================
   VESA Framebuffer Sürücüsü
   GRUB'un sağladığı 32-bit RGBA framebuffer üzerinde çizim yapar.
   ============================================================================= */

void     vesa_init(MultibootInfo* mbi);
uint32_t vesa_width();
uint32_t vesa_height();
void     vesa_put_pixel(int x, int y, uint32_t color);
uint32_t vesa_get_pixel(int x, int y);
void     vesa_fill_rect(int x, int y, int w, int h, uint32_t color);
void     vesa_draw_rect(int x, int y, int w, int h, uint32_t color);
void     vesa_fill_circle(int xc, int yc, int r, uint32_t color);
void     vesa_fill_star(int cx, int cy, int R, uint32_t color);
void     vesa_blit(int dx, int dy, int w, int h, const uint32_t* src);
void     vesa_scroll_region(int x, int y, int w, int h, int pixels_up);
void     vesa_swap();   // Çift tamponlu: back-buffer → VRAM

// Renk sabitleri (ARGB 32-bit)
#define COLOR_BLACK       0xFF000000u
#define COLOR_WHITE       0xFFFFFFFFu
#define COLOR_GRAY        0xFF808080u
#define COLOR_DARK_GRAY   0xFF1A1A1Au
#define COLOR_ACCENT_BLUE 0xFF2979FFu
#define COLOR_YELLOW      0xFFFFD600u
#define COLOR_GREEN       0xFF00E676u
#define COLOR_RED         0xFFFF1744u
#define COLOR_TRANSPARENT 0x00000000u

// Alfa karıştırma (basit % alpha)
static inline uint32_t color_blend(uint32_t dst, uint32_t src, uint8_t alpha) {
    uint8_t r = ((src>>16&0xFF)*alpha + (dst>>16&0xFF)*(255-alpha)) / 255;
    uint8_t g = ((src>>8 &0xFF)*alpha + (dst>>8 &0xFF)*(255-alpha)) / 255;
    uint8_t b = ((src    &0xFF)*alpha + (dst    &0xFF)*(255-alpha)) / 255;
    return 0xFF000000u | (r<<16) | (g<<8) | b;
}
