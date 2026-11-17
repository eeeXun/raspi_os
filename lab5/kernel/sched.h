#pragma once

extern int need_reschedule;

void sched_tick(void* data);
void schedule();
