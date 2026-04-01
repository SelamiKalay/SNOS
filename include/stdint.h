#pragma once
// Freestanding ortam için temel tipler
typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;
typedef signed char        int8_t;
typedef signed short       int16_t;
typedef signed int         int32_t;
typedef signed long long   int64_t;
typedef __SIZE_TYPE__      size_t;
typedef __PTRDIFF_TYPE__   ptrdiff_t;
typedef __UINTPTR_TYPE__   uintptr_t;

#ifndef __cplusplus
#define NULL 0
#define true  1
#define false 0
typedef int bool;
#else
#define NULL nullptr
#endif
