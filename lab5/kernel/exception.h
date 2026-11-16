#pragma once

static inline void enable_interrupt() { asm volatile("msr DAIFClr, 0xF"); }
static inline void disable_interrupt() { asm volatile("msr DAIFSet, 0xF"); }

unsigned long long irq_save();
void irq_restore(unsigned long long daif_state);
