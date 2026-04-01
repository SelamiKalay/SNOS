#pragma once
#include <stdint.h>
void  heap_init(uint32_t start, uint32_t size);
void* kmalloc(size_t size);
void  kfree(void* ptr);

// Global new / delete
void* operator new(size_t size);
void* operator new[](size_t size);
void  operator delete(void* ptr) noexcept;
void  operator delete[](void* ptr) noexcept;
void  operator delete(void* ptr, size_t) noexcept;
void  operator delete[](void* ptr, size_t) noexcept;
