#include "task.h"
#include "exception.h"
#include "list.h"
#include "memory.h"
#include "uart.h"

typedef struct {
    int priority;
    void (*callback)();
    list_head node;
} task_t;

int trace_task = 0;
LIST_HEAD(task_head);
volatile int current_priority = PRIO_NONE;

void set_task_trace(int status) { trace_task = status; }

// Only called from the top half in irq_handler, where the hardware has already
// masked DAIF. Add irq_save/irq_restore if this ever gets called from a bottom
// half.
void task_add(void (*callback)(), int priority)
{
    task_t* new_task = kmalloc(sizeof(task_t));
    new_task->callback = callback;
    new_task->priority = priority;
    task_t* entry;
    list_for_each_entry(entry, &task_head, node)
    {
        if (new_task->priority < entry->priority)
            break;
    }
    list_add_tail(&new_task->node, &entry->node);
}

void task_run()
{
    while (!list_empty(&task_head)) {
        task_t* task = list_first_entry(&task_head, task_t, node);
        if (task->priority >= current_priority)
            return;
        list_del(&task->node);
        void (*callback)() = task->callback;
        int priority = task->priority;
        kfree(task);

        int previous_priority = current_priority;
        current_priority = priority;
        if (trace_task && previous_priority != PRIO_NONE) {
            uart_puts_poll("[task] enter prio ");
            uart_put_dec_poll(priority);
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
