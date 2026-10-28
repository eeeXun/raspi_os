#include "memory.h"
#include "list.h"
#include "uart.h"

typedef struct mem_range {
    unsigned long long start;
    unsigned long long end;
} mem_range;

typedef enum {
    FRAME_FREE,
    FRAME_ALLOCATED,
    FRAME_CHUNKED,
    FRAME_BODY,
    FRAME_RESERVED
} frame_status;

typedef struct frame_t {
    frame_status status;
    int order;
    int chunk_size_idx;
    int free_chunk_count;
    list_head buddy_list;
    list_head chunked_list;
    list_head free_chunks;
} frame_t;

typedef struct chunk_t {
    list_head node;
} chunk_t;

#define FRAME_SIZE 0x1000
#define FRAME_MAX_ORDER 10
#define FRAME_NUM ((FRAME_END - FRAME_START) / FRAME_SIZE)

int chunk_sizes[] = { 32, 64, 128, 256, 512, 1024, 2048 };
#define CHUNK_SIZE_NUM (sizeof(chunk_sizes) / sizeof(int))

#define RESERVED_MAX_COUNT 16
int reserved_count = 0;
mem_range reserved[RESERVED_MAX_COUNT];

unsigned long long frame_start;
frame_t frames[FRAME_NUM];
list_head buddy_freelist[FRAME_MAX_ORDER + 1];
list_head chunk_freelist[CHUNK_SIZE_NUM];

int frame_idx(frame_t* frame) { return frame - frames; }

unsigned long long idx_to_frame_addr(int idx)
{
    return frame_start + (unsigned long long)idx * FRAME_SIZE;
}

int address_to_frame_idx(unsigned long long address)
{
    if (address < frame_start
        || address >= frame_start + FRAME_NUM * FRAME_SIZE)
        return -1;
    return (address - frame_start) / FRAME_SIZE;
}

void memory_reserve(unsigned long long start, unsigned long long end)
{
    if (reserved_count >= RESERVED_MAX_COUNT) {
        uart_puts("No more space to reserved memory!\n");
        return;
    }
    if (start >= end) {
        uart_puts("start must greater than end!\n");
        return;
    }
    start = start & ~(FRAME_SIZE - 1);
    end = (end + FRAME_SIZE - 1) & ~(FRAME_SIZE - 1);
    int i = reserved_count++;
    while (i > 0 && reserved[i - 1].start > start) {
        reserved[i] = reserved[i - 1];
        i--;
    }
    reserved[i] = (mem_range) { start, end };
}

void buddy_info()
{
    for (int i = 0; i < CHUNK_SIZE_NUM; i++) {
        int count = 0;
        frame_t* frame;
        list_for_each_entry(frame, &chunk_freelist[i], chunked_list) count
            += frame->free_chunk_count;
        uart_puts("chunk ");
        uart_put_dec(chunk_sizes[i]);
        uart_puts(": ");
        uart_put_dec(count);
        uart_put('\n');
    }
    for (int order = 0; order <= FRAME_MAX_ORDER; order++) {
        uart_puts("order ");
        uart_put_dec(order);
        uart_puts(": ");
        uart_put_dec(list_size(&buddy_freelist[order]));
        uart_put('\n');
    }
}

int clamp_idx(unsigned long long address, unsigned long long start,
    unsigned long long end)
{
    if (address < start)
        address = start;
    if (address > end)
        address = end;
    return (address - start) / FRAME_SIZE;
}

void add_free_range(int idx, int end_idx)
{
    while (idx < end_idx) {
        int block_size = end_idx - idx;
        int order = FRAME_MAX_ORDER;
        while (order > 0
            && ((idx % (1 << order)) // not align
                || (1 << order) > block_size))
            order--;
        frames[idx].status = FRAME_FREE;
        frames[idx].order = order;
        list_add_tail(&frames[idx].buddy_list, &buddy_freelist[order]);
        idx += (1 << order);
    }
}

