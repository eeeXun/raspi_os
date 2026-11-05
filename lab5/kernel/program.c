#include "memory.h"
#include "thread.h"

#define USER_STACK_SIZE 0x1000

// Implement in switch.s
extern void enter_el0(unsigned long long entry, unsigned long long user_sp);

void run_user_program(char* data, long long size)
{
    thread_t* thread = get_current_thread();
    thread->user_program = kmalloc(size);
    thread->user_stack = kmalloc(USER_STACK_SIZE);
    for (long long i = 0; i < size; i++)
        ((char*)thread->user_program)[i] = data[i];
    enter_el0((unsigned long long)thread->user_program,
        (unsigned long long)thread->user_stack + USER_STACK_SIZE);
}
