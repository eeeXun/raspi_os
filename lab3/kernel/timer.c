#include "exception.h"
#include "gic.h"
#include "register.h"
#include "string.h"
#include "uart.h"

int uptime_enabled;

#define QUEUE_SIZE 50
#define TIMEOUT_MSG_LEN 50

typedef struct {
    int used;
    char message[TIMEOUT_MSG_LEN + 1];
    unsigned long long created_time;
} timeout_data;
timeout_data timeout_data_pool[QUEUE_SIZE];

typedef struct timer_task {
    int used;
    void* callback_data;
    void (*callback)(void*);
    unsigned long long expire;
    struct timer_task* next;
} timer_task;
timer_task task_queue[QUEUE_SIZE];
timer_task* head;

void timer_init()
{
    uptime_enabled = 0;
    gic_enable(INTID_TIMER);
}

// Set timer to the head's expire
void reset_timer()
{
    if (!head) {
        // [0] = 0, disable timer
        write_reg(cntp_ctl_el0, 0);
        return;
    }
    write_reg(cntp_cval_el0, head->expire);
    // [1] = 0, Not masked timer interrupt by IMASK bit
    // [0] = 1, enable timer
    write_reg(cntp_ctl_el0, 1);
}

unsigned long long get_uptime()
{
    return read_reg(cntpct_el0) / read_reg(cntfrq_el0);
}

void timer_irq_top_half() { return; }

void timer_irq_bottom_half() {
    timer_task* task;
    while (head) {
        unsigned long long current_time = read_reg(cntpct_el0);
        if (head->expire > current_time)
            break;
        head->callback(head->callback_data);
        task = head;
        head = head->next;
        task->used = 0;
    }
    reset_timer();
    gic_enable(INTID_TIMER);
}

timer_task* add_timer(void* callback, void* data, int after)
{
    if (after <= 0) {
        uart_puts("add_timer must set after greater than 0\n");
        return 0;
    }

    unsigned long long daif_state = irq_save();

    timer_task* task = 0;
    for (int i = 0; i < QUEUE_SIZE; i++) {
        if (task_queue[i].used)
            continue;
        task = &task_queue[i];
        break;
    }
    if (!task) {
        irq_restore(daif_state);
        uart_puts("No avaiable timer task to allocate\n");
        return 0;
    }
    task->used = 1;
    task->callback = callback;
    task->callback_data = data;
    task->expire = read_reg(cntpct_el0) + read_reg(cntfrq_el0) * after;
    timer_task** current = &head;
    while (*current && task->expire >= (*current)->expire)
        current = &(*current)->next;
    task->next = *current;
    *current = task;
    if (task == head)
        reset_timer();

    irq_restore(daif_state);
    return task;
}

void timeout_callback(void* data)
{
    timeout_data* _data = (timeout_data*)data;
    uart_puts("\n[setTimeout] ");
    uart_puts(_data->message);
    uart_puts(", current time: ");
    uart_put_dec(get_uptime());
    uart_puts("s, created time: ");
    uart_put_dec(_data->created_time);
    uart_puts("s\n");
    _data->used = 0;
}

void add_timeout_task(char* msg, int after)
{
    timeout_data* data = 0;
    timer_task* task = 0;
    for (int i = 0; i < QUEUE_SIZE; i++) {
        if (timeout_data_pool[i].used)
            continue;
        data = &timeout_data_pool[i];
        break;
    }
    if (!data) {
        uart_puts("No avaiable timeout data pool\n");
        return;
    }
    data->used = 1;
    strncpy(data->message, msg, TIMEOUT_MSG_LEN + 1);
    data->created_time = get_uptime();
    task = add_timer(timeout_callback, (void*)data, after);
    if (!task) {
        uart_puts("No avaiable timer task queue!\n");
        data->used = 0;
    }
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

void enable_uptime()
{
    if (uptime_enabled)
        return;
    timer_task* task = 0;
    task = add_timer(uptime_callback, 0, 2);
    if (!task) {
        uart_puts("No avaiable timer task queue! Try again later!\n");
        return;
    }
    uptime_enabled = 1;
}

void disable_uptime() { uptime_enabled = 0; }
