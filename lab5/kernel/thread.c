#include "thread.h"
#include "list.h"
#include "memory.h"
#include "register.h"

#define THREAD_STACK_SIZE 0x1000

int next_tid = 0;
list_head run_queue;
list_head zombie_queue;

void schedule()
{
    thread_t* current = get_current_thread();
    if (current->status != DEAD)
        list_move_tail(&current->list_node, &run_queue);
    thread_t* next = list_first_entry(&run_queue, thread_t, list_node);
    if (current != next)
        switch_to(current, next);
}

void idle_thread(void* arg)
{
    thread_t *thread, *safe;
    while (1) {
        list_for_each_entry_safe(thread, safe, &zombie_queue, list_node)
        {
            list_del(&thread->list_node);
            if (thread->user_program)
                kfree(thread->user_program);
            if (thread->user_stack)
                kfree(thread->user_stack);
            kfree(thread->kernel_stack);
            kfree(thread);
        }
        schedule();
    }
}

void thread_exit()
{
    thread_t* thread = get_current_thread();
    thread->status = DEAD;
    list_del(&thread->list_node);
    list_add_tail(&thread->list_node, &zombie_queue);
    schedule();
}

void thread_entry()
{
    thread_t* thread = get_current_thread();
    thread->func(thread->arg);
    thread_exit();
}

int thread_alive(int tid)
{
    thread_t* thread;
    list_for_each_entry(thread, &run_queue, list_node)
    {
        if (thread->tid == tid)
            return 1;
    }
    return 0;
}

void thread_wait(int tid)
{
    while (thread_alive(tid))
        schedule();
}

int thread_create(void (*func)(void*), void* arg)
{
    thread_t* thread = kmalloc(sizeof(thread_t));
    thread->kernel_stack = kmalloc(THREAD_STACK_SIZE);
    thread->status = RUNNING;
    thread->tid = next_tid++;
    thread->func = func;
    thread->arg = arg;
    thread->context.fp = 0;
    thread->context.lr = (unsigned long long)thread_entry;
    thread->context.sp
        = (unsigned long long)thread->kernel_stack + THREAD_STACK_SIZE;
    thread->user_program = 0;
    thread->user_stack = 0;
    list_add_tail(&thread->list_node, &run_queue);
    return thread->tid;
}

void thread_init()
{
    INIT_LIST_HEAD(&run_queue);
    INIT_LIST_HEAD(&zombie_queue);
    thread_t* thread = kmalloc(sizeof(thread_t));
    thread->kernel_stack = 0;
    thread->status = RUNNING;
    thread->tid = next_tid++;
    thread->func = 0;
    thread->user_program = 0;
    thread->user_stack = 0;
    write_reg(tpidr_el1, thread);
    list_add_tail(&thread->list_node, &run_queue);
}
