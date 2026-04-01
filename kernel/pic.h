#pragma once
#include <stdint.h>

void pic_init();
void pic_send_eoi_master();
void pic_send_eoi_slave();
void pic_mask(uint8_t irq);
void pic_unmask(uint8_t irq);

#include <io.h>
