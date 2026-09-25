#include "gic.h"
#include "register.h"
#include "timer.h"
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

void irq_handler()
{
    unsigned int id = gic_ack();
    switch (id) {
    case INTID_TIMER:
        uart_puts("\n[timer] ");
        uart_put_dec(read_reg(cntpct_el0) / read_reg(cntfrq_el0));
        uart_puts("s\n");
        // Reset timer value
        write_reg(cntp_tval_el0, read_reg(cntfrq_el0) * 2);
        break;
    case INTID_AUX:
        uart_irq_handler();
        break;
    default:
        break;
    }
    gic_eoi(id);
}
