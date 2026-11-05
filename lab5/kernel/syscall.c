#include "syscall.h"
#include "thread.h"
#include "uart.h"

int syscall_get_tid()
{
    thread_t* thread = get_current_thread();
    return thread->tid;
}

int syscall_uart_read(char* buf, int size)
{
    for (int i = 0; i < size; i++)
        buf[i] = uart_getc();
    return size;
}

int syscall_uart_write(char* buf, int size)
{
    for (int i = 0; i < size; i++)
        uart_put(buf[i]);
    return size;
}

void system_call(trap_frame* tf)
{
    switch (tf->x[8]) {
    case 0:
        tf->x[0] = syscall_get_tid();
        break;
    case 1:
        tf->x[0] = syscall_uart_read((char*)tf->x[0], tf->x[1]);
        break;
    case 2:
        tf->x[0] = syscall_uart_write((char*)tf->x[0], tf->x[1]);
        break;
    case 3:
        break;
    case 4:
        break;
    case 5:
        break;
    case 6:
        break;
    case 7:
        break;
    default:
        break;
    }
}
