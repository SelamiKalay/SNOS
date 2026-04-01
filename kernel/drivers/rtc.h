#pragma once
#include <stdint.h>

void rtc_init();
void rtc_read_datetime();

// Global değişkenler
extern uint8_t rtc_second;
extern uint8_t rtc_minute;
extern uint8_t rtc_hour;
extern uint8_t rtc_day;
extern uint8_t rtc_month;
extern uint16_t rtc_year;
