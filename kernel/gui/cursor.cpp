#include "cursor.h"
#include "../drivers/vesa.h"
#include <string.h>

#define CUR_W 12
#define CUR_H 16

// 12x16 ok imleci sprite (1=beyaz, 2=siyah kenar, 0=şeffaf)
static const uint8_t cursor_shape[CUR_H][CUR_W] = {
    {1,0,0,0,0,0,0,0,0,0,0,0},
    {1,1,0,0,0,0,0,0,0,0,0,0},
    {1,2,1,0,0,0,0,0,0,0,0,0},
    {1,2,2,1,0,0,0,0,0,0,0,0},
    {1,2,2,2,1,0,0,0,0,0,0,0},
    {1,2,2,2,2,1,0,0,0,0,0,0},
    {1,2,2,2,2,2,1,0,0,0,0,0},
    {1,2,2,2,2,2,2,1,0,0,0,0},
    {1,2,2,2,2,2,2,2,1,0,0,0},
    {1,2,2,2,2,1,1,1,1,0,0,0},
    {1,2,2,1,2,2,1,0,0,0,0,0},
    {1,2,1,0,1,2,2,1,0,0,0,0},
    {1,1,0,0,0,1,2,2,1,0,0,0},
    {0,0,0,0,0,0,1,2,1,0,0,0},
    {0,0,0,0,0,0,0,1,1,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0},
};

static int   cur_x = 400, cur_y = 300;
static bool  cur_left = false, cur_right = false;
static uint32_t saved[CUR_H][CUR_W];

void cursor_init() {
    memset(saved, 0, sizeof(saved));
}

void cursor_erase() {
    // Altındaki pikselleri geri yaz
    for (int y = 0; y < CUR_H; y++)
        for (int x = 0; x < CUR_W; x++)
            if (cursor_shape[y][x] != 0)
                vesa_put_pixel(cur_x + x, cur_y + y, saved[y][x]);
}

void cursor_move(int dx, int dy) {
    cur_x += dx; cur_y += dy;
    int w = (int)vesa_width(),  h = (int)vesa_height();
    if (cur_x < 0) cur_x = 0;
    if (cur_y < 0) cur_y = 0;
    if (cur_x > w - 1) cur_x = w - 1;
    if (cur_y > h - 1) cur_y = h - 1;
}

void cursor_draw() {
    // Mevcut pikselleri kaydet, sonra imleci çiz
    for (int y = 0; y < CUR_H; y++) {
        for (int x = 0; x < CUR_W; x++) {
            uint8_t p = cursor_shape[y][x];
            if (p != 0) {
                saved[y][x] = vesa_get_pixel(cur_x + x, cur_y + y);
                uint32_t c = (p == 1) ? COLOR_WHITE : COLOR_BLACK;
                vesa_put_pixel(cur_x + x, cur_y + y, c);
            }
        }
    }
}

int  cursor_x() { return cur_x; }
int  cursor_y() { return cur_y; }
bool cursor_left_pressed()  { return cur_left;  }
bool cursor_right_pressed() { return cur_right; }

void cursor_set_buttons(bool left, bool right) {
    cur_left = left; cur_right = right;
}
