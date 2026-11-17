#include "exception.h"
#include "register.h"
#include "thread.h"
#include "timer.h"

extern void switch_to(thread_t* current, thread_t* next);

int need_reschedule = 0;

void sched_tick(void* data)
{
    need_reschedule = 1;
    add_timer(sched_tick, 0, read_reg(cntfrq_el0) >> 5);
}

void schedule()
{
    unsigned long long daif_state = irq_save();

    thread_t* current = get_current_thread();
    if (current->status != DEAD)
        list_move_tail(&current->list_node, &run_queue);
    thread_t* next = list_first_entry(&run_queue, thread_t, list_node);
    if (current != next)
        switch_to(current, next);

    irq_restore(daif_state);
}
