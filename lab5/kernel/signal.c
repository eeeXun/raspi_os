#include "exception.h"
#include "memory.h"
#include "program.h"
#include "string.h"
#include "thread.h"

extern void sigreturn_trampoline();

void signal_handle(trap_frame* tf)
{
    thread_t* thread = get_current_thread();
    while (thread->signal_pending
        && !thread->signal_stack // Only run handler when signal_stack is empty,
                                 // which means no handler is running
    ) {
        unsigned long long daif_state = irq_save();
        int sig = __builtin_ctz(thread->signal_pending); // Count Trailing Zeros
        thread->signal_pending &= ~(1 << sig);
        irq_restore(daif_state);

        if (!thread->signal_handlers[sig]) {
            if (sig == SIGKILL)
                thread_exit(); // default SIGKILL action
            continue;
        }
        memcpy(&thread->signal_tf, tf, sizeof(trap_frame));
        // eret in irq_entry/exception_entry goes to
        // thread->signal_handlers[sig]
        tf->elr_el1 = (unsigned long long)thread->signal_handlers[sig];
        // signal handler needs its own stack
        thread->signal_stack = kmalloc(USER_STACK_SIZE);
        tf->sp_el0 = (unsigned long long)thread->signal_stack + USER_STACK_SIZE;
        // Set lr (x30) so the handler returns to sigreturn_trampoline when
        // it finishes
        tf->x[30] = (unsigned long long)sigreturn_trampoline;
        // Pass sig as the first argument, like POSIX handlers
        tf->x[0] = sig;
        // Run one handler at a time. The handler returns to
        // sigreturn_trampoline, which calls sigreturn. Then
        // exception_handler calls signal_handle again for remaining signals
        return;
    }
}
