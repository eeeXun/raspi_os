#include "gpio.h"
#include "mmio.h"

enum {
    AUX_BASE = PERIPHERAL_BASE + 0x215000,
    AUX_ENABLES = AUX_BASE + 0x4,
    AUX_MU_IO_REG = AUX_BASE + 0x40,
    AUX_MU_IER_REG = AUX_BASE + 0x44,
    AUX_MU_IIR_REG = AUX_BASE + 0x48,
    AUX_MU_LCR_REG = AUX_BASE + 0x4C,
    AUX_MU_MCR_REG = AUX_BASE + 0x50,
    AUX_MU_LSR_REG = AUX_BASE + 0x54,
    AUX_MU_MSR_REG = AUX_BASE + 0x58,
    AUX_MU_SCRATCH = AUX_BASE + 0x5C,
    AUX_MU_CNTL_REG = AUX_BASE + 0x60,
    AUX_MU_BAUD_REG = AUX_BASE + 0x68,
};

// Reference
// https://nycu-caslab.github.io/OSC2024/labs/hardware/uart.html#initialization
#define AUX_UART_CLOCK 500000000
#define AUX_MU_BAUD(baud) ((AUX_UART_CLOCK / (baud * 8)) - 1)

void uart_init()
{
    unsigned int val;

    mmio_write(AUX_ENABLES, 1); // enable UART1
    mmio_write(AUX_MU_CNTL_REG, 0); // disable RX/TX
    mmio_write(AUX_MU_LCR_REG, 3); // 8 bits
    mmio_write(AUX_MU_MCR_REG, 0); // don't need modem signals
    mmio_write(AUX_MU_IER_REG, 0); // disable interrupts
    mmio_write(AUX_MU_IIR_REG, 0xC6); // disable interrupts
    mmio_write(AUX_MU_BAUD_REG, AUX_MU_BAUD(115200)); // baud rate 115200

    val = mmio_read(GPFSEL1);
    val &= ~((0b111 << 12) | (0b111 << 15)); // clear gpio14, gpio15
    val |= ((0b010 << 12) | (0b010 << 15)); // alt5 (set gpio14, gpio15)
    mmio_write(GPFSEL1, val);

    val = mmio_read(GPIO_PUP_PDN_CNTRL_REG0);
    val &= ~((0b11 << 28) | (0b11 << 30)); // clear gpio14 and gpio15
    mmio_write(
        GPIO_PUP_PDN_CNTRL_REG0, val); // gpio14 and gpio15 already set to 0b00

    mmio_write(AUX_MU_CNTL_REG, 3); // enable RX/TX
}

unsigned int uart_read_ready() { return mmio_read(AUX_MU_LSR_REG) & 0x01; }
unsigned int uart_write_ready() { return mmio_read(AUX_MU_LSR_REG) & 0x20; }

unsigned char uart_getb()
{
    unsigned char b;
    while (!uart_read_ready())
        ;
    b = (unsigned char)(mmio_read(AUX_MU_IO_REG));
    return b;
}

char uart_getc()
{
    char c;
    while (!uart_read_ready())
        ;
    c = (char)(mmio_read(AUX_MU_IO_REG));
    switch (c) {
    case '\r':
        c = '\n';
        break;
    case 127: // backspace in qemu is 127
        c = '\b';
        break;
    }
    return c;
}

void uart_put(char c)
{
    switch (c) {
    case '\n':
    case '\b':
        while (!uart_write_ready())
            ;
        break;
    }
    switch (c) {
    case '\n':
        mmio_write(AUX_MU_IO_REG, '\r');
        break;
    case '\b':
        mmio_write(AUX_MU_IO_REG, c);
        while (!uart_write_ready())
            ;
        mmio_write(AUX_MU_IO_REG, ' ');
        break;
    }
    while (!uart_write_ready())
        ;
    mmio_write(AUX_MU_IO_REG, (unsigned int)c);
}

void uart_puts(char* s)
{
    while (*s) {
        uart_put(*s++);
    }
}

void uart_put_dec(unsigned long long num)
{
    if (!num) {
        uart_put('0');
        return;
    }
    int count = 0;
    unsigned long long reverse_num = 0;
    while (num) {
        reverse_num = reverse_num * 10 + num % 10;
        num /= 10;
        count++;
    }
    for (int i = 0; i < count; i++) {
        uart_put((reverse_num % 10) + '0');
        reverse_num /= 10;
    }
}

void uart_put_hex(unsigned long long num)
{
    if (!num) {
        uart_put('0');
        return;
    }
    int count = 0;
    unsigned long long reverse_num = 0;
    while (num) {
        reverse_num <<= 4;
        reverse_num |= (num & 0xF);
        num >>= 4;
        count++;
    }
    char c;
    for (int i = 0; i < count; i++) {
        c = reverse_num & 0xF;
        c += c > 9 ? ('A' - 10) : '0';
        uart_put(c);
        reverse_num >>= 4;
    }
}
