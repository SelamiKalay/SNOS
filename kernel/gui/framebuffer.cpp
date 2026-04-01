#include "framebuffer.h"
#include "font.h"
#include "../drivers/vesa.h"
#include <string.h>

void fb_draw_char(int x, int y, char ch, uint32_t fg, uint32_t bg) {
    for (int row = 0; row < 16; row++) {
        for (int col = 0; col < 8; col++) {
            if (font_pixel((unsigned char)ch, row, col))
                vesa_put_pixel(x + col, y + row, fg);
            else if (bg != COLOR_TRANSPARENT)
                vesa_put_pixel(x + col, y + row, bg);
        }
    }
}

void fb_draw_string(int x, int y, const char* str, uint32_t fg, uint32_t bg) {
    int cx = x;
    while (*str) {
        if (*str == '\n') { cx = x; y += 16; }
        else { fb_draw_char(cx, y, *str, fg, bg); cx += 8; }
        str++;
    }
}

// Büyütülmüş piksel harfler (logo için)
void fb_draw_string_scaled(int x, int y, const char* str, uint32_t fg, int scale) {
    int cx = x;
    while (*str) {
        unsigned char ch = (unsigned char)*str;
        for (int row = 0; row < 16; row++) {
            for (int col = 0; col < 8; col++) {
                if (font_pixel(ch, row, col)) {
                    vesa_fill_rect(cx + col * scale, y + row * scale, scale, scale, fg);
                }
            }
        }
        cx += 8 * scale + scale;   // karakter genişliği + boşluk
        str++;
    }
}

// ===== SNOS LOGOSU =====
// Üst kısımda büyük "SNOS" yazısı + altında ince çizgi + sürüm
void fb_draw_logo(int cx, int cy) {
    const char* logo_text = "SNOS";
    int scale = 5;
    int char_w = 8 * scale + scale;  // 45px
    int total_w = 4 * char_w - scale;
    int start_x = cx - total_w / 2;

    // Hafif parlak arka plan dikdörtgeni
    vesa_fill_rect(start_x - 20, cy - 15, total_w + 40, 16 * scale + 40, 0xFF080808u);

    // Beyaz logo metni
    fb_draw_string_scaled(start_x, cy, logo_text, COLOR_WHITE, scale);

    // İnce alt çizgi
    vesa_fill_rect(start_x - 10, cy + 16 * scale + 8, total_w + 20, 2, COLOR_WHITE);

    // Sürüm etiketi
    fb_draw_string(cx - 80, cy + 16 * scale + 18, "v0.1 by Selami", 0xFFAAAAAA, COLOR_TRANSPARENT);
}