void buddy_init(unsigned long long start, unsigned long long end)
{
    frame_start = start;

    for (int i = 0; i <= FRAME_MAX_ORDER; i++)
        INIT_LIST_HEAD(&buddy_freelist[i]);
    for (int i = 0; i < CHUNK_SIZE_NUM; i++)
        INIT_LIST_HEAD(&chunk_freelist[i]);
    for (int i = 0; i < FRAME_NUM; i++) {
        frames[i].status = FRAME_BODY;
        INIT_LIST_HEAD(&frames[i].free_chunks);
    }
    int current_idx = 0;
    for (int i = 0; i < reserved_count; i++) {
        int reserved_start_idx = clamp_idx(reserved[i].start, start, end);
        int reserved_end_idx = clamp_idx(reserved[i].end, start, end);
        add_free_range(current_idx, reserved_start_idx);
        for (int j = reserved_start_idx; j < reserved_end_idx; j++)
            frames[j].status = FRAME_RESERVED;
        if (reserved_end_idx > current_idx)
            current_idx = reserved_end_idx;
    }
    int end_idx = (end - start) / FRAME_SIZE;
    add_free_range(current_idx, end_idx);
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
        && list_empty(&buddy_freelist[current_order]))
        current_order++;
    if (current_order > FRAME_MAX_ORDER) {
        uart_puts("No available page with ");
        uart_put_dec(order);
        uart_puts(" order to allocate\n");
        return 0;
    }
    frame_t* free_frame
        = list_first_entry(&buddy_freelist[current_order], frame_t, buddy_list);
    int idx = frame_idx(free_frame);
    void* ptr = (void*)idx_to_frame_addr(idx);

    list_del(&frames[idx].buddy_list);
    while (current_order > order) {
        current_order--;
        int buddy_idx = idx + (1 << current_order);
        frames[buddy_idx].order = current_order;
        frames[buddy_idx].status = FRAME_FREE;
        list_add(&frames[buddy_idx].buddy_list, &buddy_freelist[current_order]);
    }
    frames[idx].order = order;
    frames[idx].status = FRAME_ALLOCATED;
    return ptr;
}

void __free_page(int frame_idx)
{
    if (frame_idx < 0 || frame_idx >= FRAME_NUM) {
        uart_puts("Invalid frame idx, ");
        uart_put_dec(frame_idx);
        uart_puts("!\n");
        return;
    }
    int order = frames[frame_idx].order;
    frames[frame_idx].status = FRAME_FREE;
    list_add(&frames[frame_idx].buddy_list, &buddy_freelist[order]);
    while (order < FRAME_MAX_ORDER) {
        int buddy_idx = frame_idx | (1 << order); // higher address
        frame_idx = frame_idx & (~(1 << order)); // lower address
        if (buddy_idx >= FRAME_NUM || frames[frame_idx].status != FRAME_FREE
            || frames[buddy_idx].status != FRAME_FREE
            || frames[frame_idx].order != order
            || frames[buddy_idx].order != order)
            break;
        frames[buddy_idx].status = FRAME_BODY;
        list_del(&frames[buddy_idx].buddy_list);
        list_del(&frames[frame_idx].buddy_list);
        order++;
        frames[frame_idx].order = order;
        list_add(&frames[frame_idx].buddy_list, &buddy_freelist[order]);
    }
}

void free_page(unsigned long long address)
{
    int idx = address_to_frame_idx(address);
    if ((idx < 0) || (address % FRAME_SIZE)) {
        uart_puts("Invalid address!\n");
        return;
    }
    if (frames[idx].status != FRAME_ALLOCATED) {
        uart_puts("Page ");
        uart_put_dec(idx);
        uart_puts(" is not allocated!\n");
        return;
    }
    __free_page(idx);
}

