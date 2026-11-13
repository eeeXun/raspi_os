#include "gic.h"
#include "register.h"
#include "syscall.h"
#include "task.h"
#include "thread.h"
#include "timer.h"
#include "uart.h"

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

void exception_handler(trap_frame* tf)
{
    switch ((read_reg(esr_el1) >> 26) & 0b111111) {
    case 0b010101: // EC = SVC from AArch64
        enable_interrupt();
        system_call(tf);
        break;
    default:
        uart_puts("Unhandled exception, esr_el1: 0x");
        uart_put_hex(read_reg(esr_el1));
        uart_puts(", elr_el1: 0x");
        uart_put_hex(read_reg(elr_el1));
        uart_put('\n');
        if ((tf->spsr_el1 & 0xf) == 0) // from EL0
            thread_exit();
        else // from EL1, kernel bug
            while (1)
                asm volatile("wfi");
        break;
    }
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
