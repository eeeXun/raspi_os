#pragma once

typedef struct {
    unsigned long long x[31];
    unsigned long long sp_el0;
    unsigned long long spsr_el1;
    unsigned long long elr_el1;
} trap_frame;

void system_call(trap_frame* tf);
