#include "register.h"
#include "uart.h"

void exception_handler()
{
    uart_puts("### exception handler start ###\n");

    // Saved Program Status Register
    // Holds the saved process state
    uart_puts("spsr_el1: 0x");
    uart_put_hex(read_reg(spsr_el1));
    uart_put('\n');

    // Exception Link Register
    // Holds the address to return to
    uart_puts("elr_el1: 0x");
    uart_put_hex(read_reg(elr_el1));
    uart_put('\n');

    // Exception Syndrome Register
    // Holds syndrome information
    uart_puts("esr_el1: 0x");
    uart_put_hex(read_reg(esr_el1));
    uart_put('\n');

    uart_puts("### exception handler end ###\n");
}
