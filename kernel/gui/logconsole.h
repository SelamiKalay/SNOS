#pragma once
#include <stdint.h>

/* Log konsolu – ekranın sağ alt köşesinde yarı saydam panel */
void logconsole_init(int x, int y, int w, int h);
void logconsole_print(const char* msg);
void logconsole_render();
