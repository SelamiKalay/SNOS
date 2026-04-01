#pragma once
#include <stdint.h>

/* 8x16 piksel PSF-benzeri gömülü font
   font_get_bitmap(ch) → her bit bir pikseli temsil eden 16-byte dizi döner */

extern const uint8_t snos_font[128][16];  // ASCII 0-127, her char 16 satır x 8 bit

static inline bool font_pixel(unsigned char ch, int row, int col) {
    if (ch >= 128) ch = '?';
    return (snos_font[ch][row] >> (7 - col)) & 1;
}
