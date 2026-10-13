typedef struct timer_task timer_task;

void timer_init();
void timer_irq_top_half();
void timer_irq_bottom_half();
timer_task* add_timer(void* callback, void* data, int after);
void add_timeout_task(char* msg, int after);
void slow_timer_callback(void* data);
void enable_uptime();
void disable_uptime();
