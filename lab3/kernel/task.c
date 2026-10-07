#include "exception.h"
#include "uart.h"

#define QUEUE_SIZE 20
typedef struct task {
    int used;
    int priority;
    void (*callback)();
    struct task* next;
} task;
task task_pool[QUEUE_SIZE];
task* task_head;

// Only called from the top half in irq_handler, where the hardware has already
// masked DAIF. Add irq_save/irq_restore if this ever gets called from a bottom
// half.
void task_add(void (*callback)(), int priority)
{
    task* new_task = 0;
    for (int i = 0; i < QUEUE_SIZE; i++) {
        if (task_pool[i].used)
            continue;
        new_task = &task_pool[i];
        break;
    }
    if (!new_task) {
        uart_puts("\nInterrupt task queue is full!\n");
        return;
    }

    new_task->used = 1;
    new_task->callback = callback;
    new_task->priority = priority;

    task** current = &task_head;
    while (*current && priority >= (*current)->priority)
        current = &(*current)->next;
    new_task->next = *current;
    *current = new_task;
}

void task_run()
{
    task* t;
    while (1) {
        t = task_head;
        if (!t)
            return;
        task_head = t->next;
        enable_interrupt();
        t->callback();
        t->used = 0;
        disable_interrupt();
    }
}
