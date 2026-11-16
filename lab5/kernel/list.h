#pragma once

typedef struct list_head {
    struct list_head* prev;
    struct list_head* next;
} list_head;

#define LIST_HEAD(head) list_head head = { &(head), &(head) }

#define container_of(ptr, type, member)                                        \
    ((type*)((char*)(ptr) - __builtin_offsetof(type, member)))

#define list_entry(node, type, member) container_of(node, type, member)

#define list_first_entry(head, type, member)                                   \
    list_entry((head)->next, type, member)

#define list_last_entry(head, type, member)                                    \
    list_entry((head)->prev, type, member)

#define list_for_each(node, head)                                              \
    for (node = (head)->next; node != (head); node = node->next)

#define list_for_each_entry(entry, head, member)                               \
    for (entry = list_entry((head)->next, typeof(*entry), member);             \
        &entry->member != (head);                                              \
        entry = list_entry(entry->member.next, typeof(*entry), member))

#define list_for_each_safe(node, safe, head)                                   \
    for (node = (head)->next, safe = node->next; node != (head);               \
        node = safe, safe = node->next)

#define list_for_each_entry_safe(entry, safe, head, member)                    \
    for (entry = list_entry((head)->next, typeof(*entry), member),             \
        safe = list_entry(entry->member.next, typeof(*entry), member);         \
        &entry->member != (head); entry = safe,                                \
        safe = list_entry(safe->member.next, typeof(*entry), member))

static inline void INIT_LIST_HEAD(list_head* head)
{
    head->next = head;
    head->prev = head;
}

static inline int list_empty(list_head* head) { return head->next == head; }

static inline int list_size(list_head* head)
{
    if (!head)
        return 0;
    int ret = 0;
    list_head* node;
    list_for_each(node, head) ret++;
    return ret;
}

static inline void list_add(list_head* node, list_head* head)
{
    list_head* next = head->next;

    head->next = node;
    node->prev = head;
    node->next = next;
    next->prev = node;
}

static inline void list_add_tail(list_head* node, list_head* head)
{

    list_head* prev = head->prev;

    head->prev = node;
    node->next = head;
    node->prev = prev;
    prev->next = node;
}

static inline void list_del(list_head* node)
{
    list_head* prev = node->prev;
    list_head* next = node->next;
    prev->next = next;
    next->prev = prev;
}

static inline void list_move(list_head* node, list_head* head)
{
    list_del(node);
    list_add(node, head);
}

static inline void list_move_tail(list_head* node, list_head* head)
{
    list_del(node);
    list_add_tail(node, head);
}
