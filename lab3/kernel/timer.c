#include "register.h"

// Expire in n seconds
void reset_timer(unsigned long long n)
{
    write_reg(cntp_tval_el0, read_reg(cntfrq_el0) * n);
}
