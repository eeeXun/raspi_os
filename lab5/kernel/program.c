#include "cpio.h"
#include "memory.h"
#include "thread.h"
#include "uart.h"

#define USER_STACK_SIZE 0x1000

// Implement in switch.s
extern void enter_el0(unsigned long long entry, unsigned long long user_sp);

void user_entry(void* name)
{
    long long filesize;
    char* data;
    if (!cpio_find(&filesize, name, &data)) {
        uart_puts("run: ");
        uart_puts(name);
        uart_puts(": No such file\n");
        return;
    }
    thread_t* thread = get_current_thread();
    thread->user_program = kmalloc(filesize);
    thread->user_stack = kmalloc(USER_STACK_SIZE);
    for (long long i = 0; i < filesize; i++)
        ((char*)thread->user_program)[i] = data[i];
    enter_el0((unsigned long long)thread->user_program,
        (unsigned long long)thread->user_stack + USER_STACK_SIZE);
}
