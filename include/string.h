#pragma once
#include <stdint.h>

// Bellek kopyalama / doldurma (libc yok, kendimiz yazıyoruz)
static inline void* memcpy(void* dst, const void* src, size_t n) {
    uint8_t* d = (uint8_t*)dst;
    const uint8_t* s = (const uint8_t*)src;
    while (n--) *d++ = *s++;
    return dst;
}

static inline void* memset(void* ptr, int val, size_t n) {
    uint8_t* p = (uint8_t*)ptr;
    while (n--) *p++ = (uint8_t)val;
    return ptr;
}

static inline int memcmp(const void* a, const void* b, size_t n) {
    const uint8_t* pa = (const uint8_t*)a;
    const uint8_t* pb = (const uint8_t*)b;
    while (n--) {
        if (*pa != *pb) return *pa - *pb;
        pa++; pb++;
    }
    return 0;
}

static inline size_t strlen(const char* s) {
    size_t n = 0;
    while (*s++) n++;
    return n;
}

static inline int strcmp(const char* a, const char* b) {
    while (*a && *a == *b) { a++; b++; }
    return (unsigned char)*a - (unsigned char)*b;
}

static inline int strncmp(const char* a, const char* b, size_t n) {
    while (n-- && *a && *a == *b) { a++; b++; }
    if (n == (size_t)-1) return 0;
    return (unsigned char)*a - (unsigned char)*b;
}

static inline char* strcpy(char* dst, const char* src) {
    char* d = dst;
    while ((*d++ = *src++));
    return dst;
}

static inline char* strcat(char* dst, const char* src) {
    char* d = dst;
    while (*d) d++;
    while ((*d++ = *src++));
    return dst;
}

static inline const char* strchr(const char* s, int c) {
    while (*s) {
        if (*s == (char)c) return s;
        s++;
    }
    return NULL;
}

// Sayıyı stringe dönüştür
static inline void itoa(int val, char* buf, int base) {
    char tmp[32];
    int i = 0, neg = 0;
    if (val < 0 && base == 10) { neg = 1; val = -val; }
    if (val == 0) { buf[0]='0'; buf[1]=0; return; }
    while (val > 0) {
        int r = val % base;
        tmp[i++] = (r < 10) ? '0'+r : 'a'+r-10;
        val /= base;
    }
    if (neg) tmp[i++] = '-';
    int j = 0;
    while (i > 0) buf[j++] = tmp[--i];
    buf[j] = 0;
}

static inline void utoa_hex(uint32_t val, char* buf) {
    buf[0]='0'; buf[1]='x';
    for (int i = 9; i >= 2; i--) {
        int nibble = val & 0xF;
        buf[i] = (nibble < 10) ? '0'+nibble : 'a'+nibble-10;
        val >>= 4;
    }
    buf[10] = 0;
}
