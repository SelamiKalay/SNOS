#include "desktop.h"
#include "framebuffer.h"
#include "cursor.h"
#include "logconsole.h"
#include "../drivers/vesa.h"
#include "../apps/terminal.h"
#include "../drivers/rtc.h"
#include <io.h>
#include <string.h>

static int screen_w, screen_h;
static const uint32_t COLOR_TURKISH_RED = 0xFFE30A17u;
static const uint32_t COLOR_TASKBAR   = 0xFF181818u;
static const uint32_t COLOR_TASK_HOVER= 0xFF303030u;

static void itoa_pad(int val, char* buf) {
    buf[0] = '0' + (val / 10) % 10;
    buf[1] = '0' + (val % 10);
    buf[2] = '\0';
}

static void itoa_year(int val, char* buf) {
    buf[0] = '0' + (val / 1000) % 10;
    buf[1] = '0' + (val / 100) % 10;
    buf[2] = '0' + (val / 10) % 10;
    buf[3] = '0' + (val % 10);
    buf[4] = '\0';
}

// Görev çubuğu ve butonların çizimi
static void draw_taskbar() {
    int th = 40;
    int ty = screen_h - th;
    
    // Arka plan
    vesa_fill_rect(0, ty, screen_w, th, COLOR_TASKBAR);
    vesa_draw_rect(0, ty, screen_w, 1, 0xFF404040u); // İnce üst çizgi

    int mx = cursor_x();
    int my = cursor_y();

    // 1. SNOS Başlat Butonu (0 - 64)
    bool start_hover = (my >= ty && mx >= 0 && mx <= 64);
    vesa_fill_rect(0, ty + 1, 64, th - 1, start_hover ? COLOR_TURKISH_RED : 0xFFB30812u);
    fb_draw_string(16, ty + 12, "SNOS", COLOR_WHITE, COLOR_TRANSPARENT);

    // 2. Terminal Uygulama İkonu (70 - 120)
    bool term_hover = (my >= ty && mx >= 70 && mx <= 120);
    vesa_fill_rect(70, ty + 5, 50, 30, term_hover ? COLOR_TASK_HOVER : COLOR_TRANSPARENT);
    vesa_draw_rect(70, ty + 5, 50, 30, 0xFF555555u);
    fb_draw_string(85, ty + 12, ">_", 0xFF00FF00u, COLOR_TRANSPARENT);

    // 3. Sistem Tepsisi (Sağ taraf)
    // Saat ve Tarih gösterimi "16:15 | 01.04.2026"
    char time_str[32];
    char buf2[8];
    itoa_pad(rtc_hour, time_str);
    strcat(time_str, ":");
    itoa_pad(rtc_minute, buf2);
    strcat(time_str, buf2);
    strcat(time_str, " | ");
    itoa_pad(rtc_day, buf2);
    strcat(time_str, buf2);
    strcat(time_str, ".");
    itoa_pad(rtc_month, buf2);
    strcat(time_str, buf2);
    strcat(time_str, ".");
    itoa_year(rtc_year, buf2);
    strcat(time_str, buf2);

    int tray_w = 160;
    int tray_x = screen_w - tray_w - 50;
    fb_draw_string(tray_x, ty + 12, time_str, COLOR_WHITE, COLOR_TRANSPARENT);

    // 4. Güç (Shutdown) Butonu (En sağ)
    bool pwr_hover = (my >= ty && mx >= screen_w - 40);
    vesa_fill_rect(screen_w - 40, ty, 40, th, pwr_hover ? 0xFF880000u : COLOR_TRANSPARENT);
    
    // Evrensel Güç İkonu (Beyaz Halka, Kırmızı iç)
    int pcx = screen_w - 20;
    int pcy = ty + 20;
    vesa_fill_circle(pcx, pcy, 10, COLOR_WHITE);
    vesa_fill_circle(pcx, pcy, 7, COLOR_TASKBAR);
    vesa_fill_rect(pcx - 2, pcy - 12, 4, 10, COLOR_WHITE); // Üst çentik
}

static void draw_turkish_flag() {
    // 1. Kırmızı arka plan
    vesa_fill_rect(0, 0, screen_w, screen_h, COLOR_TURKISH_RED);

    // Merkez hesaplaması
    int cx = screen_w / 2;
    int cy = screen_h / 2 - 20; // Görev çubuğundan dolayı hafif yukarıda

    // 2. Büyük Beyaz Daire (Hilalin dışı)
    vesa_fill_circle(cx - 60, cy, 140, COLOR_WHITE);

    // 3. İç Kırmızı Daire (Hilali oluşturan kesim)
    vesa_fill_circle(cx - 20, cy, 110, COLOR_TURKISH_RED);

    // 4. Beyaz Yıldız
    // Hilalin hemen sağına, hafif sola dönük
    vesa_fill_star(cx + 90, cy, 55, COLOR_WHITE);
}

void desktop_init() {
    screen_w = (int)vesa_width();
    screen_h = (int)vesa_height();
    rtc_init();
}

void desktop_render() {
    draw_turkish_flag();
    draw_taskbar();
    cursor_draw();
}

void desktop_run() {
    bool running = true;
    bool last_left = false;
    
    while(running) {
        // Saati güncelle
        rtc_read_datetime();

        desktop_render();
        vesa_swap();

        bool current_left = cursor_left_pressed();
        
        // Tıklama tespiti (Sadece basıldığı ilk an)
        if (current_left && !last_left) {
            int mx = cursor_x();
            int my = cursor_y();

            // Görev Çubuğuna tıklanmışsa
            if (my >= screen_h - 40) {
                // Güç Düğmesi (En sağdaki 40px)
                if (mx >= screen_w - 40) {
                    snos_shutdown();
                }
                // Terminal İkonu (70 - 120 arası)
                else if (mx >= 70 && mx <= 120) {
                    terminal_init();
                    terminal_run(); // Terminal kapanana kadar burayı kilitler
                }
            }
        }
        
        last_left = current_left;
    }
}

void desktop_tick() {
    // Eski mimaride vardı, artık kullanılmıyor
}
