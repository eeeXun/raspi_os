#include "memory.h"
#include "list.h"
#include "uart.h"

#define FRAME_BASE_SIZE 0x1000
#define FRAME_MAX_ORDER 10
#define FRAME_NUM ((FRAME_END - FRAME_START) / FRAME_BASE_SIZE)

unsigned long long frame_base;
frame_t frames[FRAME_NUM];
list_head frame_freelist[FRAME_MAX_ORDER + 1];

int frame_idx(frame_t* frame) { return frame - frames; }

unsigned long long idx_to_frame_addr(int idx)
{
    return frame_base + (unsigned long long)idx * FRAME_BASE_SIZE;
}

int frame_addr_to_idx(unsigned long long address)
{
    if (address < frame_base
        || address >= frame_base + FRAME_NUM * FRAME_BASE_SIZE
        || (address % FRAME_BASE_SIZE) != 0)
        return -1;
    return (address - frame_base) / FRAME_BASE_SIZE;
}

void buddy_info()
{
    for (int order = 0; order <= FRAME_MAX_ORDER; order++) {
        uart_puts("order ");
        uart_put_dec(order);
        uart_puts(": ");
        uart_put_dec(list_size(&frame_freelist[order]));
        uart_put('\n');
    }
}

void buddy_init(unsigned long long start, unsigned long long end)
{
    frame_base = start;
    unsigned long long size = end - start;

    for (int i = 0; i <= FRAME_MAX_ORDER; i++)
        INIT_LIST_HEAD(&frame_freelist[i]);
    for (int i = 0; i < FRAME_NUM; i++)
        frames[i].status = FRAME_BODY;
    for (int order = FRAME_MAX_ORDER, idx = 0; order >= 0 && size > 0;
        order--) {
        unsigned long long block_size = (unsigned long long)FRAME_BASE_SIZE
            << order;
        while (size >= block_size) {
            frames[idx].order = order;
            frames[idx].status = FRAME_FREE;
            list_add_tail(&frames[idx].chain, &frame_freelist[order]);
            size -= block_size;
            idx += (1 << order);
        }
    }
}

void* alloc_page(int order)
{
    if (order < 0 || order > FRAME_MAX_ORDER) {
        uart_puts("Can't allocate page with order ");
        uart_put_dec(order);
        uart_put('\n');
        return 0;
    }
    int current_order = order;
    while (current_order <= FRAME_MAX_ORDER
        && list_empty(&frame_freelist[current_order]))
        current_order++;
    if (current_order > FRAME_MAX_ORDER) {
        uart_puts("No available page with ");
        uart_put_dec(order);
        uart_puts(" order to allocate\n");
        return 0;
    }
    frame_t* free_frame
        = list_first_entry(&frame_freelist[current_order], frame_t, chain);
    int idx = frame_idx(free_frame);
    void* ret = (void*)idx_to_frame_addr(idx);

    list_del(&frames[idx].chain);
    while (current_order > order) {
        current_order--;
        int buddy_idx = idx + (1 << current_order);
        frames[buddy_idx].order = current_order;
        frames[buddy_idx].status = FRAME_FREE;
        list_add(&frames[buddy_idx].chain, &frame_freelist[current_order]);
    }
    frames[idx].order = order;
    frames[idx].status = FRAME_ALLOCATED;
    return ret;
}

void free_page(unsigned long long address)
{
    int idx = frame_addr_to_idx(address);
    if (idx < 0) {
        uart_puts("Invalid address!\n");
        return;
    }
    if (frames[idx].status != FRAME_ALLOCATED) {
        uart_puts("Page ");
        uart_put_dec(idx);
        uart_puts("is not allocated!\n");
        return;
    }
    int order = frames[idx].order;
    frames[idx].status = FRAME_FREE;
    list_add(&frames[idx].chain, &frame_freelist[order]);
    while (order < FRAME_MAX_ORDER) {
        int buddy_idx = idx | (1 << order); // higher address
        idx = idx & (~(1 << order)); // lower address
        if (buddy_idx >= FRAME_NUM || frames[idx].status != FRAME_FREE
            || frames[buddy_idx].status != FRAME_FREE
            || frames[idx].order != order || frames[buddy_idx].order != order)
            break;
        frames[buddy_idx].status = FRAME_BODY;
        list_del(&frames[buddy_idx].chain);
        list_del(&frames[idx].chain);
        order++;
        frames[idx].order = order;
        list_add(&frames[idx].chain, &frame_freelist[order]);
    }
}
