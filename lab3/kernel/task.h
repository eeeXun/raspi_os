#include "utils.h"

enum { PRIO_UART = 1, PRIO_TIMER = 2, PRIO_NONE = INT_MAX };

void set_task_trace(int status);
void task_add(void (*callback)(), int priority);
void task_run();
