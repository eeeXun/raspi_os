#include "thread.h"
#include "exception.h"
#include "list.h"
#include "memory.h"
#include "program.h"
#include "register.h"
#include "sched.h"
#include "string.h"
#include "timer.h"

#define THREAD_STACK_SIZE 0x1000

int next_tid = 0;
LIST_HEAD(run_queue);
LIST_HEAD(zombie_queue);

extern void ret_from_fork();

user_program_t* user_program_load(void* data, int size)
{
    user_program_t* user_program = kmalloc(sizeof(user_program_t));
    user_program->data = kmalloc(size);
    memcpy(user_program->data, data, size);
    user_program->ref_count = 1;
    return user_program;
}

void user_program_put(user_program_t* user_program)
{
    if (!user_program)
        return;
    user_program->ref_count--;
    if (user_program->ref_count)
        return;
    kfree(user_program->data);
    kfree(user_program);
}

void idle_thread(void* arg)
{
    while (1) {
        unsigned long long daif_state = irq_save();
        while (!list_empty(&zombie_queue)) {
            thread_t* zombie_thread
                = list_first_entry(&zombie_queue, thread_t, list_node);
            list_del(&zombie_thread->list_node);
            user_program_put(zombie_thread->user_program);
            if (zombie_thread->user_stack)
                kfree(zombie_thread->user_stack);
            if (zombie_thread->signal_stack)
                kfree(zombie_thread->signal_stack);
            kfree(zombie_thread->kernel_stack);
            kfree(zombie_thread);
        }
        irq_restore(daif_state);
    }
}

void thread_exit()
{
    disable_interrupt();
    thread_t* thread = get_current_thread();
    thread->status = DEAD;
    list_del(&thread->list_node);
    list_add_tail(&thread->list_node, &zombie_queue);
    schedule();
}

thread_t* find_thread(int tid)
{
    thread_t* thread;
    list_for_each_entry(thread, &run_queue, list_node)
    {
        if (thread->tid == tid)
            return thread;
    }
    return 0;
}

void thread_kill(int tid)
{
    if (tid == get_current_thread()->tid) {
        thread_exit();
        return;
    }
    unsigned long long daif_state = irq_save();
    thread_t* thread = find_thread(tid);
    if (thread) {
        thread->status = DEAD;
        list_del(&thread->list_node);
        list_add_tail(&thread->list_node, &zombie_queue);
    }
    irq_restore(daif_state);
}

void thread_entry()
{
    enable_interrupt();
    thread_t* thread = get_current_thread();
    thread->func(thread->arg);
    thread_exit();
}

int thread_alive(int tid)
{
    unsigned long long daif_state = irq_save();
    thread_t* thread;
    list_for_each_entry(thread, &run_queue, list_node)
    {
        if (thread->tid == tid) {
            irq_restore(daif_state);
            return 1;
        }
    }
    irq_restore(daif_state);
    return 0;
}

void thread_wait(int tid)
{
    while (thread_alive(tid))
        schedule();
}

thread_t* thread_alloc()
{
    thread_t* thread = kmalloc(sizeof(thread_t));
    thread->kernel_stack = kmalloc(THREAD_STACK_SIZE);
    thread->status = RUNNING;
    thread->tid = next_tid++;
    thread->context.fp = 0;
    thread->context.lr = 0;
    thread->context.sp
        = (unsigned long long)thread->kernel_stack + THREAD_STACK_SIZE;
    thread->user_program = 0;
    thread->user_stack = 0;
    thread->signal_pending = 0;
    thread->signal_stack = 0;
    memset(thread->signal_handlers, 0, sizeof(thread->signal_handlers));
    return thread;
}

thread_t* thread_create(void (*func)(void*), void* arg)
{
    thread_t* thread = thread_alloc();
    thread->func = func;
    thread->arg = arg;
    thread->context.lr = (unsigned long long)thread_entry;

    unsigned long long daif_state = irq_save();
    list_add_tail(&thread->list_node, &run_queue);
    irq_restore(daif_state);
    return thread;
}

int thread_fork(trap_frame* tf)
{
    thread_t* parent = get_current_thread();
    thread_t* child = thread_alloc();
    child->user_program = parent->user_program;
    child->user_program->ref_count++;
    child->user_stack = kmalloc(USER_STACK_SIZE);
    memcpy(child->user_stack, parent->user_stack, USER_STACK_SIZE);
    trap_frame* child_tf = (trap_frame*)((char*)child->kernel_stack
        + THREAD_STACK_SIZE - sizeof(trap_frame));
    memcpy(child_tf, tf, sizeof(trap_frame));
    child_tf->x[0] = 0; // fork of child will return 0
    long long user_stack_offset = child->user_stack - parent->user_stack;
    // sp_el0 still points to the parent's user stack after the copy.
    // Not needed after we support virtual memory.
    child_tf->sp_el0 += user_stack_offset;
    // fp points to a frame record (previous fp and lr) on the user stack.
    // In theory, we should walk the whole fp chain, but it's fine in this lab.
    // Not needed after we support virtual memory.
    child_tf->x[29] += user_stack_offset;
    child->context.lr = (unsigned long long)ret_from_fork;
    child->context.sp = (unsigned long long)child_tf;
    memcpy(child->signal_handlers, parent->signal_handlers,
        sizeof(parent->signal_handlers));

    unsigned long long daif_state = irq_save();
    list_add_tail(&child->list_node, &run_queue);
    irq_restore(daif_state);
    return child->tid;
}

void thread_init()
{
    thread_t* thread = kmalloc(sizeof(thread_t));
    thread->kernel_stack = 0;
    thread->status = RUNNING;
    thread->tid = next_tid++;
    thread->func = 0;
    thread->user_program = 0;
    thread->user_stack = 0;
    thread->signal_pending = 0;
    thread->signal_stack = 0;
    memset(thread->signal_handlers, 0, sizeof(thread->signal_handlers));
    write_reg(tpidr_el1, thread);
    list_add_tail(&thread->list_node, &run_queue);
    // Create idle thread
    thread_create(idle_thread, 0);
    // Create timer interrupt for scheduling
    add_timer(sched_tick, 0, read_reg(cntfrq_el0) >> 5);
}
