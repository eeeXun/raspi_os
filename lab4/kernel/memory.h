#pragma once

#include "list.h"
#define FRAME_START 0x10000000
#define FRAME_END 0x20000000

void memory_reserve(unsigned long long start, unsigned long long end);
void buddy_info();
void buddy_init(unsigned long long start, unsigned long long end);

void* kmalloc(int size);
void kfree(void* ptr);
