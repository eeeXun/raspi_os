#pragma once

#include "list.h"

void memory_init(void* dtb_base);

void buddy_info();
void buddy_init();

void* kmalloc(int size);
void kfree(void* ptr);
