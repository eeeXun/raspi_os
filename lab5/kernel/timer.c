#include "exception.h"
#include "gic.h"
#include "list.h"
#include "memory.h"
#include "register.h"
#include "string.h"
#include "task.h"
#include "uart.h"

typedef struct {
    char* message;
    unsigned long long created_time;
    list_head node;
} timeout_data_t;

typedef struct {
    void* callback_data;
    void (*callback)(void*);
    unsigned long long expire;
    list_head node;
} timer_task_t;

int uptime_enabled = 0;
list_head timer_task_head;

void timer_init()
{
    INIT_LIST_HEAD(&timer_task_head);
    // Allow EL0 to read cntpct_el0 and cntfrq_el0
    write_reg(cntkctl_el1, read_reg(cntkctl_el1) | 0b1);
    gic_enable(INTID_TIMER);
}

// Set timer to the head's expire
void reset_timer()
{
    if (list_empty(&timer_task_head)) {
        // [0] = 0, disable timer
        write_reg(cntp_ctl_el0, 0);
        return;
    }
    write_reg(cntp_cval_el0,
        list_first_entry(&timer_task_head, timer_task_t, node)->expire);
    // [1] = 0, Not masked timer interrupt by IMASK bit
    // [0] = 1, enable timer
    write_reg(cntp_ctl_el0, 1);
}

unsigned long long get_uptime()
{
    return read_reg(cntpct_el0) / read_reg(cntfrq_el0);
}

void timer_irq_top_half() { return; }

void timer_irq_bottom_half()
{
    timer_task_t *entry, *safe;
    unsigned long long current_time = read_reg(cntpct_el0);
    list_for_each_entry_safe(entry, safe, &timer_task_head, node)
    {
        if (entry->expire > current_time)
            break;
        entry->callback(entry->callback_data);
        list_del(&entry->node);
        kfree(entry);
    }
    reset_timer();
    gic_enable(INTID_TIMER);
}

timer_task_t* add_timer(void* callback, void* data, int after)
{
    if (after <= 0) {
        uart_puts("add_timer must set after greater than 0\n");
        return 0;
    }

    unsigned long long daif_state = irq_save();

    timer_task_t* task = kmalloc(sizeof(timer_task_t));
    task->callback = callback;
    task->callback_data = data;
    task->expire = read_reg(cntpct_el0) + read_reg(cntfrq_el0) * after;
    timer_task_t* entry;
    list_for_each_entry(entry, &timer_task_head, node)
    {
        if (task->expire < entry->expire)
            break;
    }
    list_add_tail(&task->node, &entry->node);
    if (list_first_entry(&timer_task_head, timer_task_t, node) == task)
        reset_timer();

    irq_restore(daif_state);
    return task;
}

void timeout_callback(void* data)
{
    timeout_data_t* _data = (timeout_data_t*)data;
    uart_puts("\n[setTimeout] ");
    uart_puts(_data->message);
    uart_puts(", current time: ");
    uart_put_dec(get_uptime());
    uart_puts("s, created time: ");
    uart_put_dec(_data->created_time);
    uart_puts("s\n");
    kfree(_data->message);
    kfree(_data);
}

void add_timeout_task(char* msg, int after)
{
    timeout_data_t* data = kmalloc(sizeof(timeout_data_t));
    int msg_len = strlen(msg);
    data->message = kmalloc(msg_len + 1);
    strncpy(data->message, msg, msg_len + 1);
    data->created_time = get_uptime();
    add_timer(timeout_callback, (void*)data, after);
}

void uptime_callback(void* data)
{
    if (!uptime_enabled)
        return;
    uart_puts("\n[timer] uptime ");
    uart_put_dec(get_uptime());
    uart_puts("s\n");
    add_timer(uptime_callback, 0, 2);
}

void slow_timer_callback(void* data)
{
    uart_puts_poll("[slow] start, type something now\n");

    unsigned long long end = read_reg(cntpct_el0) + read_reg(cntfrq_el0) * 5;
    while (read_reg(cntpct_el0) < end)
        ;

    uart_puts_poll("[slow] end\n");
    set_task_trace(0);
}

void enable_uptime()
{
    if (uptime_enabled)
        return;
    add_timer(uptime_callback, 0, 2);
    uptime_enabled = 1;
}

void disable_uptime() { uptime_enabled = 0; }
