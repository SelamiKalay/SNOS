#include "rtc.h"
#include <io.h>

uint8_t rtc_second = 0;
uint8_t rtc_minute = 0;
uint8_t rtc_hour = 0;
uint8_t rtc_day = 0;
uint8_t rtc_month = 0;
uint16_t rtc_year = 0;

static int get_update_in_progress_flag() {
    outb(0x70, 0x0A);
    return (inb(0x71) & 0x80);
}

static uint8_t get_rtc_register(int reg) {
    outb(0x70, reg);
    return inb(0x71);
}

void rtc_init() {
    rtc_read_datetime();
}

void rtc_read_datetime() {
    // Kaldırıldı: while (get_update_in_progress_flag()); 
    // Bazı sanal makinelerde (VMware/QEMU) döngü sonsuza girip ekranın terminalde takılı kalmasına (hang) sebep oluyor.
    // Artık beklemeden doğrudan okuyacağız, saniyede 60 kez okunduğu için hata anında düzelecektir.
    
    rtc_second = get_rtc_register(0x00);
    rtc_minute = get_rtc_register(0x02);
    rtc_hour   = get_rtc_register(0x04);
    rtc_day    = get_rtc_register(0x07);
    rtc_month  = get_rtc_register(0x08);
    rtc_year   = get_rtc_register(0x09);
    
    uint8_t registerB = get_rtc_register(0x0B);

    // BCD'den Decimal'e cevir
    if (!(registerB & 0x04)) {
        rtc_second = (rtc_second & 0x0F) + ((rtc_second / 16) * 10);
        rtc_minute = (rtc_minute & 0x0F) + ((rtc_minute / 16) * 10);
        rtc_hour = ( (rtc_hour & 0x0F) + (((rtc_hour & 0x70) / 16) * 10) ) | (rtc_hour & 0x80);
        rtc_day = (rtc_day & 0x0F) + ((rtc_day / 16) * 10);
        rtc_month = (rtc_month & 0x0F) + ((rtc_month / 16) * 10);
        rtc_year = (rtc_year & 0x0F) + ((rtc_year / 16) * 10);
    }

    // GMT+3 (Türkiye Saati için kaba bir ekleme)
    rtc_hour = (rtc_hour + 3) % 24; 
    
    // Yili duzelt (2000 lerdeyiz)
    rtc_year += 2000;
}
