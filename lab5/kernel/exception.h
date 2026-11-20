#pragma once

typedef struct {
    unsigned long long x[31];
    unsigned long long sp_el0;
    unsigned long long spsr_el1;
    unsigned long long elr_el1;
} trap_frame;

static inline void enable_interrupt() { asm volatile("msr DAIFClr, 0xF"); }
static inline void disable_interrupt() { asm volatile("msr DAIFSet, 0xF"); }

unsigned long long irq_save();
void irq_restore(unsigned long long daif_state);
