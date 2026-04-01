#pragma once
#include <stdint.h>

/* Fare imleci – 16x16 ok sprite */
void cursor_init();
void cursor_move(int dx, int dy);
void cursor_draw();
void cursor_erase();
int  cursor_x();
int  cursor_y();
bool cursor_left_pressed();
bool cursor_right_pressed();
void cursor_set_buttons(bool left, bool right);
