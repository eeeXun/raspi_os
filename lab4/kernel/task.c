#include "task.h"
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
task* task_head = 0;

int trace_task = 0;
volatile int current_priority = PRIO_NONE;

void set_task_trace(int status) { trace_task = status; }

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
    while (1) {
        task* t = task_head;
        if (!t || t->priority >= current_priority)
            return;
        task_head = t->next;
        void (*callback)() = t->callback;
        t->used = 0;

        int previous_priority = current_priority;
        current_priority = t->priority;
        if (trace_task && previous_priority != PRIO_NONE) {
            uart_puts_poll("[task] enter prio ");
            uart_put_dec_poll(t->priority);
            uart_puts_poll(", preempting ");
            uart_put_dec_poll(previous_priority);
            uart_put_poll('\n');
        }

        enable_interrupt();
        callback();
        disable_interrupt();

        current_priority = previous_priority;
    }
}
