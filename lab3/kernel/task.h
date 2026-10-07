enum { PRIO_UART = 1, PRIO_TIMER = 2 };

void task_add(void (*callback)(), int priority);
void task_run();
