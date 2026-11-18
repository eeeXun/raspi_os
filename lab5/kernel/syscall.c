#include "syscall.h"
#include "cpio.h"
#include "mbox.h"
#include "program.h"
#include "signal.h"
#include "string.h"
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
    user_program_put(thread->user_program);
    thread->user_program = user_program_load(data, filesize);
    memset(thread->signal_handlers, 0, sizeof(thread->signal_handlers));

    tf->elr_el1 = (unsigned long long)thread->user_program->data;
    tf->sp_el0 = (unsigned long long)thread->user_stack + USER_STACK_SIZE;
    return 0;
}

void syscall_signal(int sig, signal_handler_t handler)
{
    if (sig < 0 || sig >= SIG_NUM)
        return;
    thread_t* thread = get_current_thread();
    thread->signal_handlers[sig] = handler;
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
        tf->x[0] = thread_fork(tf);
        break;
    case 5:
        thread_exit();
        break;
    case 6:
        tf->x[0] = mbox_call((unsigned char)tf->x[0], (unsigned int*)tf->x[1]);
        break;
    case 7:
        thread_kill(tf->x[0]);
        break;
    case 8:
        syscall_signal(tf->x[0], (signal_handler_t)tf->x[1]);
        break;
    default:
        break;
    }
}
