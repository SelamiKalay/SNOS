#pragma once
#include <stdint.h>

/* =============================================================================
   Framebuffer çizim arayüzü – font ile metin çizimi dahil
   ============================================================================= */

void fb_draw_char  (int x, int y, char ch,         uint32_t fg, uint32_t bg);
void fb_draw_string(int x, int y, const char* str, uint32_t fg, uint32_t bg);
void fb_draw_string_scaled(int x, int y, const char* str, uint32_t fg, int scale);

// Büyük piksel harflerle logo çizimi
void fb_draw_logo(int cx, int cy);  // merkez koordinatı
