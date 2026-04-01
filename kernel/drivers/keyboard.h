#pragma once
#include <stdint.h>
#include "../kernel/idt.h"

/* Klavye sürücüsü + global pano */
void keyboard_init();

// Bir sonraki karakteri bekle (bloklayan)
char keyboard_getchar();

// Pano (Clipboard)
void clipboard_copy  (const char* text, size_t len);
void clipboard_paste (char* out_buf, size_t* out_len);

// Modifier durumları
bool key_ctrl_pressed();
bool key_shift_pressed();
bool key_alt_pressed();