void alloc_chunk_page(int chunk_size_idx)
{
    void* address = alloc_page(0);
    if (!address)
        return;
    int frame_idx = address_to_frame_idx((unsigned long long)address);
    frame_t* chunked_page = &frames[frame_idx];
    chunked_page->status = FRAME_CHUNKED;
    chunked_page->chunk_size_idx = chunk_size_idx;
    chunked_page->free_chunk_count = FRAME_SIZE / chunk_sizes[chunk_size_idx];
    for (int i = 0; i < FRAME_SIZE / chunk_sizes[chunk_size_idx]; i++) {
        list_add_tail(
            &((chunk_t*)((char*)address + i * chunk_sizes[chunk_size_idx]))
                ->node,
            &chunked_page->free_chunks);
    }
    list_add(&chunked_page->chunked_list, &chunk_freelist[chunk_size_idx]);
}

void* alloc_chunk(int size)
{
    if (size > chunk_sizes[CHUNK_SIZE_NUM - 1]) {
        uart_puts("Size too large, use alloc_page instead!\n");
        return 0;
    }
    int chunk_size_idx = 0;
    for (; chunk_size_idx < CHUNK_SIZE_NUM; chunk_size_idx++) {
        if (chunk_sizes[chunk_size_idx] >= size)
            break;
    }
    if (list_empty(&chunk_freelist[chunk_size_idx]))
        alloc_chunk_page(chunk_size_idx);
    if (list_empty(&chunk_freelist[chunk_size_idx]))
        return 0;
    frame_t* chunked_page = list_first_entry(
        &chunk_freelist[chunk_size_idx], frame_t, chunked_list);
    chunk_t* chunk
        = list_first_entry(&chunked_page->free_chunks, chunk_t, node);
    list_del(&chunk->node);
    chunked_page->free_chunk_count--;
    if (!chunked_page->free_chunk_count)
        list_del(&chunked_page->chunked_list);
    return (void*)chunk;
}

void free_chunk(unsigned long long address)
{
    int frame_idx = address_to_frame_idx(address);
    if (frame_idx < 0) {
        uart_puts("Invalid address!\n");
        return;
    }
    if (frames[frame_idx].status != FRAME_CHUNKED) {
        uart_puts("Address 0x");
        uart_put_hex(address);
        uart_puts(", page ");
        uart_put_dec(frame_idx);
        uart_puts(" is not chunked!\n");
        return;
    }
    if (address % chunk_sizes[frames[frame_idx].chunk_size_idx]) {
        uart_puts("Invalid address!\n");
        return;
    }
    frame_t* chunked_page = &frames[frame_idx];
    list_add(&((chunk_t*)((char*)address))->node, &chunked_page->free_chunks);
    chunked_page->free_chunk_count++;
    if (chunked_page->free_chunk_count == 1) {
        list_add(&chunked_page->chunked_list,
            &chunk_freelist[chunked_page->chunk_size_idx]);
    } else if (chunked_page->free_chunk_count
        == FRAME_SIZE / chunk_sizes[chunked_page->chunk_size_idx]) {
        list_del(&chunked_page->chunked_list);
        INIT_LIST_HEAD(&chunked_page->free_chunks);
        __free_page(frame_idx);
    }
}

void* kmalloc(int size)
{
    if (size <= 0) {
        uart_puts("kmalloc size must greater than 0!\n");
        return 0;
    }
    if (size > (FRAME_SIZE << FRAME_MAX_ORDER)) {
        uart_puts("kmalloc size too large!\n");
        return 0;
    }
    if (size <= chunk_sizes[CHUNK_SIZE_NUM - 1])
        return alloc_chunk(size);
    int order = 0;
    while ((FRAME_SIZE << order) < size)
        order++;
    return alloc_page(order);
}

void kfree(void* ptr)
{
    unsigned long long address = (unsigned long long)ptr;
    int idx = address_to_frame_idx(address);
    if (idx < 0) {
        uart_puts("Invalid address!\n");
        return;
    }
    switch (frames[idx].status) {
    case FRAME_ALLOCATED:
        free_page(address);
        break;
    case FRAME_CHUNKED:
        free_chunk(address);
        break;
    default:
        uart_puts("Can not free address 0x");
        uart_put_hex(address);
        uart_puts("!\n");
        break;
    }
}
