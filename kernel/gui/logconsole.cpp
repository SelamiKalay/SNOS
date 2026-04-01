#include "logconsole.h"
#include "framebuffer.h"
#include "../drivers/vesa.h"
#include <string.h>

#define LOG_LINES  12
#define LOG_WIDTH  38   // max karakter/satır

static char  log_buf[LOG_LINES][LOG_WIDTH + 1];
static int   log_head = 0;
static int   log_count = 0;

static int lc_x, lc_y, lc_w, lc_h;

void logconsole_init(int x, int y, int w, int h) {
    lc_x = x; lc_y = y; lc_w = w; lc_h = h;
    for (int i = 0; i < LOG_LINES; i++)
        log_buf[i][0] = '\0';
}

void logconsole_print(const char* msg) {
    int idx = (log_head + log_count) % LOG_LINES;
    if (log_count == LOG_LINES) {
        // Tam doluysa en eskiyi üzerine yaz
        idx = log_head;
        log_head = (log_head + 1) % LOG_LINES;
    } else {
        log_count++;
    }
    // Mesajı kopyala (kırp)
    int i;
    for (i = 0; i < LOG_WIDTH && msg[i]; i++)
        log_buf[idx][i] = msg[i];
    log_buf[idx][i] = '\0';
}

void logconsole_render() {
    // Yarı saydam panel arka planı
    for (int row = lc_y; row < lc_y + lc_h; row++) {
        for (int col = lc_x; col < lc_x + lc_w; col++) {
            uint32_t dst = vesa_get_pixel(col, row);
            // Siyah + %40 opaklık = dst*0.6
            uint8_t r = (dst >> 16 & 0xFF) * 6 / 10;
            uint8_t g = (dst >>  8 & 0xFF) * 6 / 10;
            uint8_t b = (dst       & 0xFF) * 6 / 10;
            vesa_put_pixel(col, row, 0xFF000000u | (r<<16) | (g<<8) | b);
        }
    }

    // Kenarlık
    vesa_draw_rect(lc_x, lc_y, lc_w, lc_h, 0xFF333333u);

    // Başlık çizgisi
    vesa_fill_rect(lc_x + 1, lc_y + 1, lc_w - 2, 18, 0xFF111111u);
    fb_draw_string(lc_x + 6, lc_y + 2, "[SNOS Log]", 0xFF4FC3F7u, COLOR_TRANSPARENT);

    // Satırları çiz
    for (int i = 0; i < log_count; i++) {
        int idx = (log_head + i) % LOG_LINES;
        fb_draw_string(lc_x + 4, lc_y + 22 + i * 17,
                       log_buf[idx], 0xFFCCCCCC, COLOR_TRANSPARENT);
    }
}
