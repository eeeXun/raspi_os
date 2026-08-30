#define read_reg(reg)                                                          \
    ({                                                                         \
        unsigned long long __val;                                              \
        asm volatile("mrs %0, " #reg : "=r"(__val));                           \
        __val;                                                                 \
    })
