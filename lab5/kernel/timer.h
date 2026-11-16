#pragma once

#include "register.h"

void timer_init();
void timer_irq_top_half();
void timer_irq_bottom_half();
void add_timer(void* callback, void* data, unsigned long long tick);
void add_timeout_task(char* msg, int after);
void slow_timer_callback(void* data);
void enable_uptime();
void disable_uptime();

static inline unsigned long long sec2tick(int sec)
{
    return read_reg(cntfrq_el0) * sec;
}
