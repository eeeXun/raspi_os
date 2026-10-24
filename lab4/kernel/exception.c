#include "gic.h"
#include "register.h"
#include "task.h"
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
    gic_disable(id);
    switch (id) {
    case INTID_TIMER:
        timer_irq_top_half();
        task_add(timer_irq_bottom_half, PRIO_TIMER);
        break;
    case INTID_AUX:
        uart_irq_top_half();
        task_add(uart_irq_bottom_half, PRIO_UART);
        break;
    default:
        break;
    }
    gic_eoi(id);
    task_run();
}

unsigned long long irq_save()
{
    unsigned long long daif = read_reg(daif);
    asm volatile("msr DAIFSet, 0xf"); // disable interrupt
    return daif;
}

void irq_restore(unsigned long long daif_state)
{
    write_reg(daif, daif_state); // restore daif to previous state
}

void enable_interrupt() { asm volatile("msr DAIFClr, 0xf"); }

void disable_interrupt() { asm volatile("msr DAIFSet, 0xf"); }
