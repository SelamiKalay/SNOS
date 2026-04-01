#include "vesa.h"
#include <string.h>
#include "../memory/heap.h"

static uint32_t* fb_vram   = NULL;  // Donanım framebuffer
static uint32_t* fb_back   = NULL;  // Çift tampon (yazma buraya)
static uint32_t  fb_width  = 0;
static uint32_t  fb_height = 0;
static uint32_t  fb_pitch  = 0;     // Satır başına bayt

void vesa_init(MultibootInfo* mbi) {
    // GRUB framebuffer bilgisini oku
    fb_vram   = (uint32_t*)(uint32_t)mbi->framebuffer_addr;
    fb_pitch  = mbi->framebuffer_pitch;
    fb_width  = mbi->framebuffer_width;
    fb_height = mbi->framebuffer_height;

    // Arka tampon – kmalloc ile ayır
    fb_back = (uint32_t*)kmalloc(fb_height * fb_pitch);

    // Ekranı siyaha boyar
    memset(fb_back, 0, fb_height * fb_pitch);
    memset(fb_vram, 0, fb_height * fb_pitch);
}

uint32_t vesa_width()  { return fb_width;  }
uint32_t vesa_height() { return fb_height; }

void vesa_put_pixel(int x, int y, uint32_t color) {
    if ((uint32_t)x >= fb_width || (uint32_t)y >= fb_height) return;
    uint32_t offset = y * (fb_pitch / 4) + x;
    fb_back[offset] = color;
}

uint32_t vesa_get_pixel(int x, int y) {
    if ((uint32_t)x >= fb_width || (uint32_t)y >= fb_height) return 0;
    return fb_back[y * (fb_pitch / 4) + x];
}

void vesa_fill_rect(int x, int y, int w, int h, uint32_t color) {
    for (int row = y; row < y + h; row++) {
        for (int col = x; col < x + w; col++) {
            vesa_put_pixel(col, row, color);
        }
    }
}

void vesa_draw_rect(int x, int y, int w, int h, uint32_t color) {
    vesa_fill_rect(x,       y,       w, 1, color);  // üst
    vesa_fill_rect(x,       y+h-1,   w, 1, color);  // alt
    vesa_fill_rect(x,       y,       1, h, color);  // sol
    vesa_fill_rect(x+w-1,  y,       1, h, color);  // sağ
}

void vesa_fill_circle(int xc, int yc, int r, uint32_t color) {
    for (int y = -r; y <= r; y++) {
        for (int x = -r; x <= r; x++) {
            if (x*x + y*y <= r*r) {
                vesa_put_pixel(xc + x, yc + y, color);
            }
        }
    }
}

static const int star_x[10] = {0, 224, 951, 363, 587, 0, -587, -363, -951, -224};
static const int star_y[10] = {-1000, -309, -309, 118, 809, 381, 809, 118, -309, -309};

void vesa_fill_star(int cx, int cy, int R, uint32_t color) {
    int px[10], py[10];
    for(int i=0; i<10; i++) {
        // Yıldız sola dönük (Sola açılmış hilale bakar)
        int nx = star_y[i];
        int ny = -star_x[i];
        px[i] = cx + (nx * R) / 1000;
        py[i] = cy + (ny * R) / 1000;
    }
    
    for (int y = cy - R; y <= cy + R; y++) {
        for (int x = cx - R; x <= cx + R; x++) {
            bool in_poly = false;
            for (int i = 0, j = 9; i < 10; j = i++) {
                if (((py[i] > y) != (py[j] > y)) &&
                    (x < (px[j] - px[i]) * (y - py[i]) / (py[j] - py[i]) + px[i])) {
                    in_poly = !in_poly;
                }
            }
            if (in_poly) {
                vesa_put_pixel(x, y, color);
            }
        }
    }
}

void vesa_blit(int dx, int dy, int w, int h, const uint32_t* src) {
    for (int row = 0; row < h; row++) {
        for (int col = 0; col < w; col++) {
            uint32_t c = src[row * w + col];
            if ((c >> 24) != 0) // sadece opak pikseller
                vesa_put_pixel(dx + col, dy + row, c);
        }
    }
}

void vesa_scroll_region(int x, int y, int w, int h, int pixels_up) {
    uint32_t stride = fb_pitch / 4;
    for (int row = y; row < y + h - pixels_up; row++) {
        for (int col = x; col < x + w; col++) {
            fb_back[row * stride + col] = fb_back[(row + pixels_up) * stride + col];
        }
    }
    vesa_fill_rect(x, y + h - pixels_up, w, pixels_up, COLOR_BLACK);
}

void vesa_swap() {
    if (!fb_back || !fb_vram) return;
    memcpy(fb_vram, fb_back, fb_height * fb_pitch);
}
