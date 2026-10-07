void timer_init();
void timer_irq_top_half();
void timer_irq_bottom_half();
void add_timeout_task(char* msg, int after);
void enable_uptime();
void disable_uptime();
