#include "syscall.h"
#include "cpio.h"
#include "memory.h"
#include "program.h"
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

int syscall_exec(trap_frame* tf, char* name)
{
    long long filesize;
    char* data;
    if (!cpio_find(&filesize, name, &data))
        return -1;
    thread_t* thread = get_current_thread();
    if (thread->user_program)
        kfree(thread->user_program);
    thread->user_program = kmalloc(filesize);
    for (long long i = 0; i < filesize; i++)
        ((char*)thread->user_program)[i] = data[i];

    tf->elr_el1 = (unsigned long long)thread->user_program;
    tf->sp_el0 = (unsigned long long)thread->user_stack + USER_STACK_SIZE;
    return 0;
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
        tf->x[0] = syscall_exec(tf, (char*)tf->x[0]);
        break;
    case 4:
        break;
    case 5:
        thread_exit();
        break;
    case 6:
        break;
    case 7:
        break;
    default:
        break;
    }
}
