#include "heap.h"
#include <string.h>

/* Basit first-fit heap
   Her blok: [BlockHeader | kullanıcı verisi]
   Header: size (bayt), free (1=boş, 0=dolu) */

struct BlockHeader {
    size_t size;
    bool   free;
    BlockHeader* next;
};

static uint8_t* heap_start = NULL;
static size_t   heap_total = 0;
static BlockHeader* heap_head = NULL;

void heap_init(uint32_t start, uint32_t size) {
    heap_start = (uint8_t*)start;
    heap_total = size;
    heap_head  = (BlockHeader*)heap_start;
    heap_head->size = size - sizeof(BlockHeader);
    heap_head->free = true;
    heap_head->next = NULL;
}

void* kmalloc(size_t size) {
    if (!size) return NULL;
    // 8-bayt hizala
    size = (size + 7) & ~7u;

    BlockHeader* cur = heap_head;
    while (cur) {
        if (cur->free && cur->size >= size) {
            // Bölünebilir mi?
            if (cur->size >= size + sizeof(BlockHeader) + 8) {
                BlockHeader* next = (BlockHeader*)((uint8_t*)cur + sizeof(BlockHeader) + size);
                next->size = cur->size - size - sizeof(BlockHeader);
                next->free = true;
                next->next = cur->next;
                cur->size  = size;
                cur->next  = next;
            }
            cur->free = false;
            return (void*)((uint8_t*)cur + sizeof(BlockHeader));
        }
        cur = cur->next;
    }
    return NULL;  // OOM
}

void kfree(void* ptr) {
    if (!ptr) return;
    BlockHeader* hdr = (BlockHeader*)((uint8_t*)ptr - sizeof(BlockHeader));
    hdr->free = true;

    // Sağdaki blokla birleştir
    while (hdr->next && hdr->next->free) {
        hdr->size += sizeof(BlockHeader) + hdr->next->size;
        hdr->next  = hdr->next->next;
    }
}

void* operator new(size_t size)   { return kmalloc(size); }
void* operator new[](size_t size) { return kmalloc(size); }
void  operator delete(void* p) noexcept   { kfree(p); }
void  operator delete[](void* p) noexcept { kfree(p); }
void  operator delete(void* p, size_t) noexcept   { kfree(p); }
void  operator delete[](void* p, size_t) noexcept { kfree(p); }
