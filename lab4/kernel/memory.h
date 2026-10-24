#pragma once

#include "list.h"
#define FRAME_START 0x10000000
#define FRAME_END 0x20000000

typedef enum { FRAME_FREE, FRAME_ALLOCATED, FRAME_BODY } frame_status;

typedef struct frame_t {
    frame_status status;
    int order;
    list_head chain;
} frame_t;

void buddy_info();
void buddy_init(unsigned long long start, unsigned long long end);

void* alloc_page(int order);
void free_page(unsigned long long address);
