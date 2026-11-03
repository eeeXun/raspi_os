#pragma once

typedef struct list_head {
    struct list_head* prev;
    struct list_head* next;
} list_head;

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

void INIT_LIST_HEAD(list_head* head);
int list_empty(list_head* head);
int list_size(list_head* head);
void list_add(list_head* node, list_head* head);
void list_add_tail(list_head* node, list_head* head);
void list_del(list_head* node);
