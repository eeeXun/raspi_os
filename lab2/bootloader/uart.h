void uart_init();
unsigned char uart_getb();
char uart_getc();
void uart_put(char c);
void uart_put_dec(unsigned long long num);
void uart_put_hex(unsigned long long num);
void uart_puts(char* buffer);
