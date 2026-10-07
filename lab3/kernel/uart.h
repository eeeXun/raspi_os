void uart_init();
void enable_uart_interrupt();
void uart_irq_top_half();
void uart_irq_bottom_half();
char uart_getc();
void uart_put(char c);
void uart_put_dec(unsigned long long num);
void uart_put_hex(unsigned long long num);
void uart_puts(char* buffer);
