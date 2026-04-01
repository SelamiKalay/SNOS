#include "ramdisk.h"
#include <string.h>

static RdFile files[RD_MAX_FILES];

void rd_init() {
    for (int i = 0; i < RD_MAX_FILES; i++) {
        files[i].used   = false;
        files[i].is_dir = false;
        files[i].size   = 0;
        files[i].name[0]= '\0';
        files[i].data[0]= '\0';
    }
    // Varsayılan dizin ve dosyalar
    RdFile* f;
    f = rd_create("Documents", true);
    f = rd_create("readme.txt", false);
    if (f) rd_write(f,
        "SNOS v0.1\n"
        "Gelistirici: Selami\n"
        "Bu bir test belgesidir.\n", 0);
}

RdFile* rd_create(const char* name, bool is_dir) {
    for (int i = 0; i < RD_MAX_FILES; i++) {
        if (!files[i].used) {
            files[i].used   = true;
            files[i].is_dir = is_dir;
            files[i].size   = 0;
            files[i].data[0]= '\0';
            strcpy(files[i].name, name);
            return &files[i];
        }
    }
    return NULL;  // Disk dolu
}

RdFile* rd_find(const char* name) {
    for (int i = 0; i < RD_MAX_FILES; i++)
        if (files[i].used && strcmp(files[i].name, name) == 0)
            return &files[i];
    return NULL;
}

bool rd_delete(const char* name) {
    RdFile* f = rd_find(name);
    if (!f) return false;
    f->used = false;
    return true;
}

int rd_list(const char** out_names, int max) {
    int count = 0;
    for (int i = 0; i < RD_MAX_FILES && count < max; i++)
        if (files[i].used)
            out_names[count++] = files[i].name;
    return count;
}

void rd_write(RdFile* f, const char* data, uint32_t len) {
    if (!f) return;
    if (len == 0) len = (uint32_t)strlen(data);
    if (len > RD_MAX_DATA - 1) len = RD_MAX_DATA - 1;
    memcpy(f->data, data, len);
    f->data[len] = '\0';
    f->size = len;
}
