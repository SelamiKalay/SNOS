#pragma once
#include <stdint.h>

/* =============================================================================
   Ramdisk – Bellekte basit dosya sistemi
   FAT32 yerine, kernel içine gömülü sade bir dizin/dosya ağacı.
   ============================================================================= */

#define RD_MAX_FILES  32
#define RD_MAX_NAME   64
#define RD_MAX_DATA   (8 * 1024)   // 8 KB / dosya

struct RdFile {
    char     name[RD_MAX_NAME];
    char     data[RD_MAX_DATA];
    uint32_t size;
    bool     used;
    bool     is_dir;
};

void     rd_init();
RdFile*  rd_create(const char* name, bool is_dir);
RdFile*  rd_find  (const char* name);
bool     rd_delete(const char* name);
int      rd_list  (const char** out_names, int max);  // Dosya isimlerini doldur
void     rd_write (RdFile* f, const char* data, uint32_t len);
