#pragma once
#include <stdint.h>

/* Terminal (Shell) – masaüstünde açılabilir komut satırı */
void terminal_init ();
void terminal_run  ();   // Bloklayan ana döngü
void terminal_render();  // İçeriği framebuffer'a çiz
