#include "list.h"

void INIT_LIST_HEAD(list_head* head)
{
    head->next = head;
    head->prev = head;
}

int list_empty(list_head* head) { return head->next == head; }

int list_size(list_head* head)
{
    if (!head)
        return 0;
    int ret = 0;
    list_head* node;
    list_for_each(node, head) ret++;
    return ret;
}

void list_add(list_head* node, list_head* head)
{
    list_head* next = head->next;

    head->next = node;
    node->prev = head;
    node->next = next;
    next->prev = node;
}

void list_add_tail(list_head* node, list_head* head)
{
    list_head* prev = head->prev;

    head->prev = node;
    node->next = head;
    node->prev = prev;
    prev->next = node;
}

void list_del(list_head* node)
{
    list_head* prev = node->prev;
    list_head* next = node->next;
    prev->next = next;
    next->prev = prev;
}
